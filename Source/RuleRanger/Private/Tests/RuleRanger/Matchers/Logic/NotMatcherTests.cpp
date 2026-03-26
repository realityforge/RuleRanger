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
    #include "RuleRanger/Matchers/Logic/NotMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNotMatcherNullInputsReturnFalseTest,
                                 "RuleRanger.Matchers.Logic.Not.NullInputsReturnFalse",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerNotMatcherNullInputsReturnFalseTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UNotMatcher>();
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("NotMatcherObject"));
    return TestNotNull(TEXT("Not matcher should be created"), Matcher)
        && TestNotNull(TEXT("Object should be created"), Object)
        && TestFalse(TEXT("Not matcher should return false for a null object"), Matcher->Test(nullptr))
        && TestFalse(TEXT("Not matcher should return false for a null child matcher"), Matcher->Test(Object));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNotMatcherInvertsChildResultTest,
                                 "RuleRanger.Matchers.Logic.Not.InvertsChildResult",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerNotMatcherInvertsChildResultTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UNotMatcher>();
    const auto ChildMatcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestMatcher>(Matcher);
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("NotMatcherObject"));
    if (TestNotNull(TEXT("Not matcher should be created"), Matcher)
        && TestNotNull(TEXT("Child matcher should be created"), ChildMatcher)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("Matcher"), ChildMatcher))
        {
            if (RuleRangerTests::SetPropertyValue(*this, ChildMatcher, TEXT("bResult"), false))
            {
                const auto bTrueWhenChildFalse =
                    TestTrue(TEXT("Not should invert a false child"), Matcher->Test(Object));
                const auto bFalseCallCount =
                    TestEqual(TEXT("False child should be evaluated once"), ChildMatcher->GetCallCount(), 1);
                ChildMatcher->ResetCallCount();
                if (RuleRangerTests::SetPropertyValue(*this, ChildMatcher, TEXT("bResult"), true))
                {
                    const auto bFalseWhenChildTrue =
                        TestFalse(TEXT("Not should invert a true child"), Matcher->Test(Object));
                    const auto bTrueCallCount =
                        TestEqual(TEXT("True child should be evaluated once"), ChildMatcher->GetCallCount(), 1);

                    return bTrueWhenChildFalse && bFalseCallCount && bFalseWhenChildTrue && bTrueCallCount;
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
    else
    {
        return false;
    }
}

#endif
