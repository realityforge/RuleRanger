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
    #include "RuleRanger/Matchers/Logic/AndMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerAndMatcherTests
{
    bool
    SetMatchers(FAutomationTestBase& Test, UAndMatcher* Matcher, const TArray<TObjectPtr<URuleRangerMatcher>>& Matchers)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Matchers"), Matchers);
    }
} // namespace RuleRangerAndMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerAndMatcherNullObjectReturnsTrueTest,
                                 "RuleRanger.Matchers.Logic.And.NullObjectReturnsTrue",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerAndMatcherNullObjectReturnsTrueTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UAndMatcher>();
    return TestNotNull(TEXT("And matcher should be created"), Matcher)
        && TestTrue(TEXT("And matcher should return true for a null object"), Matcher->Test(nullptr));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerAndMatcherEmptyMatchersReturnsTrueTest,
                                 "RuleRanger.Matchers.Logic.And.EmptyMatchersReturnsTrue",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerAndMatcherEmptyMatchersReturnsTrueTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UAndMatcher>();
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("AndMatcherObject"));
    return TestNotNull(TEXT("And matcher should be created"), Matcher)
        && TestNotNull(TEXT("Object should be created"), Object)
        && TestTrue(TEXT("And matcher should return true with no child matchers"), Matcher->Test(Object));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerAndMatcherSkipsNullChildrenTest,
                                 "RuleRanger.Matchers.Logic.And.SkipsNullChildren",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerAndMatcherSkipsNullChildrenTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UAndMatcher>();
    const auto ChildMatcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Matcher);
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("AndMatcherObject"));
    if (TestNotNull(TEXT("And matcher should be created"), Matcher)
        && TestNotNull(TEXT("Child matcher should be created"), ChildMatcher)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        return RuleRangerAndMatcherTests::SetMatchers(*this, Matcher, { nullptr, ChildMatcher })
            && TestTrue(TEXT("Valid child matchers should still be honored"), Matcher->Test(Object))
            && TestEqual(TEXT("Valid child should be evaluated exactly once"), ChildMatcher->GetCallCount(), 1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerAndMatcherShortCircuitsOnFirstFalseTest,
                                 "RuleRanger.Matchers.Logic.And.ShortCircuitsOnFirstFalse",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerAndMatcherShortCircuitsOnFirstFalseTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UAndMatcher>();
    const auto FirstMatcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Matcher);
    const auto SecondMatcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Matcher);
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("AndMatcherObject"));
    if (TestNotNull(TEXT("And matcher should be created"), Matcher)
        && TestNotNull(TEXT("First child matcher should be created"), FirstMatcher)
        && TestNotNull(TEXT("Second child matcher should be created"), SecondMatcher)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, FirstMatcher, TEXT("bResult"), false)
            && RuleRangerAndMatcherTests::SetMatchers(*this, Matcher, { FirstMatcher, SecondMatcher }))
        {
            return TestFalse(TEXT("A false child should make the And matcher return false"), Matcher->Test(Object))
                && TestEqual(TEXT("The first child should run once"), FirstMatcher->GetCallCount(), 1)
                && TestEqual(TEXT("The second child should not run after a false child"),
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

#endif
