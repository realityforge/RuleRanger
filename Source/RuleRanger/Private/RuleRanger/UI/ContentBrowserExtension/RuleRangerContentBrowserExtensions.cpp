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
#include "RuleRanger/UI/RuleRangerCommands.h"
#include "RuleRanger/UI/RuleRangerEditorSubsystem.h"
#include "RuleRanger/UI/RuleRangerStyle.h"
#include "RuleRanger/UI/RuleRangerTools.h"
#include "RuleRangerLogging.h"

static TSharedRef<FExtender> OnExtendSelectedPathsMenu(const TArray<FString>& Paths)
{
    UE_LOGFMT(LogRuleRanger, VeryVerbose, "OnExtendSelectedPathsMenu() invoked.");

    const TSharedPtr<FUICommandList> CommandList = MakeShareable(new FUICommandList);
    const bool bHasDirs = FRuleRangerTools::HasAnyConfiguredDirs();
    const bool bIntersects = bHasDirs && FRuleRangerTools::PathSelectionIntersectsConfiguredDirs(Paths);
    CommandList->MapAction(FRuleRangerCommands::Get().ScanSelectedPaths,
                           FExecuteAction::CreateLambda([Paths] { FRuleRangerTools::OnScanSelectedPaths(Paths); }),
                           FCanExecuteAction::CreateLambda([bIntersects] { return bIntersects; }));
    CommandList->MapAction(FRuleRangerCommands::Get().FixSelectedPaths,
                           FExecuteAction::CreateLambda([Paths] { FRuleRangerTools::OnFixSelectedPaths(Paths); }),
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
    const bool bHasDirs = FRuleRangerTools::HasAnyConfiguredDirs();
    const bool bIntersects = bHasDirs && FRuleRangerTools::AssetSelectionIntersectsConfiguredDirs(Assets);
    CommandList->MapAction(FRuleRangerCommands::Get().ScanSelectedAssets,
                           FExecuteAction::CreateLambda([Assets] { FRuleRangerTools::OnScanSelectedAssets(Assets); }),
                           FCanExecuteAction::CreateLambda([bIntersects] { return bIntersects; }));
    CommandList->MapAction(FRuleRangerCommands::Get().FixSelectedAssets,
                           FExecuteAction::CreateLambda([Assets] { FRuleRangerTools::OnFixSelectedAssets(Assets); }),
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
