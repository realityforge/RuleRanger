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
    #include "RuleRanger/Matchers/Name/NamePrefixMatcher.h"
    #include "RuleRanger/Matchers/Name/NameRegexMatcher.h"
    #include "RuleRanger/Matchers/Name/NameSuffixMatcher.h"
    #include "RuleRanger/Matchers/Name/NameWildcardMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNamePrefixMatcherHonorsCaseSensitivityTest,
                                 "RuleRanger.Matchers.Name.Prefix.HonorsCaseSensitivity",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerNamePrefixMatcherHonorsCaseSensitivityTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UNamePrefixMatcher>();
    const auto Object = RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("SM_TestAsset"));
    if (TestNotNull(TEXT("Prefix matcher should be created"), Matcher)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("Prefix"), FString(TEXT("SM_"))))
        {
            const auto bCaseSensitiveMatch =
                TestTrue(TEXT("Matching prefix should pass when case-sensitive"), Matcher->Test(Object));
            if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("Prefix"), FString(TEXT("sm_"))))
            {
                const auto bCaseSensitiveMiss =
                    TestFalse(TEXT("Different case should fail when case-sensitive"), Matcher->Test(Object));
                if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("bCaseSensitive"), false))
                {
                    const auto bCaseInsensitiveMatch =
                        TestTrue(TEXT("Different case should pass when case-insensitive"), Matcher->Test(Object));
                    return bCaseSensitiveMatch && bCaseSensitiveMiss && bCaseInsensitiveMatch;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNameSuffixMatcherHonorsCaseSensitivityTest,
                                 "RuleRanger.Matchers.Name.Suffix.HonorsCaseSensitivity",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerNameSuffixMatcherHonorsCaseSensitivityTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UNameSuffixMatcher>();
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("SM_TestAsset_LOD0"));
    if (TestNotNull(TEXT("Suffix matcher should be created"), Matcher)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("Suffix"), FString(TEXT("_LOD0"))))
        {
            const auto bCaseSensitiveMatch =
                TestTrue(TEXT("Matching suffix should pass when case-sensitive"), Matcher->Test(Object));
            if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("Suffix"), FString(TEXT("_lod0"))))
            {
                const auto bCaseSensitiveMiss =
                    TestFalse(TEXT("Different case should fail when case-sensitive"), Matcher->Test(Object));
                if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("bCaseSensitive"), false))
                {
                    const auto bCaseInsensitiveMatch =
                        TestTrue(TEXT("Different case should pass when case-insensitive"), Matcher->Test(Object));
                    return bCaseSensitiveMatch && bCaseSensitiveMiss && bCaseInsensitiveMatch;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNameWildcardMatcherMatchesAndRejectsPatternsTest,
                                 "RuleRanger.Matchers.Name.Wildcard.MatchesAndRejectsPatterns",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerNameWildcardMatcherMatchesAndRejectsPatternsTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UNameWildcardMatcher>();
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("SM_Crate_LOD1"));
    if (TestNotNull(TEXT("Wildcard matcher should be created"), Matcher)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("WildcardPattern"), FString(TEXT("SM_*_LOD?"))))
        {
            const auto bPositiveMatch =
                TestTrue(TEXT("Wildcard should match the expected pattern"), Matcher->Test(Object));
            if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("WildcardPattern"), FString(TEXT("MI_*"))))
            {
                const auto bNegativeMatch =
                    TestFalse(TEXT("Wildcard should reject non-matching patterns"), Matcher->Test(Object));
                return bPositiveMatch && bNegativeMatch;
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
