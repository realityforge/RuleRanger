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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerActionContextResetCapturesReferencesTest,
                                 "RuleRanger.Context.Action.ResetCapturesReferences",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerActionContextResetCapturesReferencesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this,
                                           Fixture,
                                           TEXT("ActionContextResetObject"),
                                           ERuleRangerActionTrigger::AT_Fix))
    {
        return TestEqual(TEXT("Reset should store the rule"),
                         Fixture.ActionContext->GetRule(),
                         static_cast<const URuleRangerRule*>(Fixture.Rule))
            && TestEqual(TEXT("Reset should store the object"),
                         Fixture.ActionContext->GetObject(),
                         static_cast<const UObject*>(Fixture.Object))
            && TestEqual(TEXT("Reset should store the trigger"),
                         Fixture.ActionContext->GetActionTrigger(),
                         ERuleRangerActionTrigger::AT_Fix)
            && TestEqual(TEXT("Reset should set common state to success"),
                         Fixture.ActionContext->GetState(),
                         ERuleRangerActionState::AS_Success);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerActionContextClearResetsStateTest,
                                 "RuleRanger.Context.Action.ClearResetsState",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerActionContextClearResetsStateTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this,
                                           Fixture,
                                           TEXT("ActionContextClearObject"),
                                           ERuleRangerActionTrigger::AT_Fix))
    {
        Fixture.ActionContext->Warning(FText::FromString(TEXT("Warning before clear")));
        FRuleRangerActionContextTestAccessor::ClearContext(Fixture.ActionContext);

        return TestNull(TEXT("Clear should reset the rule"), Fixture.ActionContext->GetRule())
            && TestNull(TEXT("Clear should reset the object"), Fixture.ActionContext->GetObject())
            && TestEqual(TEXT("Clear should default the trigger to report"),
                         Fixture.ActionContext->GetActionTrigger(),
                         ERuleRangerActionTrigger::AT_Report)
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerActionContextDryRunSemanticsTest,
                                 "RuleRanger.Context.Action.DryRunSemantics",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerActionContextDryRunSemanticsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        struct FCase
        {
            ERuleRangerActionTrigger Trigger;
            bool bExpectedDryRun;
            const TCHAR* Label;
        };

        const TArray<FCase> Cases{
            { ERuleRangerActionTrigger::AT_Import, false, TEXT("Import should be fix mode") },
            { ERuleRangerActionTrigger::AT_Reimport, false, TEXT("Reimport should be fix mode") },
            { ERuleRangerActionTrigger::AT_Validate, true, TEXT("Validate should be dry run") },
            { ERuleRangerActionTrigger::AT_Save, false, TEXT("Save should be fix mode") },
            { ERuleRangerActionTrigger::AT_Report, true, TEXT("Report should be dry run") },
            { ERuleRangerActionTrigger::AT_Fix, false, TEXT("Fix should not be dry run") },
        };

        auto bAllPassed = true;
        for (const auto& Case : Cases)
        {
            FRuleRangerActionContextTestAccessor::ResetContext(Fixture.ActionContext,
                                                               Fixture.Config,
                                                               Fixture.RuleSet,
                                                               Fixture.Rule,
                                                               Fixture.Object,
                                                               Case.Trigger);
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
