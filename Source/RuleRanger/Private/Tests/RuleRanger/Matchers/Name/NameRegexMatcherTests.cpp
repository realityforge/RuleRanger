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
    #include "RuleRanger/Matchers/Name/NameRegexMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNameRegexMatcherMatchesExpectedPatternTest,
                                 "RuleRanger.Matchers.Name.Regex.MatchesExpectedPattern",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerNameRegexMatcherMatchesExpectedPatternTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UNameRegexMatcher>();
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("BP_TestWidget"));
    if (TestNotNull(TEXT("Regex matcher should be created"), Matcher)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this,
                                              Matcher,
                                              TEXT("RegexPattern"),
                                              FString(TEXT("^BP_[A-Z][A-Za-z]+$"))))
        {
            return TestTrue(TEXT("Regex should match the expected name"), Matcher->Test(Object));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNameRegexMatcherHonorsCaseSensitivityTest,
                                 "RuleRanger.Matchers.Name.Regex.HonorsCaseSensitivity",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerNameRegexMatcherHonorsCaseSensitivityTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UNameRegexMatcher>();
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("BP_TestWidget"));
    if (TestNotNull(TEXT("Regex matcher should be created"), Matcher)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("RegexPattern"), FString(TEXT("^bp_[a-z][a-z]+$"))))
        {
            const auto bCaseSensitiveMiss =
                TestFalse(TEXT("Regex should be case-sensitive by default"), Matcher->Test(Object));
            if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("bCaseSensitive"), false))
            {
                const auto bCaseInsensitiveMatch =
                    TestTrue(TEXT("Regex should match when case-insensitive"), Matcher->Test(Object));
                return bCaseSensitiveMiss && bCaseInsensitiveMatch;
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
