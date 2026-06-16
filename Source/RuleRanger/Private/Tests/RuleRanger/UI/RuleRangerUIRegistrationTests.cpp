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
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "AssetRegistry/AssetData.h"
    #include "Framework/Docking/TabManager.h"
    #include "Framework/MultiBox/MultiBoxBuilder.h"
    #include "Framework/MultiBox/MultiBoxExtender.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRanger/UI/ContentBrowserExtension/RuleRangerContentBrowserExtensions.h"
    #include "RuleRanger/UI/RuleRangerCommands.h"
    #include "RuleRanger/UI/RuleRangerStyle.h"
    #include "RuleRanger/UI/RuleRangerUIHelpers.h"
    #include "RuleRanger/UI/ToolTab/RuleRangerToolTab.h"
    #include "RuleRanger/UI/ToolsMenu/RuleRangerToolsMenu.h"
    #include "Styling/AppStyle.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "ToolMenu.h"
    #include "ToolMenus.h"

namespace RuleRangerUIRegistrationTests
{
    struct FScopedCommandRegistrationRestorer
    {
        const bool bWasRegistered{ FRuleRangerCommands::IsRegistered() };

        ~FScopedCommandRegistrationRestorer()
        {
            if (bWasRegistered && !FRuleRangerCommands::IsRegistered())
            {
                FRuleRangerCommands::Register();
            }
            else if (!bWasRegistered && FRuleRangerCommands::IsRegistered())
            {
                FRuleRangerCommands::Unregister();
            }
        }
    };

    struct FScopedStyleRegistrationRestorer
    {
        const bool bWasInitialized{ FRuleRangerStyle::IsInitializedForTest() };

        ~FScopedStyleRegistrationRestorer()
        {
            if (bWasInitialized && !FRuleRangerStyle::IsInitializedForTest())
            {
                FRuleRangerStyle::Initialize();
            }
            else if (!bWasInitialized && FRuleRangerStyle::IsInitializedForTest())
            {
                FRuleRangerStyle::Shutdown();
            }
        }
    };

    struct FScopedContentBrowserExtensionRestorer
    {
        const bool bWasRegistered{ FRuleRangerContentBrowserExtensions::AreExtendersRegisteredForTest() };

        ~FScopedContentBrowserExtensionRestorer()
        {
            if (bWasRegistered && !FRuleRangerContentBrowserExtensions::AreExtendersRegisteredForTest())
            {
                FRuleRangerContentBrowserExtensions::Initialize();
            }
            else if (!bWasRegistered && FRuleRangerContentBrowserExtensions::AreExtendersRegisteredForTest())
            {
                FRuleRangerContentBrowserExtensions::Shutdown();
            }
        }
    };

    struct FScopedToolTabRegistrationRestorer
    {
        const bool bWasRegistered{ FRuleRangerToolTab::IsRegisteredForTest() };

        ~FScopedToolTabRegistrationRestorer()
        {
            if (bWasRegistered && !FRuleRangerToolTab::IsRegisteredForTest())
            {
                FRuleRangerToolTab::Initialize();
            }
            else if (!bWasRegistered && FRuleRangerToolTab::IsRegisteredForTest())
            {
                FRuleRangerToolTab::Shutdown();
            }
        }
    };

    struct FScopedToolsMenuRegistrationRestorer
    {
        const bool bHadMenu{ UToolMenus::Get() && UToolMenus::Get()->FindMenu(TEXT("MainFrame.MainMenu.Tools")) };

        ~FScopedToolsMenuRegistrationRestorer()
        {
            FRuleRangerToolsMenu::Shutdown();
            if (bHadMenu)
            {
                FRuleRangerToolsMenu::Initialize();
            }
        }
    };

    bool TestCommandInfo(FAutomationTestBase& Test,
                         const TSharedPtr<FUICommandInfo>& Command,
                         const TCHAR* Description,
                         const TCHAR* ExpectedLabel)
    {
        return Test.TestTrue(Description, Command.IsValid())
            && Test.TestEqual(TEXT("Command label should match"),
                              Command->GetLabel().ToString(),
                              FString(ExpectedLabel))
            && Test.TestEqual(TEXT("Command type should be a button"),
                              Command->GetUserInterfaceType(),
                              EUserInterfaceActionType::Button);
    }
} // namespace RuleRangerUIRegistrationTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCommandsRegisterUnregisterIdempotentlyTest,
                                 "RuleRanger.UI.Commands.RegisterUnregisterIdempotently",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCommandsRegisterUnregisterIdempotentlyTest::RunTest(const FString&)
{
    RuleRangerUIRegistrationTests::FScopedCommandRegistrationRestorer RegistrationRestorer;

    FRuleRangerCommands::Register();
    FRuleRangerCommands::Register();

    auto& Commands = FRuleRangerCommands::Get();
    const bool bCommandsRegistered =
        TestTrue(TEXT("Commands should be registered"), FRuleRangerCommands::IsRegistered())
        && RuleRangerUIRegistrationTests::TestCommandInfo(*this,
                                                          Commands.ScanSelectedPaths,
                                                          TEXT("Scan selected paths command should be valid"),
                                                          TEXT("Scan with RuleRanger"))
        && RuleRangerUIRegistrationTests::TestCommandInfo(*this,
                                                          Commands.FixSelectedPaths,
                                                          TEXT("Fix selected paths command should be valid"),
                                                          TEXT("Apply fixes with RuleRanger"))
        && RuleRangerUIRegistrationTests::TestCommandInfo(*this,
                                                          Commands.ScanSelectedAssets,
                                                          TEXT("Scan selected assets command should be valid"),
                                                          TEXT("Scan with RuleRanger"))
        && RuleRangerUIRegistrationTests::TestCommandInfo(*this,
                                                          Commands.FixSelectedAssets,
                                                          TEXT("Fix selected assets command should be valid"),
                                                          TEXT("Apply fixes with RuleRanger"))
        && RuleRangerUIRegistrationTests::TestCommandInfo(*this,
                                                          Commands.ScanProjectContent,
                                                          TEXT("Scan project content command should be valid"),
                                                          TEXT("Scan Content"))
        && RuleRangerUIRegistrationTests::TestCommandInfo(*this,
                                                          Commands.FixProjectContent,
                                                          TEXT("Fix project content command should be valid"),
                                                          TEXT("Scan & Fix Content"));

    FRuleRangerCommands::Unregister();
    FRuleRangerCommands::Unregister();

    return bCommandsRegistered
        && TestFalse(TEXT("Commands should be unregistered after shutdown"), FRuleRangerCommands::IsRegistered());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerStyleRegistersExpectedIconsAndShutsDownTwiceTest,
                                 "RuleRanger.UI.Style.RegistersExpectedIconsAndShutsDownTwice",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerStyleRegistersExpectedIconsAndShutsDownTwiceTest::RunTest(const FString&)
{
    RuleRangerUIRegistrationTests::FScopedStyleRegistrationRestorer StyleRestorer;

    FRuleRangerStyle::Initialize();
    FRuleRangerStyle::Initialize();

    const bool bStyleInitialized =
        TestTrue(TEXT("Style should be initialized"), FRuleRangerStyle::IsInitializedForTest())
        && TestEqual(TEXT("RuleRanger style name should be stable"),
                     FRuleRangerStyle::GetStyleSetName(),
                     FName(TEXT("RuleRangerStyle")))
        && TestEqual(TEXT("Settings icon should come from app style"),
                     FRuleRangerStyle::GetSettingsIcon().GetStyleSetName(),
                     FAppStyle::GetAppStyleSetName())
        && TestEqual(TEXT("Scan icon style should be stable"),
                     FRuleRangerStyle::GetScanIcon().GetStyleName(),
                     FName(TEXT("Symbols.SearchGlass")))
        && TestEqual(TEXT("Scan-and-fix icon style should be stable"),
                     FRuleRangerStyle::GetScanAndFixIcon().GetStyleName(),
                     FName(TEXT("MessageLog.Fix")))
        && TestNotNull(TEXT("Info brush should resolve"), FRuleRangerStyle::GetNoteMessageBrush())
        && TestNotNull(TEXT("Warning brush should resolve"), FRuleRangerStyle::GetWarningMessageBrush())
        && TestNotNull(TEXT("Error brush should resolve"), FRuleRangerStyle::GetErrorMessageBrush());

    FRuleRangerStyle::Shutdown();
    FRuleRangerStyle::Shutdown();

    return bStyleInitialized
        && TestFalse(TEXT("Style should be uninitialized after repeated shutdown"),
                     FRuleRangerStyle::IsInitializedForTest());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerContentBrowserExtensionsRegisterAndCreateExtendersTest,
                                 "RuleRanger.UI.ContentBrowserExtensions.RegisterAndCreateExtenders",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerContentBrowserExtensionsRegisterAndCreateExtendersTest::RunTest(const FString&)
{
    RuleRangerUIRegistrationTests::FScopedCommandRegistrationRestorer CommandRestorer;
    RuleRangerUIRegistrationTests::FScopedContentBrowserExtensionRestorer ExtensionRestorer;
    RuleRangerTests::FScopedRuleRangerDeveloperSettingsOverride SettingsOverride({});

    FRuleRangerCommands::Register();
    FRuleRangerContentBrowserExtensions::Shutdown();

    const bool bShutdownClearsHandles =
        TestFalse(TEXT("Content browser extension handles should be cleared after shutdown"),
                  FRuleRangerContentBrowserExtensions::AreExtendersRegisteredForTest());

    FRuleRangerContentBrowserExtensions::Initialize();
    FRuleRangerContentBrowserExtensions::Initialize();
    const bool bInitializeSetsHandles =
        TestTrue(TEXT("Content browser extension handles should be valid after initialize"),
                 FRuleRangerContentBrowserExtensions::AreExtendersRegisteredForTest());

    const auto PathExtender =
        FRuleRangerContentBrowserExtensions::CreateSelectedPathsMenuExtenderForTest({ TEXT("/Game/Outside") });
    const auto AssetObject = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(
        TEXT("/Game/RuleRangerUIRegistration/AssetMenu"),
        TEXT("AssetMenu"));
    const auto AssetExtender = TestNotNull(TEXT("Asset object should be created"), AssetObject)
        ? FRuleRangerContentBrowserExtensions::CreateSelectedAssetsMenuExtenderForTest({ FAssetData(AssetObject) })
        : MakeShared<FExtender>();

    FRuleRangerContentBrowserExtensions::Shutdown();
    FRuleRangerContentBrowserExtensions::Shutdown();

    return bShutdownClearsHandles && bInitializeSetsHandles
        && TestEqual(TEXT("Path context extender should contain one menu extension"), PathExtender->NumExtensions(), 1)
        && TestEqual(TEXT("Asset context extender should contain one menu extension"),
                     AssetExtender->NumExtensions(),
                     1)
        && TestFalse(TEXT("Content browser extension handles should stay cleared after repeated shutdown"),
                     FRuleRangerContentBrowserExtensions::AreExtendersRegisteredForTest());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolsMenuBuildsExpectedEntriesTest,
                                 "RuleRanger.UI.ToolsMenu.BuildsExpectedEntries",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolsMenuBuildsExpectedEntriesTest::RunTest(const FString&)
{
    RuleRangerTests::FScopedRuleRangerDeveloperSettingsOverride SettingsOverride({});

    const auto Menu = NewObject<UToolMenu>();
    if (!TestNotNull(TEXT("Tool menu should be created"), Menu))
    {
        return false;
    }

    Menu->InitMenu(FToolMenuOwner(TEXT("RuleRangerTests")), TEXT("RuleRanger.Tests.ToolsMenu"));
    FRuleRangerToolsMenu::FillRuleRangerSubMenuForTest(Menu);

    return TestTrue(TEXT("Settings entry should be present"),
                    Menu->ContainsEntry(TEXT("RuleRanger.OpenProjectSettings")))
        && TestTrue(TEXT("Tool window entry should be present"), Menu->ContainsEntry(TEXT("RuleRanger.OpenTool")))
        && TestTrue(TEXT("Scan all entry should be present"), Menu->ContainsEntry(TEXT("RuleRanger.ScanAll")))
        && TestTrue(TEXT("Fix all entry should be present"), Menu->ContainsEntry(TEXT("RuleRanger.FixAll")))
        && TestTrue(TEXT("Scan project entry should be present"), Menu->ContainsEntry(TEXT("RuleRanger.ScanProject")))
        && TestTrue(TEXT("Fix project entry should be present"), Menu->ContainsEntry(TEXT("RuleRanger.FixProject")))
        && TestTrue(TEXT("Scan content entry should be present"),
                    Menu->ContainsEntry(TEXT("RuleRanger.ScanConfiguredContent")))
        && TestTrue(TEXT("Fix content entry should be present"),
                    Menu->ContainsEntry(TEXT("RuleRanger.FixConfiguredContent")))
        && TestTrue(TEXT("No configured dirs hint should be present"),
                    Menu->ContainsEntry(TEXT("RuleRanger.NoConfiguredDirsHint")));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolsMenuRegistersAndShutsDownIdempotentlyTest,
                                 "RuleRanger.UI.ToolsMenu.RegistersAndShutsDownIdempotently",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolsMenuRegistersAndShutsDownIdempotentlyTest::RunTest(const FString&)
{
    RuleRangerUIRegistrationTests::FScopedToolsMenuRegistrationRestorer ToolsMenuRestorer;

    FRuleRangerToolsMenu::Initialize();
    FRuleRangerToolsMenu::Initialize();

    const auto MainToolsMenu = UToolMenus::Get()->FindMenu(TEXT("MainFrame.MainMenu.Tools"));
    const bool bMenuRegistered = TestNotNull(TEXT("Main Tools menu should be available"), MainToolsMenu)
        && TestTrue(TEXT("RuleRanger submenu should be present"),
                    MainToolsMenu->ContainsEntry(TEXT("RuleRangerSubMenu")));

    FRuleRangerToolsMenu::Shutdown();
    FRuleRangerToolsMenu::Shutdown();

    return bMenuRegistered;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolTabRegistersAndShutsDownIdempotentlyTest,
                                 "RuleRanger.UI.ToolTab.RegistersAndShutsDownIdempotently",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolTabRegistersAndShutsDownIdempotentlyTest::RunTest(const FString&)
{
    RuleRangerUIRegistrationTests::FScopedToolTabRegistrationRestorer TabRestorer;

    FRuleRangerToolTab::Shutdown();
    const bool bShutdownClearsSpawner =
        TestFalse(TEXT("Tool tab spawner should be absent after shutdown"), FRuleRangerToolTab::IsRegisteredForTest());

    FRuleRangerToolTab::Initialize();
    FRuleRangerToolTab::Initialize();
    const bool bInitializeRegistersSpawner =
        TestTrue(TEXT("Tool tab spawner should be present after initialize"), FRuleRangerToolTab::IsRegisteredForTest())
        && TestEqual(TEXT("Tool tab name should be stable"),
                     FRuleRangerToolTab::GetTabName(),
                     FName(TEXT("RuleRangerTool")));

    FRuleRangerToolTab::Shutdown();
    FRuleRangerToolTab::Shutdown();

    return bShutdownClearsSpawner && bInitializeRegistersSpawner
        && TestFalse(TEXT("Tool tab spawner should stay absent after repeated shutdown"),
                     FRuleRangerToolTab::IsRegisteredForTest());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerUIHelpersConstructWidgetsAndMenuEntriesTest,
                                 "RuleRanger.UI.Helpers.ConstructWidgetsAndMenuEntries",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerUIHelpersConstructWidgetsAndMenuEntriesTest::RunTest(const FString&)
{
    bool bExecuted = false;

    const auto IconLabel =
        RuleRangerUI::MakeIconLabel(FRuleRangerStyle::GetScanIcon(), FText::FromString(TEXT("Scan")));
    const auto BrushLabel = RuleRangerUI::MakeIconLabelFromBrush(FRuleRangerStyle::GetWarningMessageBrush(),
                                                                 FText::FromString(TEXT("Warning")));

    FMenuBuilder MenuBuilder(true, nullptr);
    RuleRangerUI::AddMenuEntry(MenuBuilder,
                               FText::FromString(TEXT("Run")),
                               FText::FromString(TEXT("Run tooltip")),
                               FRuleRangerStyle::GetScanIcon(),
                               FExecuteAction::CreateLambda([&bExecuted] { bExecuted = true; }),
                               FCanExecuteAction::CreateLambda([] { return true; }));

    const auto MenuWidget = MenuBuilder.MakeWidget();

    return TestTrue(TEXT("Icon-label widget should be valid"), IconLabel->GetVisibility().IsVisible())
        && TestTrue(TEXT("Brush-label widget should be valid"), BrushLabel->GetVisibility().IsVisible())
        && TestTrue(TEXT("Menu widget should be valid"), MenuWidget->GetVisibility().IsVisible())
        && TestFalse(TEXT("Constructing a menu entry should not execute it"), bExecuted);
}

#endif
