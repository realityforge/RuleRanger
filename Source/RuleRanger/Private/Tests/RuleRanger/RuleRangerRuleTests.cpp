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

    #include "Engine/Texture2D.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRangerRule.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerRuleTests
{
    bool SetMatchers(FAutomationTestBase& Test,
                     URuleRangerRule* Rule,
                     const TArray<TObjectPtr<URuleRangerMatcher>>& Matchers)
    {
        return RuleRangerTests::SetPropertyValue(Test, Rule, TEXT("Matchers"), Matchers);
    }

    bool
    SetActions(FAutomationTestBase& Test, URuleRangerRule* Rule, const TArray<TObjectPtr<URuleRangerAction>>& Actions)
    {
        return RuleRangerTests::SetPropertyValue(Test, Rule, TEXT("Actions"), Actions);
    }
} // namespace RuleRangerRuleTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleMatchSkipsMatchersWhenNoActionAcceptsObjectTypeTest,
                                 "RuleRanger.Rule.Match.SkipsMatchersWhenNoActionAcceptsObjectType",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleMatchSkipsMatchersWhenNoActionAcceptsObjectTypeTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        const auto Matcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Fixture.Rule);
        const auto Action = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        if (TestNotNull(TEXT("Matcher should be created"), Matcher)
            && TestNotNull(TEXT("Action should be created"), Action))
        {
            if (RuleRangerTests::SetPropertyValue(*this,
                                                  Action,
                                                  TEXT("ExpectedType"),
                                                  TSubclassOf<UObject>(UTexture2D::StaticClass()))
                && RuleRangerRuleTests::SetMatchers(*this, Fixture.Rule, { Matcher })
                && RuleRangerRuleTests::SetActions(*this, Fixture.Rule, { Action }))
            {
                const auto bMatched = Fixture.Rule->Match(Fixture.ActionContext, Fixture.Object);

                return TestFalse(TEXT("Rule should not match when no actions accept the object type"), bMatched)
                    && TestEqual(TEXT("Matchers should not be evaluated in this case"), Matcher->GetCallCount(), 0);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleMatchInvalidMatcherAddsErrorTest,
                                 "RuleRanger.Rule.Match.InvalidMatcherAddsError",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleMatchInvalidMatcherAddsErrorTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        const auto Action = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        if (TestNotNull(TEXT("Action should be created"), Action))
        {
            const TArray<TObjectPtr<URuleRangerMatcher>> Matchers{ nullptr };
            if (RuleRangerRuleTests::SetMatchers(*this, Fixture.Rule, Matchers)
                && RuleRangerRuleTests::SetActions(*this, Fixture.Rule, { Action }))
            {
                const auto bMatched = Fixture.Rule->Match(Fixture.ActionContext, Fixture.Object);

                return TestFalse(TEXT("Invalid matcher should prevent a match"), bMatched)
                    && RuleRangerTests::TestTextArrayContains(*this,
                                                              Fixture.ActionContext->GetErrorMessages(),
                                                              TEXT("Invalid matcher should add an error"),
                                                              TEXT("Invalid Matcher detected at index 0"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleMatchStopsOnFalseMatcherTest,
                                 "RuleRanger.Rule.Match.StopsOnFalseMatcher",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleMatchStopsOnFalseMatcherTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        const auto FirstMatcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Fixture.Rule);
        const auto SecondMatcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Fixture.Rule);
        const auto Action = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        if (TestNotNull(TEXT("First matcher should be created"), FirstMatcher)
            && TestNotNull(TEXT("Second matcher should be created"), SecondMatcher)
            && TestNotNull(TEXT("Action should be created"), Action))
        {
            if (RuleRangerTests::SetPropertyValue(*this, FirstMatcher, TEXT("bResult"), false)
                && RuleRangerTests::SetPropertyValue(*this, SecondMatcher, TEXT("bResult"), true)
                && RuleRangerRuleTests::SetMatchers(*this, Fixture.Rule, { FirstMatcher, SecondMatcher })
                && RuleRangerRuleTests::SetActions(*this, Fixture.Rule, { Action }))
            {
                const auto bMatched = Fixture.Rule->Match(Fixture.ActionContext, Fixture.Object);

                return TestFalse(TEXT("A false matcher should stop the match"), bMatched)
                    && TestEqual(TEXT("The first matcher should be evaluated once"), FirstMatcher->GetCallCount(), 1)
                    && TestEqual(TEXT("The second matcher should not run after a false matcher"),
                                 SecondMatcher->GetCallCount(),
                                 0);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleApplyInvalidActionAddsErrorTest,
                                 "RuleRanger.Rule.Apply.InvalidActionAddsError",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleApplyInvalidActionAddsErrorTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        const auto ValidAction = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        if (TestNotNull(TEXT("Valid action should be created"), ValidAction))
        {
            const TArray<TObjectPtr<URuleRangerAction>> Actions{ nullptr, ValidAction };
            if (RuleRangerRuleTests::SetActions(*this, Fixture.Rule, Actions))
            {
                Fixture.Rule->Apply(Fixture.ActionContext, Fixture.Object);

                return RuleRangerTests::TestTextArrayContains(*this,
                                                              Fixture.ActionContext->GetErrorMessages(),
                                                              TEXT("Invalid action should add an error"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleApplySkipsWrongExpectedTypeTest,
                                 "RuleRanger.Rule.Apply.SkipsWrongExpectedType",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleApplySkipsWrongExpectedTypeTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        const auto WrongTypeAction = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        const auto ValidAction = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        if (TestNotNull(TEXT("Wrong-type action should be created"), WrongTypeAction)
            && TestNotNull(TEXT("Valid action should be created"), ValidAction))
        {
            if (RuleRangerTests::SetPropertyValue(*this,
                                                  WrongTypeAction,
                                                  TEXT("ExpectedType"),
                                                  TSubclassOf<UObject>(UTexture2D::StaticClass()))
                && RuleRangerRuleTests::SetActions(*this, Fixture.Rule, { WrongTypeAction, ValidAction }))
            {
                AddExpectedMessagePlain(TEXT("Attempt to run on Object that is not an instance of the type Texture2D."),
                                        ELogVerbosity::Error,
                                        EAutomationExpectedMessageFlags::Contains,
                                        1);
                Fixture.Rule->Apply(Fixture.ActionContext, Fixture.Object);

                return TestEqual(TEXT("The wrong-type action should not run when the object type is wrong"),
                                 WrongTypeAction->GetApplyCount(),
                                 0)
                    && TestEqual(TEXT("A compatible action should still run"), ValidAction->GetApplyCount(), 1);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleApplyStopsAfterErrorByDefaultTest,
                                 "RuleRanger.Rule.Apply.StopsAfterErrorByDefault",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleApplyStopsAfterErrorByDefaultTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        const auto FirstAction = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        const auto SecondAction = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        if (TestNotNull(TEXT("First action should be created"), FirstAction)
            && TestNotNull(TEXT("Second action should be created"), SecondAction))
        {
            if (RuleRangerTests::SetPropertyValue(*this,
                                                  FirstAction,
                                                  TEXT("Outcome"),
                                                  ERuleRangerAutomationTestActionOutcome::Error)
                && RuleRangerRuleTests::SetActions(*this, Fixture.Rule, { FirstAction, SecondAction }))
            {
                Fixture.Rule->Apply(Fixture.ActionContext, Fixture.Object);

                return TestEqual(TEXT("The first action should run once"), FirstAction->GetApplyCount(), 1)
                    && TestEqual(TEXT("The second action should be skipped after an error"),
                                 SecondAction->GetApplyCount(),
                                 0)
                    && TestEqual(TEXT("The context should end in the error state"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleApplyContinuesAfterErrorWhenConfiguredTest,
                                 "RuleRanger.Rule.Apply.ContinuesAfterErrorWhenConfigured",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleApplyContinuesAfterErrorWhenConfiguredTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        const auto FirstAction = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        const auto SecondAction = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        if (TestNotNull(TEXT("First action should be created"), FirstAction)
            && TestNotNull(TEXT("Second action should be created"), SecondAction))
        {
            if (RuleRangerTests::SetPropertyValue(*this, Fixture.Rule.Get(), TEXT("bContinueOnError"), true)
                && RuleRangerTests::SetPropertyValue(*this,
                                                     FirstAction,
                                                     TEXT("Outcome"),
                                                     ERuleRangerAutomationTestActionOutcome::Error)
                && RuleRangerRuleTests::SetActions(*this, Fixture.Rule, { FirstAction, SecondAction }))
            {
                Fixture.Rule->Apply(Fixture.ActionContext, Fixture.Object);

                return TestEqual(TEXT("The first action should run once"), FirstAction->GetApplyCount(), 1)
                    && TestEqual(TEXT("The second action should still run"), SecondAction->GetApplyCount(), 1)
                    && TestEqual(TEXT("The context should still end in the error state"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleApplyStopsAfterFatalTest,
                                 "RuleRanger.Rule.Apply.StopsAfterFatal",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleApplyStopsAfterFatalTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        const auto FirstAction = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        const auto SecondAction = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Fixture.Rule);
        if (TestNotNull(TEXT("First action should be created"), FirstAction)
            && TestNotNull(TEXT("Second action should be created"), SecondAction))
        {
            if (RuleRangerTests::SetPropertyValue(*this,
                                                  FirstAction,
                                                  TEXT("Outcome"),
                                                  ERuleRangerAutomationTestActionOutcome::Fatal)
                && RuleRangerRuleTests::SetActions(*this, Fixture.Rule, { FirstAction, SecondAction }))
            {
                Fixture.Rule->Apply(Fixture.ActionContext, Fixture.Object);

                return TestEqual(TEXT("The first action should run once"), FirstAction->GetApplyCount(), 1)
                    && TestEqual(TEXT("The second action should be skipped after a fatal"),
                                 SecondAction->GetApplyCount(),
                                 0)
                    && TestEqual(TEXT("The context should end in the fatal state"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRulePreSaveCleansArraysAndAutofillsDescriptionTest,
                                 "RuleRanger.Rule.PreSave.CleansArraysAndAutofillsDescription",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRulePreSaveCleansArraysAndAutofillsDescriptionTest::RunTest(const FString&)
{
    const auto Rule = RuleRangerTests::NewTransientObject<URuleRangerRule>();
    const auto Matcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Rule);
    const auto Action = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>(Rule);
    if (TestNotNull(TEXT("Rule should be created"), Rule) && TestNotNull(TEXT("Matcher should be created"), Matcher)
        && TestNotNull(TEXT("Action should be created"), Action))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Rule, TEXT("Description"), FString(TEXT("   ")))
            && RuleRangerRuleTests::SetMatchers(*this, Rule, { nullptr, Matcher })
            && RuleRangerRuleTests::SetActions(*this, Rule, { nullptr, Action }))
        {
            const RuleRangerTests::FPreSaveContextHolder SaveContext;
            Rule->PreSave(SaveContext.Context);

            return TestEqual(TEXT("PreSave should remove null matchers"), Rule->Matchers.Num(), 1)
                && TestEqual(TEXT("PreSave should remove null actions"), Rule->Actions.Num(), 1)
                && TestEqual(TEXT("PreSave should autofill description from the single action"),
                             Rule->Description,
                             FString(TEXT("Automation Test Action")));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleValidationFailsWithoutValidActionsTest,
                                 "RuleRanger.Validation.Rule.NoValidActions",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleValidationFailsWithoutValidActionsTest::RunTest(const FString&)
{
    const auto Rule = RuleRangerTests::NewTransientObject<URuleRangerRule>();
    if (TestNotNull(TEXT("Rule should be created"), Rule))
    {
        const TArray<TObjectPtr<URuleRangerAction>> Actions{ nullptr };
        if (RuleRangerRuleTests::SetActions(*this, Rule, Actions))
        {
            return RuleRangerTests::TestValidation(*this,
                                                   Rule,
                                                   EDataValidationResult::Invalid,
                                                   TEXT("RuleRangerRule has no actions"));
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

#endif
