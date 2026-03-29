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
    #include "RuleRangerProjectRule.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerProjectRuleTests
{
    bool SetActions(FAutomationTestBase& Test,
                    URuleRangerProjectRule* Rule,
                    const TArray<TObjectPtr<URuleRangerProjectAction>>& Actions)
    {
        return RuleRangerTests::SetPropertyValue(Test, Rule, TEXT("Actions"), Actions);
    }

    bool SetContinueOnError(FAutomationTestBase& Test, URuleRangerProjectRule* Rule, const bool bContinueOnError)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Rule, TEXT("bContinueOnError"), bContinueOnError);
    }
} // namespace RuleRangerProjectRuleTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectRuleApplyInvalidActionAddsErrorTest,
                                 "RuleRanger.ProjectRule.Apply.InvalidActionAddsError",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectRuleApplyInvalidActionAddsErrorTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture))
    {
        const auto ValidAction =
            RuleRangerTests::NewTransientObject<URuleRangerAutomationTestProjectAction>(Fixture.Rule);
        if (TestNotNull(TEXT("Valid project action should be created"), ValidAction))
        {
            const TArray<TObjectPtr<URuleRangerProjectAction>> Actions{ nullptr, ValidAction };
            if (RuleRangerProjectRuleTests::SetActions(*this, Fixture.Rule, Actions))
            {
                Fixture.Rule->Apply(Fixture.ActionContext);

                return RuleRangerTests::TestTextArrayContains(*this,
                                                              Fixture.ActionContext->GetErrorMessages(),
                                                              TEXT("Invalid project action should add an error"),
                                                              TEXT("Invalid Action detected at index 0"))
                    && TestEqual(TEXT("A later valid action should still run"), ValidAction->GetApplyCount(), 1);
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectRuleApplyStopsAfterErrorByDefaultTest,
                                 "RuleRanger.ProjectRule.Apply.StopsAfterErrorByDefault",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectRuleApplyStopsAfterErrorByDefaultTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture))
    {
        const auto FirstAction =
            RuleRangerTests::NewTransientObject<URuleRangerAutomationTestProjectAction>(Fixture.Rule);
        const auto SecondAction =
            RuleRangerTests::NewTransientObject<URuleRangerAutomationTestProjectAction>(Fixture.Rule);
        if (TestNotNull(TEXT("First project action should be created"), FirstAction)
            && TestNotNull(TEXT("Second project action should be created"), SecondAction))
        {
            if (RuleRangerTests::SetPropertyValue(*this,
                                                  FirstAction,
                                                  TEXT("Outcome"),
                                                  ERuleRangerAutomationTestActionOutcome::Error)
                && RuleRangerProjectRuleTests::SetActions(*this, Fixture.Rule, { FirstAction, SecondAction }))
            {
                Fixture.Rule->Apply(Fixture.ActionContext);

                return TestEqual(TEXT("The first action should run once"), FirstAction->GetApplyCount(), 1)
                    && TestEqual(TEXT("The second action should be skipped after the error"),
                                 SecondAction->GetApplyCount(),
                                 0)
                    && TestEqual(TEXT("The project action context should stay in the error state"),
                                 Fixture.ActionContext->GetState(),
                                 ERuleRangerActionState::AS_Error);
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectRuleApplyContinuesAfterErrorWhenConfiguredTest,
                                 "RuleRanger.ProjectRule.Apply.ContinuesAfterErrorWhenConfigured",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectRuleApplyContinuesAfterErrorWhenConfiguredTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture))
    {
        const auto FirstAction =
            RuleRangerTests::NewTransientObject<URuleRangerAutomationTestProjectAction>(Fixture.Rule);
        const auto SecondAction =
            RuleRangerTests::NewTransientObject<URuleRangerAutomationTestProjectAction>(Fixture.Rule);
        if (TestNotNull(TEXT("First project action should be created"), FirstAction)
            && TestNotNull(TEXT("Second project action should be created"), SecondAction))
        {
            if (RuleRangerTests::SetPropertyValue(*this,
                                                  FirstAction,
                                                  TEXT("Outcome"),
                                                  ERuleRangerAutomationTestActionOutcome::Error)
                && RuleRangerProjectRuleTests::SetContinueOnError(*this, Fixture.Rule, true)
                && RuleRangerProjectRuleTests::SetActions(*this, Fixture.Rule, { FirstAction, SecondAction }))
            {
                Fixture.Rule->Apply(Fixture.ActionContext);

                return TestEqual(TEXT("The first action should run once"), FirstAction->GetApplyCount(), 1)
                    && TestEqual(TEXT("The second action should still run when continue-on-error is enabled"),
                                 SecondAction->GetApplyCount(),
                                 1);
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectRuleApplyStopsAfterFatalTest,
                                 "RuleRanger.ProjectRule.Apply.StopsAfterFatal",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectRuleApplyStopsAfterFatalTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture))
    {
        const auto FirstAction =
            RuleRangerTests::NewTransientObject<URuleRangerAutomationTestProjectAction>(Fixture.Rule);
        const auto SecondAction =
            RuleRangerTests::NewTransientObject<URuleRangerAutomationTestProjectAction>(Fixture.Rule);
        if (TestNotNull(TEXT("First project action should be created"), FirstAction)
            && TestNotNull(TEXT("Second project action should be created"), SecondAction))
        {
            if (RuleRangerTests::SetPropertyValue(*this,
                                                  FirstAction,
                                                  TEXT("Outcome"),
                                                  ERuleRangerAutomationTestActionOutcome::Fatal)
                && RuleRangerProjectRuleTests::SetContinueOnError(*this, Fixture.Rule, true)
                && RuleRangerProjectRuleTests::SetActions(*this, Fixture.Rule, { FirstAction, SecondAction }))
            {
                Fixture.Rule->Apply(Fixture.ActionContext);

                return TestEqual(TEXT("The first action should run once"), FirstAction->GetApplyCount(), 1)
                    && TestEqual(TEXT("The second action should be skipped after a fatal state"),
                                 SecondAction->GetApplyCount(),
                                 0)
                    && TestEqual(TEXT("The project action context should stay in the fatal state"),
                                 Fixture.ActionContext->GetState(),
                                 ERuleRangerActionState::AS_Fatal);
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectRulePreSaveCleansArraysAndAutofillsDescriptionTest,
                                 "RuleRanger.ProjectRule.PreSave.CleansArraysAndAutofillsDescription",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectRulePreSaveCleansArraysAndAutofillsDescriptionTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture))
    {
        const auto Action = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestProjectAction>(Fixture.Rule);
        if (TestNotNull(TEXT("Project action should be created"), Action))
        {
            if (RuleRangerProjectRuleTests::SetActions(*this, Fixture.Rule, { nullptr, Action }))
            {
                RuleRangerTests::FPreSaveContextHolder SaveContextHolder;
                Fixture.Rule->PreSave(SaveContextHolder.Context);

                return TestEqual(TEXT("PreSave should remove invalid project actions"), Fixture.Rule->Actions.Num(), 1)
                    && TestEqual(TEXT("PreSave should derive the description from the single action display name"),
                                 Fixture.Rule->Description,
                                 FString(TEXT("Automation Test Project Action")));
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectRuleValidationNoValidActionsTest,
                                 "RuleRanger.Validation.ProjectRule.NoValidActions",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectRuleValidationNoValidActionsTest::RunTest(const FString&)
{
    const auto Rule = RuleRangerTests::NewTransientObject<URuleRangerProjectRule>();
    if (TestNotNull(TEXT("Project rule should be created"), Rule))
    {
        return RuleRangerTests::TestValidation(*this, Rule, EDataValidationResult::Invalid, TEXT("has no actions"));
    }
    else
    {
        return false;
    }
}

#endif
