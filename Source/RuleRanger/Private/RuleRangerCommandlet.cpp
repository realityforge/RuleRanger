/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "RuleRangerCommandlet.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "Editor.h"
#include "Logging/StructuredLog.h"
#include "Misc/FileHelper.h"
#include "RuleRanger/RuleRangerEditorSubsystem.h"
#include "RuleRangerActionContext.h"
#include "RuleRangerLogging.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "ShaderCompiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RuleRangerCommandlet)

static void WaitForShaders()
{
    if (GShaderCompilingManager)
    {
        GShaderCompilingManager->FinishAllCompilation();
    }
}

URuleRangerCommandlet::URuleRangerCommandlet()
{
    IsClient = false;
    IsServer = false;
    LogToConsole = true;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerCommandlet::CollectAssetsFromAllowlist(const TArray<FString>& AllowlistPaths,
                                                       TArray<FAssetData>& Assets)
{
    const auto& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    for (const auto& Path : AllowlistPaths)
    {
        TArray<FAssetData> PathAssets;
        AssetRegistry.Get().GetAssetsByPath(*Path, PathAssets, true);
        Assets.Append(PathAssets);
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerCommandlet::DeriveAllowlistPaths(const FString& Params, TArray<FString>& AllowlistPaths)
{
    FString PathsParam;
    if (FParse::Value(*Params, TEXT("paths="), PathsParam))
    {
        PathsParam.ParseIntoArray(AllowlistPaths, TEXT(","), true);
    }
    if (0 == AllowlistPaths.Num())
    {
        AllowlistPaths.Add(TEXT("/Game"));
    }
}

void URuleRangerCommandlet::ResetState()
{
    NumErrors = 0;
    NumWarnings = 0;
    NumFatals = 0;
    NumAssetsScanned = 0;
    JsonResults.Reset();
}

int32 URuleRangerCommandlet::Main(const FString& Params)
{
    if (const auto Subsystem = GEditor ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>() : nullptr)
    {
        const auto bFix = Params.Contains(TEXT("fix"));
        const auto bExitOnWarning = Params.Contains(TEXT("exitOnWarning"));
        const auto bQuiet = Params.Contains(TEXT("quiet"));

        FString ReportPath;
        FParse::Value(*Params, TEXT("report="), ReportPath);

        TArray<FString> AllowlistPaths;
        DeriveAllowlistPaths(Params, AllowlistPaths);

        TArray<FAssetData> Assets;
        CollectAssetsFromAllowlist(AllowlistPaths, Assets);

        // Reset state
        ResetState();

        // blocks until all async package loads are finished
        FlushAsyncLoading();

        // Make sure all shaders are compiled
        WaitForShaders();

        // --- Scan assets ---
        for (const auto& Asset : Assets)
        {
            CurrentAsset = Asset;
            if (const auto Object = Asset.GetAsset())
            {
                NumAssetsScanned++;
                if (bFix)
                {
                    Subsystem->ScanAndFixObject(Object, this);
                }
                else
                {
                    Subsystem->ScanObject(Object, this);
                }
            }
        }

        // --- JSON report ---
        if (!ReportPath.IsEmpty())
        {
            const auto Root = MakeShared<FJsonObject>();

            // Summary
            const auto Summary = MakeShared<FJsonObject>();
            Summary->SetNumberField(TEXT("AssetsScanned"), NumAssetsScanned);
            Summary->SetNumberField(TEXT("Errors"), NumErrors);
            Summary->SetNumberField(TEXT("Warnings"), NumWarnings);
            Summary->SetNumberField(TEXT("Fatals"), NumFatals);
            Root->SetObjectField(TEXT("Summary"), Summary);

            // Results
            Root->SetArrayField(TEXT("Results"), JsonResults);

            FString OutputString;
            const auto Writer = TJsonWriterFactory<>::Create(&OutputString);
            FJsonSerializer::Serialize(Root, Writer);

            FFileHelper::SaveStringToFile(OutputString, *ReportPath);

            if (!bQuiet)
            {
                UE_LOGFMT(LogRuleRanger, Display, "RuleRanger report written to {Path}", ReportPath);
            }
        }

        const int32 Result = NumErrors > 0 || NumFatals > 0 || (bExitOnWarning && NumWarnings > 0) ? 1 : 0;
        ResetState();
        return Result;
    }
    else
    {
        UE_LOGFMT(LogRuleRanger, Error, "RuleRangerCommandlet: Unable to get URuleRangerEditorSubsystem");
        return 1;
    }
}

void URuleRangerCommandlet::OnRuleApplied(URuleRangerActionContext* ActionContext)
{
    const auto Fatals = ActionContext->GetFatalMessages().Num();
    const auto Errors = ActionContext->GetErrorMessages().Num();
    const auto Warnings = ActionContext->GetWarningMessages().Num();

    NumFatals += Fatals;
    NumErrors += Errors;
    NumWarnings += Warnings;

    if (Errors > 0 || Fatals > 0 || Warnings > 0)
    {
        auto AssetResult = MakeShared<FJsonObject>();
        AssetResult->SetStringField(TEXT("AssetName"), CurrentAsset.AssetName.ToString());
        AssetResult->SetStringField(TEXT("AssetPath"), CurrentAsset.GetObjectPathString());

        if (0 != Fatals || 0 != Errors)
        {
            TArray<TSharedPtr<FJsonValue>> ErrorsJson;
            for (const auto& Error : ActionContext->GetErrorMessages())
            {
                ErrorsJson.Add(MakeShared<FJsonValueString>(Error.ToString()));
            }
            for (const auto& Fatal : ActionContext->GetFatalMessages())
            {
                ErrorsJson.Add(MakeShared<FJsonValueString>(Fatal.ToString()));
            }
            AssetResult->SetArrayField(TEXT("Errors"), ErrorsJson);
        }

        if (0 != Warnings)
        {
            TArray<TSharedPtr<FJsonValue>> WarningsJson;
            for (const auto& Warning : ActionContext->GetWarningMessages())
            {
                WarningsJson.Add(MakeShared<FJsonValueString>(Warning.ToString()));
            }
            AssetResult->SetArrayField(TEXT("Warnings"), WarningsJson);
        }

        JsonResults.Add(MakeShared<FJsonValueObject>(AssetResult));
    }
}
