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
    #include "RuleRanger/Actions/Common/EnsureMatchersMatchAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerEnsureMatchersMatchActionTests
{
    bool SetMatchers(FAutomationTestBase& Test, UObject* Object, const TArray<TObjectPtr<URuleRangerMatcher>>& Matchers)
    {
        return RuleRangerTests::SetPropertyValue(Test, Object, TEXT("Matchers"), Matchers);
    }
} // namespace RuleRangerEnsureMatchersMatchActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMatchersMatchActionAddsDefaultFailureMessageTest,
                                 "RuleRanger.Actions.Common.EnsureMatchersMatch.AddsDefaultFailureMessage",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMatchersMatchActionAddsDefaultFailureMessageTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMatchersMatchAction>();
    const auto Matcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Action);
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureMatchersMatchAction should be created"), Action)
        && TestNotNull(TEXT("Matcher should be created"), Matcher))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("bResult"), false)
            && RuleRangerEnsureMatchersMatchActionTests::SetMatchers(*this, Action, { Matcher }))
        {
            Action->Apply(Fixture.ActionContext, Fixture.Object);

            return TestEqual(TEXT("One failing matcher should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && TestEqual(TEXT("Failing matcher should be evaluated once"), Matcher->GetCallCount(), 1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Default failure message should mention a failed matcher"),
                       TEXT("Failed matcher"))
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Default failure message should include the one-based index"),
                       TEXT("index 1"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMatchersMatchActionUsesConfiguredMessageForEachFailureTest,
                                 "RuleRanger.Actions.Common.EnsureMatchersMatch.UsesConfiguredMessageForEachFailure",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMatchersMatchActionUsesConfiguredMessageForEachFailureTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMatchersMatchAction>();
    const auto FirstMatcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Action);
    const auto SecondMatcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Action);
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureMatchersMatchAction should be created"), Action)
        && TestNotNull(TEXT("First matcher should be created"), FirstMatcher)
        && TestNotNull(TEXT("Second matcher should be created"), SecondMatcher))
    {
        if (RuleRangerTests::SetPropertyValue(*this, FirstMatcher, TEXT("bResult"), false)
            && RuleRangerTests::SetPropertyValue(*this, SecondMatcher, TEXT("bResult"), false)
            && RuleRangerTests::SetPropertyValue(*this,
                                                 Action,
                                                 TEXT("Message"),
                                                 FString(TEXT("Custom matcher failure")))
            && RuleRangerEnsureMatchersMatchActionTests::SetMatchers(*this, Action, { FirstMatcher, SecondMatcher }))
        {
            Action->Apply(Fixture.ActionContext, Fixture.Object);

            return TestEqual(TEXT("Each failing matcher should add an error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             2)
                && TestEqual(TEXT("The first matcher should be evaluated once"), FirstMatcher->GetCallCount(), 1)
                && TestEqual(TEXT("The second matcher should be evaluated once"), SecondMatcher->GetCallCount(), 1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The configured failure message should be preserved"),
                                                          TEXT("Custom matcher failure"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMatchersMatchActionLogsInvalidMatchersAndContinuesTest,
                                 "RuleRanger.Actions.Common.EnsureMatchersMatch.LogsInvalidMatchersAndContinues",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMatchersMatchActionLogsInvalidMatchersAndContinuesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMatchersMatchAction>();
    const auto Matcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Action);
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureMatchersMatchAction should be created"), Action)
        && TestNotNull(TEXT("Matcher should be created"), Matcher))
    {
        if (RuleRangerEnsureMatchersMatchActionTests::SetMatchers(*this, Action, { nullptr, Matcher }))
        {
            Action->Apply(Fixture.ActionContext, Fixture.Object);

            return TestTrue(TEXT("Invalid matchers should not add errors when valid matchers succeed"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty())
                && TestEqual(TEXT("Valid matchers should still be evaluated"), Matcher->GetCallCount(), 1);
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
