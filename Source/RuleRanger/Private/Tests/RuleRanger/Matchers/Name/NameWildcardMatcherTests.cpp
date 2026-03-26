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
    #include "RuleRanger/Matchers/Name/NameWildcardMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

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

#endif
