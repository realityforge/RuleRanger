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
    #include "RuleRanger/Matchers/Path/PathLengthMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerPathLengthMatcherHonorsThresholdTest,
                                 "RuleRanger.Matchers.Path.Length.HonorsThreshold",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerPathLengthMatcherHonorsThresholdTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UPathLengthMatcher>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(TEXT("/Game/ABCDE"), TEXT("LengthAsset"));
    if (TestNotNull(TEXT("PathLength matcher should be created"), Matcher)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("MaxPathLength"), 6))
        {
            const auto bAtThresholdMatch =
                TestTrue(TEXT("Package paths at the threshold should match"), Matcher->Test(Object));
            if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("MaxPathLength"), 7))
            {
                const auto bAboveThresholdMiss =
                    TestFalse(TEXT("Package paths below the threshold should not match"), Matcher->Test(Object));
                return bAtThresholdMatch && bAboveThresholdMiss;
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
