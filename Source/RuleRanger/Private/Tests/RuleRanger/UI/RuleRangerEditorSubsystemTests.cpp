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

    #include "Misc/AutomationTest.h"
    #include "RuleRanger/UI/RuleRangerEditorSubsystem.h"
    #include "RuleRangerActionContext.h"
    #include "RuleRangerConfig.h"
    #include "RuleRangerProjectRule.h"
    #include "RuleRangerRule.h"
    #include "RuleRangerRuleSet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

class FRuleRangerEditorSubsystemTestAccessor
{
public:
    static bool IsRuleSetConfigCacheDirty(const URuleRangerEditorSubsystem* const Subsystem)
    {
        return Subsystem->bRuleSetConfigCacheDirty;
    }

    static bool IsValidationMatchCacheEmpty(const URuleRangerEditorSubsystem* const Subsystem)
    {
        return Subsystem->ValidationMatchCache.IsEmpty() && !Subsystem->ValidationMatchCacheObject.IsValid();
    }
};

namespace RuleRangerEditorSubsystemTests
{
    struct FAssetRuleFixture
    {
        TObjectPtr<URuleRangerConfig> Config{ nullptr };
        TObjectPtr<URuleRangerRuleSet> RuleSet{ nullptr };
        TObjectPtr<URuleRangerRule> Rule{ nullptr };
        TObjectPtr<URuleRangerAutomationTestAction> Action{ nullptr };
        TObjectPtr<URuleRangerAutomationTestMatcher> Matcher{ nullptr };
        TObjectPtr<URuleRangerAutomationTestObject> Object{ nullptr };
    };

    static bool CreateAssetRuleFixture(FAutomationTestBase& Test, FAssetRuleFixture& OutFixture)
    {
        OutFixture.Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
        OutFixture.RuleSet =
            RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(OutFixture.Config, TEXT("RuleSet"));
        OutFixture.Rule = RuleRangerTests::NewTransientObject<URuleRangerRule>(OutFixture.RuleSet, TEXT("Rule"));
        OutFixture.Action =
            RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(OutFixture.Rule, TEXT("Action"));
        OutFixture.Matcher =
            RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(OutFixture.Rule, TEXT("Matcher"));
        OutFixture.Object = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(
            TEXT("/Game/Developers/Tests/RuleRanger/EditorSubsystem/AssetObject"),
            TEXT("AssetObject"));

        if (!Test.TestNotNull(TEXT("Config should be created"), OutFixture.Config.Get())
            || !Test.TestNotNull(TEXT("Rule set should be created"), OutFixture.RuleSet.Get())
            || !Test.TestNotNull(TEXT("Rule should be created"), OutFixture.Rule.Get())
            || !Test.TestNotNull(TEXT("Action should be created"), OutFixture.Action.Get())
            || !Test.TestNotNull(TEXT("Matcher should be created"), OutFixture.Matcher.Get())
            || !Test.TestNotNull(TEXT("Object should be created"), OutFixture.Object.Get()))
        {
            return false;
        }

        OutFixture.Config->Dirs.Add({ TEXT("/Game/Developers/Tests/RuleRanger/EditorSubsystem/") });
        OutFixture.Rule->Matchers = { OutFixture.Matcher };
        OutFixture.Rule->Actions = { OutFixture.Action };
        OutFixture.RuleSet->Rules = { OutFixture.Rule };
        OutFixture.Config->RuleSets = { OutFixture.RuleSet };
        return true;
    }

    static URuleRangerConfig* CreateProjectRuleConfig(FAutomationTestBase& Test,
                                                      URuleRangerAutomationTestProjectAction*& OutAction)
    {
        const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
        const auto RuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("ProjectRuleSet"));
        const auto Rule = RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(RuleSet, TEXT("ProjectRule"));
        OutAction =
            RuleRangerTests::NewTransientObject<URuleRangerAutomationTestProjectAction>(Rule, TEXT("ProjectAction"));
        if (!Test.TestNotNull(TEXT("Project config should be created"), Config)
            || !Test.TestNotNull(TEXT("Project rule set should be created"), RuleSet)
            || !Test.TestNotNull(TEXT("Project rule should be created"), Rule)
            || !Test.TestNotNull(TEXT("Project action should be created"), OutAction))
        {
            return nullptr;
        }

        Rule->Actions = { OutAction };
        RuleSet->ProjectRules = { Rule };
        Config->RuleSets = { RuleSet };
        return Config;
    }
} // namespace RuleRangerEditorSubsystemTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorSubsystemScanAndFixForwardToRuleEngineTest,
                                 "RuleRanger.UI.EditorSubsystem.ScanAndFixForwardToRuleEngine",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorSubsystemScanAndFixForwardToRuleEngineTest::RunTest(const FString&)
{
    const auto Subsystem = GEditor ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>() : nullptr;
    RuleRangerEditorSubsystemTests::FAssetRuleFixture Fixture;
    if (!TestNotNull(TEXT("RuleRanger editor subsystem should be available"), Subsystem)
        || !RuleRangerEditorSubsystemTests::CreateAssetRuleFixture(*this, Fixture))
    {
        return false;
    }

    RuleRangerTests::FScopedRuleRangerDeveloperSettingsOverride SettingsOverride({ Fixture.Config });
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerAutomationCapturingResultHandler>();
    if (!TestNotNull(TEXT("Capturing result handler should be created"), Handler))
    {
        return false;
    }

    Subsystem->ScanObject(Fixture.Object, Handler);
    const bool bScanForwarded =
        TestEqual(TEXT("Demand scan should apply the rule once"), Fixture.Action->GetApplyCount(), 1)
        && TestEqual(TEXT("Demand scan should report through the handler"), Handler->CallCount, 1)
        && TestEqual(TEXT("Demand scan should use the report trigger"),
                     Fixture.Action->GetLastTrigger(),
                     ERuleRangerActionTrigger::AT_Report);

    Subsystem->ScanAndFixObject(Fixture.Object, Handler);
    return bScanForwarded
        && TestEqual(TEXT("Demand fix should apply the rule again"), Fixture.Action->GetApplyCount(), 2)
        && TestEqual(TEXT("Demand fix should report through the handler"), Handler->CallCount, 2)
        && TestEqual(TEXT("Demand fix should use the fix trigger"),
                     Fixture.Action->GetLastTrigger(),
                     ERuleRangerActionTrigger::AT_Fix);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorSubsystemValidateCachesAndClearsRuleMatchesTest,
                                 "RuleRanger.UI.EditorSubsystem.ValidateCachesAndClearsRuleMatches",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorSubsystemValidateCachesAndClearsRuleMatchesTest::RunTest(const FString&)
{
    const auto Subsystem = GEditor ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>() : nullptr;
    RuleRangerEditorSubsystemTests::FAssetRuleFixture Fixture;
    if (!TestNotNull(TEXT("RuleRanger editor subsystem should be available"), Subsystem)
        || !RuleRangerEditorSubsystemTests::CreateAssetRuleFixture(*this, Fixture))
    {
        return false;
    }

    RuleRangerTests::FScopedRuleRangerDeveloperSettingsOverride SettingsOverride({ Fixture.Config });
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerAutomationCapturingResultHandler>();
    if (!TestNotNull(TEXT("Capturing result handler should be created"), Handler))
    {
        return false;
    }

    const bool bCanValidate = Subsystem->CanValidateObject(Fixture.Object, false);
    const int32 MatcherCallsAfterCanValidate = Fixture.Matcher->GetCallCount();
    Subsystem->ValidateObject(Fixture.Object, false, Handler);

    return TestTrue(TEXT("CanValidateObject should accept matching validate rules"), bCanValidate)
        && TestEqual(TEXT("CanValidateObject should evaluate the matcher once"), MatcherCallsAfterCanValidate, 1)
        && TestEqual(TEXT("ValidateObject should run a positive match again before applying the rule"),
                     Fixture.Matcher->GetCallCount(),
                     2)
        && TestEqual(TEXT("ValidateObject should apply the validation rule"), Fixture.Action->GetApplyCount(), 1)
        && TestEqual(TEXT("ValidateObject should use the validate trigger"),
                     Fixture.Action->GetLastTrigger(),
                     ERuleRangerActionTrigger::AT_Validate)
        && TestTrue(TEXT("ValidateObject should clear validation match cache"),
                    FRuleRangerEditorSubsystemTestAccessor::IsValidationMatchCacheEmpty(Subsystem));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorSubsystemProjectScanHonorsReportFixAndCancellationTest,
                                 "RuleRanger.UI.EditorSubsystem.ProjectScanHonorsReportFixAndCancellation",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorSubsystemProjectScanHonorsReportFixAndCancellationTest::RunTest(const FString&)
{
    const auto Subsystem = GEditor ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>() : nullptr;
    URuleRangerAutomationTestProjectAction* Action = nullptr;
    const auto Config = RuleRangerEditorSubsystemTests::CreateProjectRuleConfig(*this, Action);
    if (!TestNotNull(TEXT("RuleRanger editor subsystem should be available"), Subsystem)
        || !TestNotNull(TEXT("Project config should be available"), Config)
        || !TestNotNull(TEXT("Project action should be available"), Action))
    {
        return false;
    }

    RuleRangerTests::FScopedRuleRangerDeveloperSettingsOverride SettingsOverride({ Config });
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerAutomationCapturingProjectResultHandler>();
    if (!TestNotNull(TEXT("Capturing project result handler should be created"), Handler))
    {
        return false;
    }

    Subsystem->RunProjectScan(false, Handler);
    const bool bReportRun = TestEqual(TEXT("Project report scan should apply the rule"), Action->GetApplyCount(), 1)
        && TestEqual(TEXT("Project report scan should notify the handler"), Handler->CallCount, 1)
        && TestEqual(TEXT("Project report scan should use the report trigger"),
                     Action->GetLastTrigger(),
                     ERuleRangerProjectActionTrigger::AT_Report);

    Subsystem->RunProjectScan(true, Handler);
    bool bShouldContinue = false;
    Subsystem->RunProjectScanCancellable(true, Handler, [&bShouldContinue] { return bShouldContinue; });

    return bReportRun && TestEqual(TEXT("Project fix scan should apply the rule"), Action->GetApplyCount(), 2)
        && TestEqual(TEXT("Project fix scan should notify the handler"), Handler->CallCount, 2)
        && TestEqual(TEXT("Project fix scan should use the fix trigger"),
                     Action->GetLastTrigger(),
                     ERuleRangerProjectActionTrigger::AT_Fix)
        && TestEqual(TEXT("Cancelled project scan should not apply another rule"), Action->GetApplyCount(), 2);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorSubsystemConfiguredPathsAndProjectRulesReflectSettingsTest,
                                 "RuleRanger.UI.EditorSubsystem.ConfiguredPathsAndProjectRulesReflectSettings",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorSubsystemConfiguredPathsAndProjectRulesReflectSettingsTest::RunTest(const FString&)
{
    const auto Subsystem = GEditor ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>() : nullptr;
    URuleRangerAutomationTestProjectAction* Action = nullptr;
    const auto Config = RuleRangerEditorSubsystemTests::CreateProjectRuleConfig(*this, Action);
    if (!TestNotNull(TEXT("RuleRanger editor subsystem should be available"), Subsystem)
        || !TestNotNull(TEXT("Project config should be available"), Config))
    {
        return false;
    }

    Config->Dirs.Add({ TEXT("/Game/RuleRangerConfigured/") });
    RuleRangerTests::FScopedRuleRangerDeveloperSettingsOverride SettingsOverride({ Config });

    TArray<FString> Paths;
    Subsystem->CollectConfiguredPaths(Paths);

    return TestTrue(TEXT("A configured directory should be detected"), Subsystem->HasAnyConfiguredDirs())
        && TestTrue(TEXT("A project rule should be detected"), Subsystem->HasAnyProjectRules())
        && TestEqual(TEXT("One configured path should be collected"), Paths.Num(), 1)
        && TestEqual(TEXT("The configured path should be returned"),
                     Paths[0],
                     FString(TEXT("/Game/RuleRangerConfigured/")))
        && TestFalse(TEXT("Collecting paths should populate the settings cache"),
                     FRuleRangerEditorSubsystemTestAccessor::IsRuleSetConfigCacheDirty(Subsystem));
}

#endif
