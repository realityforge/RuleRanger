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
    #include "RuleRanger/Matchers/Common/IsOutermostObjectMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerIsOutermostObjectMatcherDistinguishesPackagesFromPackagedObjectsTest,
    "RuleRanger.Matchers.Common.IsOutermostObject.DistinguishesPackagesFromPackagedObjects",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerIsOutermostObjectMatcherDistinguishesPackagesFromPackagedObjectsTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UIsOutermostObjectMatcher>();
    const auto Package = RuleRangerTests::NewTransientPackage(TEXT("/Game/RuleRangerTests/OutermostPackage"));
    const auto Object = Package
        ? RuleRangerTests::NewTransientObject<URuleRangerAutomationTestObject>(Package, TEXT("OutermostAsset"))
        : nullptr;
    if (TestNotNull(TEXT("IsOutermostObject matcher should be created"), Matcher)
        && TestNotNull(TEXT("Test package should be created"), Package)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        return TestTrue(TEXT("Packages should be considered outermost objects"), Matcher->Test(Package))
            && TestFalse(TEXT("Objects inside a package should not be considered outermost"), Matcher->Test(Object));
    }
    else
    {
        return false;
    }
}

#endif
