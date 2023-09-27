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
#include "RuleRanger/RuleRangerEditorSubsystem.h"
#include "RuleRangerDeveloperSettings.h"
#include "RuleRangerLogging.h"
#include "RuleRangerRule.h"
#include "Subsystems/EditorAssetSubsystem.h"

static FName ImportMarkerKey = FName(TEXT("RuleRangerImportProcessed"));
static FString ImportMarkerValue = FString(TEXT("True"));

void URuleRangerEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    // Register delegate for OnAssetPostImport callback
    if (const auto Subsystem = GEditor->GetEditorSubsystem<UImportSubsystem>())
    {
        OnAssetPostImportDelegateHandle =
            Subsystem->OnAssetPostImport.AddUObject(this, &URuleRangerEditorSubsystem::OnAssetPostImport);
    }
}

void URuleRangerEditorSubsystem::Deinitialize()
{
    // Deregister delegate for OnAssetPostImport callback
    if (OnAssetPostImportDelegateHandle.IsValid())
    {
        if (const auto Subsystem = GEditor->GetEditorSubsystem<UImportSubsystem>())
        {
            Subsystem->OnAssetPostImport.Remove(OnAssetPostImportDelegateHandle);
            OnAssetPostImportDelegateHandle.Reset();
        }
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerEditorSubsystem::OnAssetPostImport([[maybe_unused]] UFactory* Factory, UObject* Object)
{
    if (IsValid(Object))
    {
        const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();

        // Use a metadata tag when we have imported an asset so that when we try to reimport asset we can
        // identify this through the presence of tag.
        const bool bIsReimport = Subsystem && Subsystem->GetMetadataTag(Object, ImportMarkerKey) == ImportMarkerValue;
        const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
        check(IsValid(DeveloperSettings));

        for (auto RuleSetIt = DeveloperSettings->Rules.CreateIterator(); RuleSetIt; ++RuleSetIt)
        {
            const auto RuleSet = RuleSetIt->Get();
            if (const auto Path = Object->GetPathName(); Path.StartsWith(RuleSet->Dir.Path))
            {
                for (const auto RulePtr : RuleSet->Rules)
                {
                    // ReSharper disable once CppTooWideScopeInitStatement
                    const auto Rule = RulePtr.Get();
                    if ((!bIsReimport && Rule->bApplyOnImport) || (bIsReimport && Rule->bApplyOnReimport))
                    {
                        UE_LOG(RuleRanger, Warning, TEXT("OnAssetPostImport=%s"), *Object->GetName());
                    }
                }
            }
        }
    }
}
