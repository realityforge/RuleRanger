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
    #include "RuleRanger/Actions/RuleRanger/EnsureRuleRangerConfigReferencesResolveAction.h"
    #include "RuleRangerRuleSet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureConfigReferencesResolveActionDetectsMissingRuleSetEntriesTest,
                                 "RuleRanger.Actions.RuleRanger.ConfigReferencesResolve.DetectsMissingRuleSetEntries",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureConfigReferencesResolveActionDetectsMissingRuleSetEntriesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRuleRangerConfigReferencesResolveAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto RuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Object);
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRuleRangerConfigReferencesResolveAction should be created"), Action)
        && TestNotNull(TEXT("RuleRangerConfig should be created"), Object)
        && TestNotNull(TEXT("Referenced RuleRangerRuleSet should be created"), RuleSet))
    {
        TArray<TObjectPtr<URuleRangerRuleSet>> RuleSets;
        RuleSets.Add(RuleSet);
        RuleSets.Add(nullptr);
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("RuleSets"), RuleSets))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Config reference validation should target RuleRangerConfig"),
                             Action->GetExpectedType(),
                             URuleRangerConfig::StaticClass())
                && TestEqual(TEXT("Missing RuleSet references should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Missing RuleSet references should mention the missing index"),
                       TEXT("index 1"));
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
