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
    #include "RuleRanger/Actions/RuleRanger/EnsureRuleRangerRuleSetReferencesResolveAction.h"
    #include "RuleRangerProjectRule.h"
    #include "RuleRangerRuleSet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureRuleSetReferencesResolveActionDetectsMissingEntriesTest,
                                 "RuleRanger.Actions.RuleRanger.RuleSetReferencesResolve.DetectsMissingEntries",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRuleSetReferencesResolveActionDetectsMissingEntriesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRuleRangerRuleSetReferencesResolveAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRuleRangerRuleSetReferencesResolveAction should be created"), Action)
        && TestNotNull(TEXT("RuleRangerRuleSet should be created"), Object))
    {
        TArray<TObjectPtr<URuleRangerRuleSet>> RuleSets;
        TArray<TObjectPtr<URuleRangerProjectRule>> ProjectRules;
        TArray<TObjectPtr<URuleRangerRule>> Rules;
        RuleSets.Add(nullptr);
        ProjectRules.Add(nullptr);
        Rules.Add(nullptr);
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("RuleSets"), RuleSets)
            && RuleRangerTests::SetPropertyValue(*this, Object, TEXT("ProjectRules"), ProjectRules)
            && RuleRangerTests::SetPropertyValue(*this, Object, TEXT("Rules"), Rules))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("RuleSet reference validation should target RuleRangerRuleSet"),
                             Action->GetExpectedType(),
                             URuleRangerRuleSet::StaticClass())
                && TestEqual(TEXT("Missing RuleSet, ProjectRule, and Rule references should add three errors"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             3)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Missing RuleSet references should mention ProjectRule entries"),
                       TEXT("ProjectRule entry at index 0"))
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("Missing Rule references should mention Rule entries"),
                                                          TEXT("Rule entry at index 0"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureRuleSetReferencesResolveActionPassesResolvedEntriesTest,
                                 "RuleRanger.Actions.RuleRanger.RuleSetReferencesResolve.PassesResolvedEntries",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRuleSetReferencesResolveActionPassesResolvedEntriesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRuleRangerRuleSetReferencesResolveAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>();
    const auto ChildRuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Object);
    const auto ProjectRule = RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(Object);
    const auto Rule = RuleRangerTests::NewTransientObject<URuleRangerRule>(Object);
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRuleRangerRuleSetReferencesResolveAction should be created"), Action)
        && TestNotNull(TEXT("RuleRangerRuleSet should be created"), Object)
        && TestNotNull(TEXT("Child RuleRangerRuleSet should be created"), ChildRuleSet)
        && TestNotNull(TEXT("RuleRangerProjectRule should be created"), ProjectRule)
        && TestNotNull(TEXT("RuleRangerRule should be created"), Rule))
    {
        TArray<TObjectPtr<URuleRangerRuleSet>> RuleSets;
        TArray<TObjectPtr<URuleRangerProjectRule>> ProjectRules;
        TArray<TObjectPtr<URuleRangerRule>> Rules;
        RuleSets.Add(ChildRuleSet);
        ProjectRules.Add(ProjectRule);
        Rules.Add(Rule);
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("RuleSets"), RuleSets)
            && RuleRangerTests::SetPropertyValue(*this, Object, TEXT("ProjectRules"), ProjectRules)
            && RuleRangerTests::SetPropertyValue(*this, Object, TEXT("Rules"), Rules))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestTrue(TEXT("Resolved RuleSet references should not add context errors"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty());
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
