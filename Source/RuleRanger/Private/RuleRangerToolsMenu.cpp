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

#include "RuleRangerToolsMenu.h"
#include "Editor.h"
#include "Framework/Docking/TabManager.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "RuleRanger/RuleRangerEditorSubsystem.h"
#include "RuleRangerDeveloperSettings.h"
#include "RuleRangerStyle.h"
#include "ToolMenus.h"
#include "Widgets/Text/STextBlock.h"

FDelegateHandle FRuleRangerToolsMenu::RegisterHandle;
int32 FRuleRangerToolsMenu::OwnerToken = 0;

void FRuleRangerToolsMenu::Initialize()
{
    RegisterHandle = UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateStatic(&FRuleRangerToolsMenu::RegisterMenus));
    if (UToolMenus::Get())
    {
        RegisterMenus();
    }
}

void FRuleRangerToolsMenu::Shutdown()
{
    if (RegisterHandle.IsValid())
    {
        UToolMenus::UnRegisterStartupCallback(RegisterHandle);
        RegisterHandle.Reset();
    }
    if (const auto ToolMenus = UToolMenus::Get())
    {
        ToolMenus->UnregisterOwner(&OwnerToken);
    }
}

void FRuleRangerToolsMenu::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(&OwnerToken);

    if (const auto ToolsMenu = UToolMenus::Get()->ExtendMenu("MainFrame.MainMenu.Tools"))
    {
        auto& Section = ToolsMenu->AddSection("RuleRangerToolsSection",
                                              NSLOCTEXT("RuleRanger", "ToolsSectionHeading", "Rule Ranger"));

        Section.AddSubMenu("RuleRangerSubMenu",
                           NSLOCTEXT("RuleRanger", "RuleRangerToolsMenu", "Rule Ranger"),
                           NSLOCTEXT("RuleRanger", "RuleRangerToolsMenu_Tooltip", "Rule Ranger operations"),
                           FNewToolMenuDelegate::CreateStatic(&FRuleRangerToolsMenu::FillRuleRangerSubMenu),
                           false,
                           FRuleRangerStyle::GetScanIcon());
    }
}

bool FRuleRangerToolsMenu::HasAnyConfiguredDirs()
{
    const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>();
    return Subsystem ? Subsystem->HasAnyConfiguredDirs() : false;
}

bool FRuleRangerToolsMenu::HasAnyProjectRules()
{
    const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>();
    return Subsystem ? Subsystem->HasAnyProjectRules() : false;
}

void FRuleRangerToolsMenu::FillRuleRangerSubMenu(UToolMenu* Menu)
{
    auto& SubSection =
        Menu->AddSection("RuleRangerToolsSubSection", NSLOCTEXT("RuleRanger", "RuleRangerSection", "Actions"));

    if (!HasAnyConfiguredDirs())
    {
        const auto InfoText =
            NSLOCTEXT("RuleRanger",
                      "NoConfiguredDirsHint",
                      "No Rule Ranger directories configured.\nSet them in Project Settings → Editor → Rule Ranger.");
        SubSection.AddEntry(
            FToolMenuEntry::InitWidget(TEXT("RuleRanger.NoConfiguredDirsHint"),
                                       SNew(STextBlock)
                                           .Text(InfoText)
                                           .WrapTextAt(380.f)
                                           .Margin(FMargin(20.f, 4.f, 20.f, 4.f))
                                           .ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f))),
                                       FText::GetEmpty(),
                                       true));
    }

    // Always-present convenience action
    {
        auto Entry = FToolMenuEntry::InitMenuEntry(
            TEXT("RuleRanger.OpenProjectSettings"),
            NSLOCTEXT("RuleRanger", "OpenProjectSettings", "Open Project Settings…"),
            NSLOCTEXT("RuleRanger",
                      "OpenProjectSettings_Tooltip",
                      "Open Project Settings to the Editor → Rule Ranger page"),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateStatic(&FRuleRangerToolsMenu::OnOpenProjectSettings)));
        SubSection.AddEntry(MoveTemp(Entry));
    }

    SubSection.AddSeparator(NAME_None);

    // Project-level actions
    {
        auto Entry = FToolMenuEntry::InitMenuEntry(
            TEXT("RuleRanger.ScanProject"),
            NSLOCTEXT("RuleRanger", "ScanProject", "Scan Project"),
            NSLOCTEXT("RuleRanger",
                      "ScanProject_Tooltip",
                      "Execute project-level rules defined in configured RuleRanger rule sets\n"
                      "(Disabled if no project rules exist)"),
            FRuleRangerStyle::GetScanIcon(),
            FUIAction(FExecuteAction::CreateStatic(&FRuleRangerToolsMenu::OnScanProject),
                      FCanExecuteAction::CreateLambda([] { return HasAnyProjectRules(); })));
        SubSection.AddEntry(MoveTemp(Entry));
    }

    {
        auto Entry = FToolMenuEntry::InitMenuEntry(
            TEXT("RuleRanger.FixProject"),
            NSLOCTEXT("RuleRanger", "FixProject", "Scan & Fix Project"),
            NSLOCTEXT("RuleRanger",
                      "FixProject_Tooltip",
                      "Execute project-level rules and apply fixes where supported\n"
                      "(Disabled if no project rules exist)"),
            FRuleRangerStyle::GetScanAndFixIcon(),
            FUIAction(FExecuteAction::CreateStatic(&FRuleRangerToolsMenu::OnFixProject),
                      FCanExecuteAction::CreateLambda([] { return HasAnyProjectRules(); })));
        SubSection.AddEntry(MoveTemp(Entry));
    }

    SubSection.AddSeparator(NAME_None);

    // Content actions
    {
        auto Entry = FToolMenuEntry::InitMenuEntry(
            TEXT("RuleRanger.ScanConfiguredContent"),
            NSLOCTEXT("RuleRanger", "ScanConfiguredContent", "Scan Content"),
            NSLOCTEXT(
                "RuleRanger",
                "ScanConfiguredContent_Tooltip",
                "Scan content in configured directories (configure under Project Settings → Editor → Rule Ranger)\n"
                "(Disabled if no directories are configured)"),
            FRuleRangerStyle::GetScanIcon(),
            FUIAction(FExecuteAction::CreateStatic(&FRuleRangerToolsMenu::OnScanConfiguredContent),
                      FCanExecuteAction::CreateLambda([] { return HasAnyConfiguredDirs(); })));
        SubSection.AddEntry(MoveTemp(Entry));
    }

    {
        auto Entry = FToolMenuEntry::InitMenuEntry(
            TEXT("RuleRanger.FixConfiguredContent"),
            NSLOCTEXT("RuleRanger", "FixConfiguredContent", "Scan & Fix Content"),
            NSLOCTEXT(
                "RuleRanger",
                "FixConfiguredContent_Tooltip",
                "Scan and apply fixes in configured directories (configure under Project Settings → Editor → Rule Ranger)\n"
                "(Disabled if no directories are configured)"),
            FRuleRangerStyle::GetScanAndFixIcon(),
            FUIAction(FExecuteAction::CreateStatic(&FRuleRangerToolsMenu::OnFixConfiguredContent),
                      FCanExecuteAction::CreateLambda([] { return HasAnyConfiguredDirs(); })));
        SubSection.AddEntry(MoveTemp(Entry));
    }
}

void FRuleRangerToolsMenu::OnScanConfiguredContent()
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnScanConfiguredContent();
    }
}

void FRuleRangerToolsMenu::OnFixConfiguredContent()
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnFixConfiguredContent();
    }
}

void FRuleRangerToolsMenu::OnScanProject()
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnScanProject();
    }
}

void FRuleRangerToolsMenu::OnFixProject()
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        Subsystem->OnFixProject();
    }
}

void FRuleRangerToolsMenu::OnOpenProjectSettings()
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
