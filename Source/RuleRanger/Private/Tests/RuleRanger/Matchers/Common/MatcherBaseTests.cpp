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
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerMatcherBaseReturnsFalseForAnyObjectTest,
                                 "RuleRanger.Matchers.Common.MatcherBase.ReturnsFalseForAnyObject",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerMatcherBaseReturnsFalseForAnyObjectTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationMatcherFallback>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestObject>();
    if (TestNotNull(TEXT("Matcher should be created"), Matcher)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        return TestFalse(TEXT("The matcher base should return false for valid objects"), Matcher->Test(Object))
            && TestFalse(TEXT("The matcher base should return false for null objects"), Matcher->Test(nullptr));
    }
    else
    {
        return false;
    }
}

#endif
