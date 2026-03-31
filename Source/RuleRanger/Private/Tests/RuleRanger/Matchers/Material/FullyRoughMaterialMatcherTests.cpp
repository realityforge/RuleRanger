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
    #include "RuleRanger/Matchers/Material/FullyRoughMaterialMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerFullyRoughMaterialMatcherTests
{
    bool
    SetExpectedValue(FAutomationTestBase& Test, UFullyRoughMaterialMatcher* const Matcher, const bool bExpectedValue)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Matcher, TEXT("bFullyRough"), bExpectedValue);
    }
} // namespace RuleRangerFullyRoughMaterialMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerFullyRoughMaterialMatcherMatchesTrueSettingTest,
                                 "RuleRanger.Matchers.Material.FullyRough.MatchesTrueSetting",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerFullyRoughMaterialMatcherMatchesTrueSettingTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UFullyRoughMaterialMatcher>();
    const auto Material =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/Tests/RuleRanger/Material/FullyRoughTrue"),
                                             TEXT("FullyRoughTrueMaterial"));
    if (TestNotNull(TEXT("FullyRoughMaterialMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Material should be created"), Material))
    {
        Material->bFullyRough = true;
        if (RuleRangerFullyRoughMaterialMatcherTests::SetExpectedValue(*this, Matcher, true))
        {
            return TestTrue(TEXT("Materials with matching true settings should match"), Matcher->Test(Material));
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerFullyRoughMaterialMatcherMatchesFalseSettingTest,
                                 "RuleRanger.Matchers.Material.FullyRough.MatchesFalseSetting",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerFullyRoughMaterialMatcherMatchesFalseSettingTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UFullyRoughMaterialMatcher>();
    const auto Material =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/Tests/RuleRanger/Material/FullyRoughFalse"),
                                             TEXT("FullyRoughFalseMaterial"));
    if (TestNotNull(TEXT("FullyRoughMaterialMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Material should be created"), Material))
    {
        Material->bFullyRough = false;
        if (RuleRangerFullyRoughMaterialMatcherTests::SetExpectedValue(*this, Matcher, false))
        {
            return TestTrue(TEXT("Materials with matching false settings should match"), Matcher->Test(Material));
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerFullyRoughMaterialMatcherRejectsNullAndWrongTypeTest,
                                 "RuleRanger.Matchers.Material.FullyRough.RejectsNullAndWrongType",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerFullyRoughMaterialMatcherRejectsNullAndWrongTypeTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UFullyRoughMaterialMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestObject>();
    if (TestNotNull(TEXT("FullyRoughMaterialMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Non-material object should be created"), Object))
    {
        return TestFalse(TEXT("Null should not match"), Matcher->Test(nullptr))
            && TestFalse(TEXT("Objects that are not materials should not match"), Matcher->Test(Object));
    }

    return false;
}

#endif
