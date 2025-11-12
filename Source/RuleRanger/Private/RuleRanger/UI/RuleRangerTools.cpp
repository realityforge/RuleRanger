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

#include "RuleRangerTools.h"
#include "AssetRegistry/AssetData.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "Framework/Docking/TabManager.h"
#include "IContentBrowserSingleton.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "RuleRanger/UI/RuleRangerEditorSubsystem.h"
#include "RuleRanger/UI/ToolTab/RuleRangerToolTab.h"
#include "RuleRangerDeveloperSettings.h"

bool FRuleRangerTools::HasAnyConfiguredDirs()
{
    const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>();
    return Subsystem ? Subsystem->HasAnyConfiguredDirs() : false;
}

bool FRuleRangerTools::CanRunScanAll()
{
    return CanRunScanContent() || CanRunScanProject();
}

bool FRuleRangerTools::CanRunScanProject()
{
    const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>();
    return Subsystem ? Subsystem->HasAnyProjectRules() : false;
}

bool FRuleRangerTools::CanRunScanContent()
{
    return HasAnyConfiguredDirs();
}

bool FRuleRangerTools::CanRunScanSelected()
{
    return AssetSelectionIntersectsConfiguredDirs() || PathSelectionIntersectsConfiguredDirs();
}

void FRuleRangerTools::OnScanContent()
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnScanContent();
    }
}

void FRuleRangerTools::OnFixContent()
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnFixContent();
    }
}

void FRuleRangerTools::OnScanProject()
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnScanProject();
    }
}

void FRuleRangerTools::OnFixProject()
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnFixProject();
    }
}

void FRuleRangerTools::OnScanAll()
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnScanProject();
        Subsystem->OnScanContent();
    }
}

void FRuleRangerTools::OnFixAll()
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnFixProject();
        Subsystem->OnFixContent();
    }
}

void FRuleRangerTools::OnOpenProjectSettings()
{
    if (const auto SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>(TEXT("Settings")))
    {
        if (const auto Settings = GetDefault<URuleRangerDeveloperSettings>())
        {
            const auto Container = Settings->GetContainerName();
            const auto Category = Settings->GetCategoryName();
            const auto Section = Settings->GetSectionName();
            SettingsModule->ShowViewer(Container, Category, Section);
        }
    }
    else
    {
        FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("ProjectSettings")));
    }
}

void FRuleRangerTools::OnOpenTool()
{
    FGlobalTabmanager::Get()->TryInvokeTab(FRuleRangerToolTab::GetTabName());
}

void FRuleRangerTools::OnScanSelected()
{
    OnScanSelectedAssets();
    OnScanSelectedPaths();
}

void FRuleRangerTools::OnFixSelected()
{
    OnFixSelectedAssets();
    OnFixSelectedPaths();
}

void FRuleRangerTools::OnScanSelectedAssets()
{
    TArray<FAssetData> Assets;
    GetSelectedAssets(Assets);
    if (Assets.Num() > 0)
    {
        OnScanSelectedAssets(Assets);
    }
}

void FRuleRangerTools::OnFixSelectedAssets()
{
    TArray<FAssetData> Assets;
    GetSelectedAssets(Assets);
    if (Assets.Num() > 0)
    {
        OnFixSelectedAssets(Assets);
    }
}

void FRuleRangerTools::OnScanSelectedAssets(const TArray<FAssetData>& Assets)
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnScanSelectedAssets(Assets);
    }
}

void FRuleRangerTools::OnFixSelectedAssets(const TArray<FAssetData>& Assets)
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnFixSelectedAssets(Assets);
    }
}

void FRuleRangerTools::OnScanSelectedPaths()
{
    TArray<FString> Paths;
    GetSelectedPaths(Paths);
    if (Paths.Num() > 0)
    {
        OnScanSelectedPaths(Paths);
    }
}

void FRuleRangerTools::OnFixSelectedPaths()
{
    TArray<FString> Paths;
    GetSelectedPaths(Paths);
    if (Paths.Num() > 0)
    {
        OnFixSelectedPaths(Paths);
    }
}

void FRuleRangerTools::OnScanSelectedPaths(const TArray<FString>& AssetPaths)
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnScanSelectedPaths(AssetPaths);
    }
}

void FRuleRangerTools::OnFixSelectedPaths(const TArray<FString>& AssetPaths)
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnFixSelectedPaths(AssetPaths);
    }
}

bool FRuleRangerTools::PathSelectionIntersectsConfiguredDirs()
{
    TArray<FString> Paths;
    GetSelectedPaths(Paths);
    return PathSelectionIntersectsConfiguredDirs(Paths);
}

bool FRuleRangerTools::PathSelectionIntersectsConfiguredDirs(const TArray<FString>& Paths)
{
    TArray<FString> DirPaths;
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->CollectConfiguredPaths(DirPaths);
    }
    for (auto Path : Paths)
    {
        if (!Path.IsEmpty() && !Path.EndsWith(TEXT("/")))
        {
            Path.Append(TEXT("/"));
        }
        for (const auto& Prefix : DirPaths)
        {
            if (Path.StartsWith(Prefix, ESearchCase::CaseSensitive)
                || Prefix.StartsWith(Path, ESearchCase::CaseSensitive))
            {
                return true;
            }
        }
    }
    return false;
}

void FRuleRangerTools::CollectAllSelectedAssets(TArray<FAssetData>& OutAssets)
{
    OutAssets.Reset();

    GetSelectedAssets(OutAssets);

    TArray<FString> Paths;
    GetSelectedPaths(Paths);
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        // Expand folders into assets
        Subsystem->CollectAssetsFromPaths(Paths, OutAssets);
    }
}

void FRuleRangerTools::CollectAllConfiguredAssets(TArray<FAssetData>& OutAssets)
{
    OutAssets.Reset();

    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        TArray<FString> Paths;
        Subsystem->CollectConfiguredPaths(Paths);
        // Expand folders into assets
        Subsystem->CollectAssetsFromPaths(Paths, OutAssets);
    }
}

bool FRuleRangerTools::AssetSelectionIntersectsConfiguredDirs()
{
    TArray<FAssetData> Assets;
    GetSelectedAssets(Assets);
    return AssetSelectionIntersectsConfiguredDirs(Assets);
}

bool FRuleRangerTools::AssetSelectionIntersectsConfiguredDirs(const TArray<FAssetData>& Assets)
{
    TArray<FString> Paths;
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->CollectConfiguredPaths(Paths);
    }
    for (const auto& Asset : Assets)
    {
        const auto Path = Asset.GetSoftObjectPath().ToString();
        for (const auto& Prefix : Paths)
        {
            if (Path.StartsWith(Prefix, ESearchCase::CaseSensitive))
            {
                return true;
            }
        }
    }
    return false;
}

void FRuleRangerTools::GetSelectedAssets(TArray<FAssetData>& OutAssets)
{
    OutAssets.Reset();
    if (const auto ContentBrowser = GetContentBrowser())
    {
        ContentBrowser->GetSelectedAssets(OutAssets);
    }
}

void FRuleRangerTools::GetSelectedPaths(TArray<FString>& OutPaths)
{
    OutPaths.Reset();
    if (const auto ContentBrowser = GetContentBrowser())
    {
        ContentBrowser->GetSelectedFolders(OutPaths);
    }
}

IContentBrowserSingleton* FRuleRangerTools::GetContentBrowser()
{
    static const FName NAME_ModuleName{ "ContentBrowser" };
    return FModuleManager::Get().IsModuleLoaded(NAME_ModuleName)
        ? &FModuleManager::LoadModuleChecked<FContentBrowserModule>(NAME_ModuleName).Get()
        : nullptr;
}
