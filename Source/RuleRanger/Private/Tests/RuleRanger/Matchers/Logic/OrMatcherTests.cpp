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
    #include "RuleRanger/Matchers/Logic/OrMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerOrMatcherTests
{
    bool
    SetMatchers(FAutomationTestBase& Test, UOrMatcher* Matcher, const TArray<TObjectPtr<URuleRangerMatcher>>& Matchers)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Matchers"), Matchers);
    }
} // namespace RuleRangerOrMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerOrMatcherNullObjectReturnsFalseTest,
                                 "RuleRanger.Matchers.Logic.Or.NullObjectReturnsFalse",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerOrMatcherNullObjectReturnsFalseTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UOrMatcher>();
    return TestNotNull(TEXT("Or matcher should be created"), Matcher)
        && TestFalse(TEXT("Or matcher should return false for a null object"), Matcher->Test(nullptr));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerOrMatcherEmptyMatchersReturnsFalseTest,
                                 "RuleRanger.Matchers.Logic.Or.EmptyMatchersReturnsFalse",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerOrMatcherEmptyMatchersReturnsFalseTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UOrMatcher>();
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("OrMatcherObject"));
    return TestNotNull(TEXT("Or matcher should be created"), Matcher)
        && TestNotNull(TEXT("Object should be created"), Object)
        && TestFalse(TEXT("Or matcher should return false with no child matchers"), Matcher->Test(Object));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerOrMatcherSkipsNullChildrenAndShortCircuitsOnTrueTest,
                                 "RuleRanger.Matchers.Logic.Or.SkipsNullChildrenAndShortCircuitsOnTrue",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerOrMatcherSkipsNullChildrenAndShortCircuitsOnTrueTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UOrMatcher>();
    const auto FirstMatcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Matcher);
    const auto SecondMatcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Matcher);
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("OrMatcherObject"));
    if (TestNotNull(TEXT("Or matcher should be created"), Matcher)
        && TestNotNull(TEXT("First child matcher should be created"), FirstMatcher)
        && TestNotNull(TEXT("Second child matcher should be created"), SecondMatcher)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, FirstMatcher, TEXT("bResult"), true)
            && RuleRangerOrMatcherTests::SetMatchers(*this, Matcher, { nullptr, FirstMatcher, SecondMatcher }))
        {
            return TestTrue(TEXT("A true child should make the Or matcher return true"), Matcher->Test(Object))
                && TestEqual(TEXT("The first valid child should run once"), FirstMatcher->GetCallCount(), 1)
                && TestEqual(TEXT("Later children should not run after a true child"),
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
