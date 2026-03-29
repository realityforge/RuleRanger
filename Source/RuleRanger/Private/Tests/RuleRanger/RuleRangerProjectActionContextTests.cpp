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
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectActionContextResetCapturesReferencesTest,
                                 "RuleRanger.Context.Project.ResetCapturesReferences",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectActionContextResetCapturesReferencesTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture, ERuleRangerProjectActionTrigger::AT_Fix))
    {
        return TestEqual(TEXT("Reset should store the config"),
                         Fixture.ActionContext->GetConfig(),
                         static_cast<const URuleRangerConfig*>(Fixture.Config))
            && TestEqual(TEXT("Reset should store the rule set"),
                         Fixture.ActionContext->GetRuleSet(),
                         static_cast<const URuleRangerRuleSet*>(Fixture.RuleSet))
            && TestEqual(TEXT("Reset should store the rule"),
                         Fixture.ActionContext->GetRule(),
                         static_cast<const URuleRangerProjectRule*>(Fixture.Rule))
            && TestEqual(TEXT("Reset should store the trigger"),
                         Fixture.ActionContext->GetActionTrigger(),
                         ERuleRangerProjectActionTrigger::AT_Fix)
            && TestEqual(TEXT("Reset should set common state to success"),
                         Fixture.ActionContext->GetState(),
                         ERuleRangerActionState::AS_Success);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectActionContextClearResetsStateTest,
                                 "RuleRanger.Context.Project.ClearResetsState",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectActionContextClearResetsStateTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture, ERuleRangerProjectActionTrigger::AT_Fix))
    {
        Fixture.ActionContext->Warning(FText::FromString(TEXT("Warning before clear")));
        FRuleRangerProjectActionContextTestAccessor::ClearContext(Fixture.ActionContext);

        return TestNull(TEXT("Clear should reset the config"), Fixture.ActionContext->GetConfig())
            && TestNull(TEXT("Clear should reset the rule set"), Fixture.ActionContext->GetRuleSet())
            && TestNull(TEXT("Clear should reset the rule"), Fixture.ActionContext->GetRule())
            && TestEqual(TEXT("Clear should default the trigger to report"),
                         Fixture.ActionContext->GetActionTrigger(),
                         ERuleRangerProjectActionTrigger::AT_Report)
            && TestEqual(TEXT("Clear should reset state to success"),
                         Fixture.ActionContext->GetState(),
                         ERuleRangerActionState::AS_Success)
            && TestTrue(TEXT("Clear should remove warnings"), Fixture.ActionContext->GetWarningMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectActionContextDryRunSemanticsTest,
                                 "RuleRanger.Context.Project.DryRunSemantics",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectActionContextDryRunSemanticsTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture))
    {
        struct FCase
        {
            ERuleRangerProjectActionTrigger Trigger;
            bool bExpectedDryRun;
            const TCHAR* Label;
        };

        const TArray<FCase> Cases{
            { ERuleRangerProjectActionTrigger::AT_Report, true, TEXT("Report should be dry run") },
            { ERuleRangerProjectActionTrigger::AT_Fix, false, TEXT("Fix should not be dry run") },
        };

        auto bAllPassed = true;
        for (const auto& Case : Cases)
        {
            RuleRangerTests::ResetProjectRuleFixtureContext(Fixture, Case.Trigger);
            bAllPassed = TestEqual(Case.Label, Fixture.ActionContext->IsDryRun(), Case.bExpectedDryRun) && bAllPassed;
        }

        return bAllPassed;
    }
    else
    {
        return false;
    }
}

#endif
