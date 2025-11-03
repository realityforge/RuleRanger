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

#include "RuleRangerContentBrowserExtensions.h"
#include "ContentBrowserModule.h"
#include "Logging/StructuredLog.h"
#include "RuleRanger/RuleRangerEditorSubsystem.h"
#include "RuleRangerCommands.h"
#include "RuleRangerConfig.h"
#include "RuleRangerDeveloperSettings.h"
#include "RuleRangerLogging.h"
#include "RuleRangerStyle.h"

static void OnScanSelectedAssets(const TArray<FAssetData>& Assets)
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnScanSelectedAssets(Assets);
    }
}

static void OnFixSelectedAssets(const TArray<FAssetData>& Assets)
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnFixSelectedAssets(Assets);
    }
}

static void OnScanSelectedPaths(const TArray<FString>& AssetPaths)
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnScanSelectedPaths(AssetPaths);
    }
}

static void OnFixSelectedPaths(const TArray<FString>& AssetPaths)
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnFixSelectedPaths(AssetPaths);
    }
}

static bool HasAnyConfiguredDirs()
{
    const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>();
    return Subsystem ? Subsystem->HasAnyConfiguredDirs() : false;
}

static bool SelectionIntersectsConfiguredDirs(const TArray<FString>& Paths)
{
    const auto DevSettings = GetDefault<URuleRangerDeveloperSettings>();
    if (IsValid(DevSettings))
    {
        TArray<FString> DirPaths;
        for (const auto& SoftConfig : DevSettings->Configs)
        {
            if (const auto Config = SoftConfig.LoadSynchronous())
            {
                for (const auto& Dir : Config->Dirs)
                {
                    if (!Dir.Path.IsEmpty())
                    {
                        DirPaths.Add(Dir.Path);
                    }
                }
            }
        }
        for (auto P : Paths)
        {
            if (!P.IsEmpty() && !P.EndsWith(TEXT("/")))
            {
                P.Append(TEXT("/"));
            }
            for (const auto& D : DirPaths)
            {
                if (P.StartsWith(D, ESearchCase::CaseSensitive) || D.StartsWith(P, ESearchCase::CaseSensitive))
                {
                    return true;
                }
            }
        }
        return false;
    }
    else
    {
        return false;
    }
}

static bool SelectionIntersectsConfiguredDirs(const TArray<FAssetData>& Assets)
{
    const auto DevSettings = GetDefault<URuleRangerDeveloperSettings>();
    if (IsValid(DevSettings))
    {
        TArray<FString> DirPaths;
        for (const auto& SoftConfig : DevSettings->Configs)
        {
            if (const auto Config = SoftConfig.LoadSynchronous())
            {
                for (const auto& Dir : Config->Dirs)
                {
                    if (!Dir.Path.IsEmpty())
                    {
                        DirPaths.Add(Dir.Path);
                    }
                }
            }
        }
        for (const auto& A : Assets)
        {
            const auto PathStr = A.GetSoftObjectPath().ToString();
            for (const auto& D : DirPaths)
            {
                if (PathStr.StartsWith(D, ESearchCase::CaseSensitive))
                {
                    return true;
                }
            }
        }
        return false;
    }
    else
    {
        return false;
    }
}

static TSharedRef<FExtender> OnExtendSelectedPathsMenu(const TArray<FString>& Paths)
{
    UE_LOGFMT(LogRuleRanger, VeryVerbose, "OnExtendSelectedPathsMenu() invoked.");

    const TSharedPtr<FUICommandList> CommandList = MakeShareable(new FUICommandList);
    const bool bHasDirs = HasAnyConfiguredDirs();
    const bool bIntersects = bHasDirs && SelectionIntersectsConfiguredDirs(Paths);
    CommandList->MapAction(FRuleRangerCommands::Get().ScanSelectedPaths,
                           FExecuteAction::CreateLambda([Paths] { OnScanSelectedPaths(Paths); }),
                           FCanExecuteAction::CreateLambda([bIntersects] { return bIntersects; }));
    CommandList->MapAction(FRuleRangerCommands::Get().FixSelectedPaths,
                           FExecuteAction::CreateLambda([Paths] { OnFixSelectedPaths(Paths); }),
                           FCanExecuteAction::CreateLambda([bIntersects] { return bIntersects; }));
    auto Extender = MakeShared<FExtender>();
    FText ScanTip;
    FText FixTip;
    if (!bHasDirs)
    {
        ScanTip =
            NSLOCTEXT("RuleRanger",
                      "CBNoDirsScanTip",
                      "No RuleRanger directories configured. Set them in Project Settings → Editor → Rule Ranger.");
        FixTip = ScanTip;
    }
    else if (!bIntersects)
    {
        ScanTip =
            NSLOCTEXT("RuleRanger", "CBNoIntersectScanTip", "Selection is outside configured RuleRanger directories.");
        FixTip = ScanTip;
    }
    else
    {
        ScanTip = NSLOCTEXT("RuleRanger", "CBScanPathsTip", "Scan selected paths with RuleRanger.");
        FixTip = NSLOCTEXT("RuleRanger", "CBFixPathsTip", "Scan and fix selected paths with RuleRanger.");
    }
    const auto MenuExtensionDelegate = FMenuExtensionDelegate::CreateLambda([ScanTip, FixTip](auto& MenuBuilder) {
        MenuBuilder.BeginSection("RuleRangerContentBrowserContext",
                                 NSLOCTEXT("RuleRanger", "ContextMenuSectionName", "Rule Ranger"));
        MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().ScanSelectedPaths,
                                 NAME_None,
                                 TAttribute<FText>(),
                                 TAttribute<FText>(ScanTip),
                                 FRuleRangerStyle::GetScanIcon());
        MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().FixSelectedPaths,
                                 NAME_None,
                                 TAttribute<FText>(),
                                 TAttribute<FText>(FixTip),
                                 FRuleRangerStyle::GetScanAndFixIcon());
        MenuBuilder.AddSeparator();
        MenuBuilder.EndSection();
    });
    Extender->AddMenuExtension("PathContextBulkOperations", EExtensionHook::After, CommandList, MenuExtensionDelegate);
    return Extender;
}

static TSharedRef<FExtender> OnExtendForSelectedAssetsMenu(const TArray<FAssetData>& Assets)
{
    UE_LOGFMT(LogRuleRanger, VeryVerbose, "OnExtendForSelectedAssetsMenu() invoked.");

    const TSharedPtr<FUICommandList> CommandList = MakeShareable(new FUICommandList);
    const bool bHasDirs = HasAnyConfiguredDirs();
    const bool bIntersects = bHasDirs && SelectionIntersectsConfiguredDirs(Assets);
    CommandList->MapAction(FRuleRangerCommands::Get().ScanSelectedAssets,
                           FExecuteAction::CreateLambda([Assets] { OnScanSelectedAssets(Assets); }),
                           FCanExecuteAction::CreateLambda([bIntersects] { return bIntersects; }));
    CommandList->MapAction(FRuleRangerCommands::Get().FixSelectedAssets,
                           FExecuteAction::CreateLambda([Assets] { OnFixSelectedAssets(Assets); }),
                           FCanExecuteAction::CreateLambda([bIntersects] { return bIntersects; }));

    TSharedRef<FExtender> Extender = MakeShared<FExtender>();
    FText ScanTip;
    FText FixTip;
    if (!bHasDirs)
    {
        ScanTip =
            NSLOCTEXT("RuleRanger",
                      "CBNoDirsScanTipA",
                      "No RuleRanger directories configured. Set them in Project Settings → Editor → Rule Ranger.");
        FixTip = ScanTip;
    }
    else if (!bIntersects)
    {
        ScanTip =
            NSLOCTEXT("RuleRanger", "CBNoIntersectScanTipA", "Selection is outside configured RuleRanger directories.");
        FixTip = ScanTip;
    }
    else
    {
        ScanTip = NSLOCTEXT("RuleRanger", "CBScanAssetsTip", "Scan selected assets with RuleRanger.");
        FixTip = NSLOCTEXT("RuleRanger", "CBFixAssetsTip", "Scan and fix selected assets with RuleRanger.");
    }
    const auto MenuExtensionDelegate = FMenuExtensionDelegate::CreateLambda([ScanTip, FixTip](auto& MenuBuilder) {
        MenuBuilder.BeginSection("RuleRangerContentBrowserContext",
                                 NSLOCTEXT("RuleRanger", "ContextMenuSectionName", "Rule Ranger"));
        MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().ScanSelectedAssets,
                                 NAME_None,
                                 TAttribute<FText>(),
                                 TAttribute<FText>(ScanTip),
                                 FRuleRangerStyle::GetScanIcon());
        MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().FixSelectedAssets,
                                 NAME_None,
                                 TAttribute<FText>(),
                                 TAttribute<FText>(FixTip),
                                 FRuleRangerStyle::GetScanAndFixIcon());
        MenuBuilder.AddSeparator();
        MenuBuilder.EndSection();
    });
    Extender->AddMenuExtension("CommonAssetActions", EExtensionHook::After, CommandList, MenuExtensionDelegate);

    return Extender;
}

FContentBrowserMenuExtender_SelectedPaths FRuleRangerContentBrowserExtensions::SelectedPathsDelegate;
FDelegateHandle FRuleRangerContentBrowserExtensions::SelectedPathsDelegateHandle;
FContentBrowserMenuExtender_SelectedAssets FRuleRangerContentBrowserExtensions::SelectedAssetsDelegate;
FDelegateHandle FRuleRangerContentBrowserExtensions::SelectedAssetsDelegateHandle;

void FRuleRangerContentBrowserExtensions::Initialize()
{
    auto& Module = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

    UE_LOGFMT(LogRuleRanger,
              VeryVerbose,
              "FRuleRangerContentBrowserExtensions::Initialize(): Registering ContentBrowser Extensions.");
    SelectedPathsDelegate = FContentBrowserMenuExtender_SelectedPaths::CreateStatic(&OnExtendSelectedPathsMenu);
    Module.GetAllPathViewContextMenuExtenders().Add(SelectedPathsDelegate);
    SelectedPathsDelegateHandle = SelectedPathsDelegate.GetHandle();
    check(SelectedPathsDelegateHandle.IsValid());

    // Asset extenders
    SelectedAssetsDelegate = FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&OnExtendForSelectedAssetsMenu);
    Module.GetAllAssetViewContextMenuExtenders().Add(SelectedAssetsDelegate);
    SelectedAssetsDelegateHandle = SelectedAssetsDelegate.GetHandle();
    check(SelectedAssetsDelegateHandle.IsValid());
}

void FRuleRangerContentBrowserExtensions::Shutdown()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const FName ContentBrowserModuleName{ TEXT("ContentBrowser") };
    if (FModuleManager::Get().IsModuleLoaded(ContentBrowserModuleName))
    {
        auto& Module = FModuleManager::LoadModuleChecked<FContentBrowserModule>(ContentBrowserModuleName);
        if (SelectedPathsDelegateHandle.IsValid())
        {
            UE_LOGFMT(LogRuleRanger,
                      VeryVerbose,
                      "FRuleRangerContentBrowserExtensions::Shutdown(): "
                      "Deregistering Path ContextMenu ContentBrowser Extensions.");
            auto Target = SelectedPathsDelegateHandle;
            auto& Extenders = Module.GetAllPathViewContextMenuExtenders();
            Extenders.RemoveAll([&Target](const auto& Delegate) { return Delegate.GetHandle() == Target; });
            Target.Reset();
        }
        else
        {
            UE_LOGFMT(LogRuleRanger,
                      Verbose,
                      "FRuleRangerContentBrowserExtensions::Shutdown(): "
                      "Skipping deregister of Path ContextMenu ContentBrowser Extensions as handle is Invalid.");
        }

        if (SelectedAssetsDelegateHandle.IsValid())
        {
            UE_LOGFMT(LogRuleRanger,
                      VeryVerbose,
                      "FRuleRangerContentBrowserExtensions::Shutdown(): "
                      "Deregistering Asset ContextMenu ContentBrowser Extensions.");
            auto Target = SelectedAssetsDelegateHandle;
            auto& Extenders = Module.GetAllAssetViewContextMenuExtenders();
            Extenders.RemoveAll([&Target](auto& Delegate) { return Delegate.GetHandle() == Target; });
            SelectedAssetsDelegateHandle.Reset();
        }
        else
        {
            UE_LOGFMT(LogRuleRanger,
                      Verbose,
                      "FRuleRangerContentBrowserExtensions::Shutdown(): "
                      "Skipping deregister of Asset Asset ContextMenu ContentBrowser Extensions as handle is Invalid.");
        }
    }
}
