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
#include "RuleRanger/RuleRangerEditorSubsystem.h"
#include "RuleRangerStyle.h"
#include "ToolMenus.h"

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

    if (const auto ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools"))
    {
        auto& Section = ToolsMenu->AddSection("RuleRangerToolsSection",
                                              NSLOCTEXT("RuleRanger", "ToolsSectionHeading", "Rule Ranger"));

        Section.AddSubMenu("RuleRangerSubMenu",
                           NSLOCTEXT("RuleRanger", "RuleRangerToolsMenu", "Rule Ranger"),
                           NSLOCTEXT("RuleRanger", "RuleRangerToolsMenu_Tooltip", "Rule Ranger operations"),
                           FNewToolMenuDelegate::CreateStatic(&FRuleRangerToolsMenu::FillRuleRangerSubMenu),
                           false,
                           FSlateIcon(FRuleRangerStyle::GetStyleSetName(), TEXT("RuleRanger.ScanSelectedPaths")));
    }
}

void FRuleRangerToolsMenu::FillRuleRangerSubMenu(UToolMenu* Menu)
{
    auto& SubSection =
        Menu->AddSection("RuleRangerToolsSubSection", NSLOCTEXT("RuleRanger", "RuleRangerSection", "Actions"));

    {
        auto Entry = FToolMenuEntry::InitMenuEntry(
            TEXT("RuleRanger.ScanConfiguredContent"),
            NSLOCTEXT("RuleRanger", "ScanConfiguredContent", "Scan Content"),
            NSLOCTEXT("RuleRanger", "ScanConfiguredContent_Tooltip", "Scan content in configured directories"),
            FSlateIcon(FRuleRangerStyle::GetStyleSetName(), TEXT("RuleRanger.ScanProjectContent")),
            FUIAction(FExecuteAction::CreateStatic(&FRuleRangerToolsMenu::OnScanConfiguredContent)));
        SubSection.AddEntry(MoveTemp(Entry));
    }

    {
        auto Entry = FToolMenuEntry::InitMenuEntry(
            TEXT("RuleRanger.FixConfiguredContent"),
            NSLOCTEXT("RuleRanger", "FixConfiguredContent", "Scan & Fix Content"),
            NSLOCTEXT("RuleRanger", "FixConfiguredContent_Tooltip", "Scan and apply fixes in configured directories"),
            FSlateIcon(FRuleRangerStyle::GetStyleSetName(), TEXT("RuleRanger.FixProjectContent")),
            FUIAction(FExecuteAction::CreateStatic(&FRuleRangerToolsMenu::OnFixConfiguredContent)));
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
