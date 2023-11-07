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
#include "RuleRanger.h"
#include "RuleRangerLogging.h"
#include "RuleRangerStyle.h"

// -------------------------------------------------------------------------------------------
// Content browser extensions
// -------------------------------------------------------------------------------------------

static void OnScanSelectedPaths(const TArray<FString>& SelectedPaths)
{
    if (const auto& Module = FModuleManager::GetModulePtr<FRuleRangerModule>(FRuleRangerModule::GetModuleName()))
    {
        // TODO: Replace this with dialog to perform scan
        UE_LOG(RuleRanger, Error, TEXT("OnScanSelectedPaths not implemented yet."));
    }
}

// ReSharper disable once CppPassValueParameterByConstReference
static void OnExtendContentBrowserForSelectedPaths(FMenuBuilder& MenuBuilder, const TArray<FString> SelectedPaths)
{
    UE_LOG(RuleRanger, VeryVerbose, TEXT("FRuleRangerContentBrowserExtensions: OnExtendContentBrowser() invoked."));

    MenuBuilder.BeginSection("RuleRangerContentBrowserContext",
                             NSLOCTEXT("RuleRanger", "ContextMenuSectionName", "Rule Ranger"));

    MenuBuilder.AddMenuEntry(
        NSLOCTEXT("RuleRanger", "ScanSelectedPaths", "Scan with RuleRanger"),
        NSLOCTEXT("RuleRanger", "ScanSelectedPaths", "Scan selected paths with RuleRanger"),
        FSlateIcon(FRuleRangerStyle::GetStyleSetName(), FRuleRangerStyle::GetToolbarIconStyleName()),
        FUIAction(FExecuteAction::CreateLambda([SelectedPaths]() { OnScanSelectedPaths(SelectedPaths); })),
        NAME_None,
        EUserInterfaceActionType::Button);

    MenuBuilder.AddSeparator();
    MenuBuilder.EndSection();
}

static TSharedRef<FExtender> OnContentBrowserExtendSelectedPathsMenu(const TArray<FString>& SelectedPaths)
{
    UE_LOG(RuleRanger,
           VeryVerbose,
           TEXT("FRuleRangerContentBrowserExtensions: OnContentBrowserExtendSelectedPathsMenu() invoked."));
    auto Extender = MakeShared<FExtender>();
    const auto MenuExtensionDelegate =
        FMenuExtensionDelegate::CreateStatic(&OnExtendContentBrowserForSelectedPaths, SelectedPaths);
    Extender->AddMenuExtension("PathContextBulkOperations", EExtensionHook::After, nullptr, MenuExtensionDelegate);
    return Extender;
}

static void OnScanSelectedAssets(const TArray<FAssetData>& SelectedAssets)
{
    if (const auto& Module = FModuleManager::GetModulePtr<FRuleRangerModule>(FRuleRangerModule::GetModuleName()))
    {
        // TODO: Replace this with dialog to perform scan
        UE_LOG(RuleRanger, Error, TEXT("OnScanSelectedAssets not implemented yet."));
    }
}

// ReSharper disable once CppPassValueParameterByConstReference
static void OnExtendContentBrowserForSelectedAssets(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets)
{
    UE_LOG(RuleRanger, VeryVerbose, TEXT("FRuleRangerContentBrowserExtensions: OnExtendAssetSelectionMenu() invoked."));

    MenuBuilder.BeginSection("RuleRangerContentBrowserContext",
                             NSLOCTEXT("RuleRanger", "ContextMenuSectionName", "Rule Ranger"));

    MenuBuilder.AddMenuEntry(
        NSLOCTEXT("RuleRanger", "ScanSelectedPaths", "Scan with RuleRanger"),
        NSLOCTEXT("RuleRanger", "ScanSelectedPaths", "Scan selected paths with RuleRanger"),
        FSlateIcon(FRuleRangerStyle::GetStyleSetName(), FRuleRangerStyle::GetToolbarIconStyleName()),
        FUIAction(FExecuteAction::CreateLambda([SelectedAssets]() { OnScanSelectedAssets(SelectedAssets); })),
        NAME_None,
        EUserInterfaceActionType::Button);

    MenuBuilder.AddSeparator();
    MenuBuilder.EndSection();
}

static TSharedRef<FExtender> OnContentBrowserExtendSelectedAssetsMenu(const TArray<FAssetData>& SelectedAssets)
{
    UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerModule: OnContentBrowserExtendSelectedAssetsMenu() invoked."));
    TSharedRef<FExtender> Extender = MakeShared<FExtender>();
    const auto MenuExtensionDelegate =
        FMenuExtensionDelegate::CreateStatic(&OnExtendContentBrowserForSelectedAssets, SelectedAssets);
    Extender->AddMenuExtension("CommonAssetActions", EExtensionHook::After, nullptr, MenuExtensionDelegate);
    return Extender;
}

void FRuleRangerContentBrowserExtensions::Initialize()
{
    auto& Module = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

    UE_LOG(RuleRanger,
           VeryVerbose,
           TEXT("FRuleRangerContentBrowserExtensions::Shutdown(): Registering ContentBrowser Extensions."));
    ContentBrowserMenuExtender_SelectedPathsDelegate =
        FContentBrowserMenuExtender_SelectedPaths::CreateStatic(&OnContentBrowserExtendSelectedPathsMenu);
    Module.GetAllPathViewContextMenuExtenders().Add(ContentBrowserMenuExtender_SelectedPathsDelegate);
    ContentBrowserMenuExtender_SelectedPathsDelegateHandle =
        ContentBrowserMenuExtender_SelectedPathsDelegate.GetHandle();
    check(ContentBrowserMenuExtender_SelectedPathsDelegateHandle.IsValid());

    // Asset extenders
    ContentBrowserMenuExtender_SelectedAssetsDelegate =
        FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&OnContentBrowserExtendSelectedAssetsMenu);
    Module.GetAllAssetViewContextMenuExtenders().Add(ContentBrowserMenuExtender_SelectedAssetsDelegate);
    ContentBrowserMenuExtender_SelectedAssetsDelegateHandle =
        ContentBrowserMenuExtender_SelectedAssetsDelegate.GetHandle();
    check(ContentBrowserMenuExtender_SelectedAssetsDelegateHandle.IsValid());
}

void FRuleRangerContentBrowserExtensions::Shutdown()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const FName ContentBrowserModuleName{ TEXT("ContentBrowser") };
    if (FModuleManager::Get().IsModuleLoaded(ContentBrowserModuleName))
    {
        auto& Module = FModuleManager::LoadModuleChecked<FContentBrowserModule>(ContentBrowserModuleName);
        if (ContentBrowserMenuExtender_SelectedPathsDelegateHandle.IsValid())
        {
            UE_LOG(RuleRanger,
                   VeryVerbose,
                   TEXT("FRuleRangerContentBrowserExtensions::Shutdown(): Deregistering ContentBrowser Extensions."));
            auto Target = ContentBrowserMenuExtender_SelectedPathsDelegateHandle;
            Module.GetAllPathViewContextMenuExtenders().RemoveAll(
                [&Target](const auto& Delegate) { return Delegate.GetHandle() == Target; });
            Target.Reset();
        }
        else
        {
            UE_LOG(RuleRanger,
                   Verbose,
                   TEXT("FRuleRangerContentBrowserExtensions::Shutdown(): Skipping deregister of "
                        "ContentBrowserExtensions as handle is Invalid."));
        }
    }
}
