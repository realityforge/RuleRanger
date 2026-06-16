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
    #include "Misc/AutomationTest.h"
    #include "RuleRanger/UI/RuleRangerTools.h"
    #include "RuleRangerConfig.h"
    #include "RuleRangerProjectRule.h"
    #include "RuleRangerRuleSet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerToolsTests
{
    static URuleRangerConfig* CreateConfiguredDirConfig(FAutomationTestBase& Test)
    {
        const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
        if (Test.TestNotNull(TEXT("Tool config should be created"), Config))
        {
            Config->Dirs.Add({ TEXT("/Game/RuleRangerTools/") });
            return Config;
        }
        return nullptr;
    }

    static URuleRangerConfig* CreateProjectRuleConfig(FAutomationTestBase& Test)
    {
        const auto Config = CreateConfiguredDirConfig(Test);
        const auto RuleSet =
            Config ? RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("ToolRuleSet")) : nullptr;
        const auto Rule =
            RuleSet ? RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(RuleSet, TEXT("ToolRule")) : nullptr;
        if (Test.TestNotNull(TEXT("Tool rule set should be created"), RuleSet)
            && Test.TestNotNull(TEXT("Tool project rule should be created"), Rule))
        {
            RuleSet->ProjectRules = { Rule };
            Config->RuleSets = { RuleSet };
            return Config;
        }
        return nullptr;
    }
} // namespace RuleRangerToolsTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolsConfiguredDirEnablementReflectsSubsystemStateTest,
                                 "RuleRanger.UI.Tools.ConfiguredDirEnablementReflectsSubsystemState",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolsConfiguredDirEnablementReflectsSubsystemStateTest::RunTest(const FString&)
{
    const auto Config = RuleRangerToolsTests::CreateProjectRuleConfig(*this);
    if (!TestNotNull(TEXT("Tool config should be available"), Config))
    {
        return false;
    }

    RuleRangerTests::FScopedRuleRangerDeveloperSettingsOverride SettingsOverride({ Config });

    return TestTrue(TEXT("Configured content should enable content scans"), FRuleRangerTools::CanRunScanContent())
        && TestTrue(TEXT("Configured project rules should enable project scans"), FRuleRangerTools::CanRunScanProject())
        && TestTrue(TEXT("Either content or project scans should enable scan-all"), FRuleRangerTools::CanRunScanAll());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolsSelectionPredicatesIntersectConfiguredDirsTest,
                                 "RuleRanger.UI.Tools.SelectionPredicatesIntersectConfiguredDirs",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolsSelectionPredicatesIntersectConfiguredDirsTest::RunTest(const FString&)
{
    const auto Config = RuleRangerToolsTests::CreateConfiguredDirConfig(*this);
    if (!TestNotNull(TEXT("Tool config should be available"), Config))
    {
        return false;
    }

    RuleRangerTests::FScopedRuleRangerDeveloperSettingsOverride SettingsOverride({ Config });
    const auto MatchingObject = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(
        TEXT("/Game/RuleRangerTools/AssetSelection"),
        TEXT("AssetSelection"));
    const auto NonMatchingObject = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(
        TEXT("/Game/RuleRangerOther/AssetSelection"),
        TEXT("AssetSelection"));
    if (!TestNotNull(TEXT("Matching object should be created"), MatchingObject)
        || !TestNotNull(TEXT("Non-matching object should be created"), NonMatchingObject))
    {
        return false;
    }

    return TestTrue(TEXT("Matching asset selection should intersect configured dirs"),
                    FRuleRangerTools::AssetSelectionIntersectsConfiguredDirs({ FAssetData(MatchingObject) }))
        && TestFalse(TEXT("Non-matching asset selection should not intersect configured dirs"),
                     FRuleRangerTools::AssetSelectionIntersectsConfiguredDirs({ FAssetData(NonMatchingObject) }))
        && TestTrue(TEXT("Child folder selection should intersect configured dirs"),
                    FRuleRangerTools::PathSelectionIntersectsConfiguredDirs({ TEXT("/Game/RuleRangerTools/Child") }))
        && TestTrue(TEXT("Parent folder selection should intersect configured dirs"),
                    FRuleRangerTools::PathSelectionIntersectsConfiguredDirs({ TEXT("/Game") }))
        && TestFalse(TEXT("Unrelated folder selection should not intersect configured dirs"),
                     FRuleRangerTools::PathSelectionIntersectsConfiguredDirs({ TEXT("/Engine") }));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolsExplicitEmptySelectionsDoNotCrashTest,
                                 "RuleRanger.UI.Tools.ExplicitEmptySelectionsDoNotCrash",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolsExplicitEmptySelectionsDoNotCrashTest::RunTest(const FString&)
{
    const auto Config = RuleRangerToolsTests::CreateConfiguredDirConfig(*this);
    if (!TestNotNull(TEXT("Tool config should be available"), Config))
    {
        return false;
    }

    RuleRangerTests::FScopedRuleRangerDeveloperSettingsOverride SettingsOverride({ Config });

    FRuleRangerTools::OnScanSelectedAssets({});
    FRuleRangerTools::OnFixSelectedAssets({});
    FRuleRangerTools::OnScanSelectedPaths({});
    FRuleRangerTools::OnFixSelectedPaths({});

    return true;
}

#endif
