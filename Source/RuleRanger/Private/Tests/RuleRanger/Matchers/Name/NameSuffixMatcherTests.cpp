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
    #include "RuleRanger/Matchers/Name/NameSuffixMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

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

#endif
