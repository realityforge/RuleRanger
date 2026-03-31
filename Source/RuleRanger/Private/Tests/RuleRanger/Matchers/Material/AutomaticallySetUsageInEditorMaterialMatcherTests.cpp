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
    #include "RuleRanger/Matchers/Material/AutomaticallySetUsageInEditorMaterialMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerAutomaticallySetUsageInEditorMaterialMatcherTests
{
    bool SetExpectedValue(FAutomationTestBase& Test,
                          UAutomaticallySetUsageInEditorMaterialMatcher* const Matcher,
                          const bool bExpectedValue)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test,
                                                     Matcher,
                                                     TEXT("bAutomaticallySetUsageInEditor"),
                                                     bExpectedValue);
    }
} // namespace RuleRangerAutomaticallySetUsageInEditorMaterialMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerAutomaticallySetUsageInEditorMaterialMatcherMatchesTrueSettingTest,
                                 "RuleRanger.Matchers.Material.AutomaticallySetUsageInEditor.MatchesTrueSetting",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerAutomaticallySetUsageInEditorMaterialMatcherMatchesTrueSettingTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UAutomaticallySetUsageInEditorMaterialMatcher>();
    const auto Material =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/Tests/RuleRanger/Material/AutoUsageTrue"),
                                             TEXT("AutoUsageTrueMaterial"));
    if (TestNotNull(TEXT("AutomaticallySetUsageInEditorMaterialMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Material should be created"), Material))
    {
        Material->bAutomaticallySetUsageInEditor = true;
        if (RuleRangerAutomaticallySetUsageInEditorMaterialMatcherTests::SetExpectedValue(*this, Matcher, true))
        {
            return TestTrue(TEXT("Materials with matching true settings should match"), Matcher->Test(Material));
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerAutomaticallySetUsageInEditorMaterialMatcherMatchesFalseSettingTest,
                                 "RuleRanger.Matchers.Material.AutomaticallySetUsageInEditor.MatchesFalseSetting",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerAutomaticallySetUsageInEditorMaterialMatcherMatchesFalseSettingTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UAutomaticallySetUsageInEditorMaterialMatcher>();
    const auto Material =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/Tests/RuleRanger/Material/AutoUsageFalse"),
                                             TEXT("AutoUsageFalseMaterial"));
    if (TestNotNull(TEXT("AutomaticallySetUsageInEditorMaterialMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Material should be created"), Material))
    {
        Material->bAutomaticallySetUsageInEditor = false;
        if (RuleRangerAutomaticallySetUsageInEditorMaterialMatcherTests::SetExpectedValue(*this, Matcher, false))
        {
            return TestTrue(TEXT("Materials with matching false settings should match"), Matcher->Test(Material));
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerAutomaticallySetUsageInEditorMaterialMatcherRejectsNullAndWrongTypeTest,
                                 "RuleRanger.Matchers.Material.AutomaticallySetUsageInEditor.RejectsNullAndWrongType",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerAutomaticallySetUsageInEditorMaterialMatcherRejectsNullAndWrongTypeTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UAutomaticallySetUsageInEditorMaterialMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestObject>();
    if (TestNotNull(TEXT("AutomaticallySetUsageInEditorMaterialMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Non-material object should be created"), Object))
    {
        return TestFalse(TEXT("Null should not match"), Matcher->Test(nullptr))
            && TestFalse(TEXT("Objects that are not materials should not match"), Matcher->Test(Object));
    }

    return false;
}

#endif
