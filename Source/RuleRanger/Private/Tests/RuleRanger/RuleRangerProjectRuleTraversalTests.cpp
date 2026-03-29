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
    #include "RuleRanger/ProjectRuleTraversal.h"
    #include "RuleRangerConfig.h"
    #include "RuleRangerProjectRule.h"
    #include "RuleRangerRuleSet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerProjectRuleTraversalTests
{
    bool SetRuleSets(FAutomationTestBase& Test,
                     URuleRangerConfig* Config,
                     const TArray<TObjectPtr<URuleRangerRuleSet>>& RuleSets)
    {
        return RuleRangerTests::SetPropertyValue(Test, Config, TEXT("RuleSets"), RuleSets);
    }

    bool SetNestedRuleSets(FAutomationTestBase& Test,
                           URuleRangerRuleSet* RuleSet,
                           const TArray<TObjectPtr<URuleRangerRuleSet>>& NestedRuleSets)
    {
        return RuleRangerTests::SetPropertyValue(Test, RuleSet, TEXT("RuleSets"), NestedRuleSets);
    }

    bool SetProjectRules(FAutomationTestBase& Test,
                         URuleRangerRuleSet* RuleSet,
                         const TArray<TObjectPtr<URuleRangerProjectRule>>& ProjectRules)
    {
        return RuleRangerTests::SetPropertyValue(Test, RuleSet, TEXT("ProjectRules"), ProjectRules);
    }

    bool SetApplyOnDemand(FAutomationTestBase& Test, URuleRangerProjectRule* Rule, const bool bApplyOnDemand)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Rule, TEXT("bApplyOnDemand"), bApplyOnDemand);
    }
} // namespace RuleRangerProjectRuleTraversalTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectRuleTraversalTraversesNestedSetsAndSkipsInvalidEntriesTest,
                                 "RuleRanger.ProjectTraversal.TraversesNestedSetsAndSkipsInvalidEntries",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectRuleTraversalTraversesNestedSetsAndSkipsInvalidEntriesTest::RunTest(const FString&)
{
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto RootRuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("RootRuleSet"));
    const auto NestedRuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("NestedRuleSet"));
    const auto RootRule = RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(RootRuleSet, TEXT("RootRule"));
    const auto NestedRule =
        RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(NestedRuleSet, TEXT("NestedRule"));
    const auto SkippedRule =
        RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(RootRuleSet, TEXT("SkippedRule"));
    if (TestNotNull(TEXT("Config should be created"), Config)
        && TestNotNull(TEXT("Root rule set should be created"), RootRuleSet)
        && TestNotNull(TEXT("Nested rule set should be created"), NestedRuleSet)
        && TestNotNull(TEXT("Root project rule should be created"), RootRule)
        && TestNotNull(TEXT("Nested project rule should be created"), NestedRule)
        && TestNotNull(TEXT("Skipped project rule should be created"), SkippedRule))
    {
        AddExpectedMessagePlain(TEXT("Invalid Nested RuleSet skipped"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        AddExpectedMessagePlain(TEXT("Invalid ProjectRule skipped at index 1"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);

        if (RuleRangerProjectRuleTraversalTests::SetApplyOnDemand(*this, SkippedRule, false)
            && RuleRangerProjectRuleTraversalTests::SetProjectRules(*this, NestedRuleSet, { NestedRule })
            && RuleRangerProjectRuleTraversalTests::SetNestedRuleSets(*this, RootRuleSet, { NestedRuleSet, nullptr })
            && RuleRangerProjectRuleTraversalTests::SetProjectRules(*this,
                                                                    RootRuleSet,
                                                                    { RootRule, nullptr, SkippedRule })
            && RuleRangerProjectRuleTraversalTests::SetRuleSets(*this, Config, { RootRuleSet }))
        {
            TArray<FString> VisitedRules;
            const TArray<TWeakObjectPtr<URuleRangerConfig>> Configs{ Config };
            RuleRanger::Traversal::TraverseProjectRulesForConfigs(
                Configs,
                [&VisitedRules](URuleRangerConfig* InConfig,
                                URuleRangerRuleSet* InRuleSet,
                                URuleRangerProjectRule* InRule) {
                    VisitedRules.Add(FString::Printf(TEXT("%s:%s"), *InRuleSet->GetName(), *InRule->GetName()));
                    return true;
                });

            return TestEqual(TEXT("Only apply-on-demand project rules should be visited"), VisitedRules.Num(), 2)
                && TestEqual(TEXT("Nested rule sets should be traversed before the parent rule set"),
                             VisitedRules[0],
                             FString(TEXT("NestedRuleSet:NestedRule")))
                && TestEqual(TEXT("Parent project rules should be visited after nested rule sets"),
                             VisitedRules[1],
                             FString(TEXT("RootRuleSet:RootRule")));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectRuleTraversalStopsWhenVisitorRequestsStopTest,
                                 "RuleRanger.ProjectTraversal.StopsWhenVisitorRequestsStop",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectRuleTraversalStopsWhenVisitorRequestsStopTest::RunTest(const FString&)
{
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto FirstRuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("FirstRuleSet"));
    const auto SecondRuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("SecondRuleSet"));
    const auto FirstRule = RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(FirstRuleSet, TEXT("FirstRule"));
    const auto SecondRule =
        RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(SecondRuleSet, TEXT("SecondRule"));
    if (TestNotNull(TEXT("Config should be created"), Config)
        && TestNotNull(TEXT("First rule set should be created"), FirstRuleSet)
        && TestNotNull(TEXT("Second rule set should be created"), SecondRuleSet)
        && TestNotNull(TEXT("First project rule should be created"), FirstRule)
        && TestNotNull(TEXT("Second project rule should be created"), SecondRule))
    {
        if (RuleRangerProjectRuleTraversalTests::SetProjectRules(*this, FirstRuleSet, { FirstRule })
            && RuleRangerProjectRuleTraversalTests::SetProjectRules(*this, SecondRuleSet, { SecondRule })
            && RuleRangerProjectRuleTraversalTests::SetRuleSets(*this, Config, { FirstRuleSet, SecondRuleSet }))
        {
            auto VisitCount = 0;
            const TArray<TWeakObjectPtr<URuleRangerConfig>> Configs{ Config };
            RuleRanger::Traversal::TraverseProjectRulesForConfigs(Configs,
                                                                  [&VisitCount](URuleRangerConfig* InConfig,
                                                                                URuleRangerRuleSet* InRuleSet,
                                                                                URuleRangerProjectRule* InRule) {
                                                                      VisitCount++;
                                                                      return false;
                                                                  });

            return TestEqual(TEXT("Traversal should stop after the first visitor-requested stop"), VisitCount, 1);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectRuleTraversalDetectsCyclesAndCountsApplyOnDemandRulesTest,
                                 "RuleRanger.ProjectTraversal.DetectsCyclesAndCountsApplyOnDemandRules",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectRuleTraversalDetectsCyclesAndCountsApplyOnDemandRulesTest::RunTest(const FString&)
{
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto RootRuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("CycleRoot"));
    const auto NestedRuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("CycleNested"));
    const auto RootRule = RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(RootRuleSet, TEXT("RootRule"));
    const auto NestedRule =
        RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(NestedRuleSet, TEXT("NestedRule"));
    const auto SkippedRule =
        RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(RootRuleSet, TEXT("SkippedRule"));
    if (TestNotNull(TEXT("Config should be created"), Config)
        && TestNotNull(TEXT("Root rule set should be created"), RootRuleSet)
        && TestNotNull(TEXT("Nested rule set should be created"), NestedRuleSet)
        && TestNotNull(TEXT("Root project rule should be created"), RootRule)
        && TestNotNull(TEXT("Nested project rule should be created"), NestedRule)
        && TestNotNull(TEXT("Skipped project rule should be created"), SkippedRule))
    {
        AddExpectedMessagePlain(TEXT("Detected cyclic reference involving Rule Set"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        if (RuleRangerProjectRuleTraversalTests::SetApplyOnDemand(*this, SkippedRule, false)
            && RuleRangerProjectRuleTraversalTests::SetNestedRuleSets(*this, RootRuleSet, { NestedRuleSet })
            && RuleRangerProjectRuleTraversalTests::SetNestedRuleSets(*this, NestedRuleSet, { RootRuleSet })
            && RuleRangerProjectRuleTraversalTests::SetProjectRules(*this, RootRuleSet, { RootRule, SkippedRule })
            && RuleRangerProjectRuleTraversalTests::SetProjectRules(*this, NestedRuleSet, { NestedRule })
            && RuleRangerProjectRuleTraversalTests::SetRuleSets(*this, Config, { RootRuleSet }))
        {
            auto VisitCount = 0;
            const TArray<TWeakObjectPtr<URuleRangerConfig>> Configs{ Config };
            RuleRanger::Traversal::TraverseProjectRulesForConfigs(Configs,
                                                                  [&VisitCount](URuleRangerConfig* InConfig,
                                                                                URuleRangerRuleSet* InRuleSet,
                                                                                URuleRangerProjectRule* InRule) {
                                                                      VisitCount++;
                                                                      return true;
                                                                  });

            TSet<const URuleRangerRuleSet*> Visited;
            const auto Count = RuleRanger::Traversal::CountProjectRulesInRuleSet(RootRuleSet, Visited);

            return TestEqual(TEXT("Traversal should visit each apply-on-demand project rule once despite the cycle"),
                             VisitCount,
                             2)
                && TestEqual(TEXT("CountProjectRulesInRuleSet should ignore non-demand rules and cycles"), Count, 2)
                && TestEqual(TEXT("CountProjectRulesForConfigs should match the in-rule-set count"),
                             RuleRanger::Traversal::CountProjectRulesForConfigs(Configs),
                             2);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectRuleTraversalSoftConfigsLoadSynchronouslyTest,
                                 "RuleRanger.ProjectTraversal.SoftConfigsLoadSynchronously",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectRuleTraversalSoftConfigsLoadSynchronouslyTest::RunTest(const FString&)
{
    const auto Config = RuleRangerTests::NewRegisteredPackagedAsset<URuleRangerConfig>(
        TEXT("/Game/Developers/RuleRangerTests/ProjectTraversal/SoftConfig"),
        TEXT("SoftConfig"));
    const auto RuleSet = Config ? NewObject<URuleRangerRuleSet>(Config, TEXT("SoftRuleSet")) : nullptr;
    const auto Rule = RuleSet ? NewObject<URuleRangerProjectRule>(RuleSet, TEXT("SoftRule")) : nullptr;
    if (TestNotNull(TEXT("Packaged config should be created"), Config)
        && TestNotNull(TEXT("Soft-config rule set should be created"), RuleSet)
        && TestNotNull(TEXT("Soft-config project rule should be created"), Rule))
    {
        if (RuleRangerProjectRuleTraversalTests::SetProjectRules(*this, RuleSet, { Rule })
            && RuleRangerProjectRuleTraversalTests::SetRuleSets(*this, Config, { RuleSet }))
        {
            auto VisitCount = 0;
            const TArray<TSoftObjectPtr<URuleRangerConfig>> Configs{ TSoftObjectPtr<URuleRangerConfig>(Config) };
            RuleRanger::Traversal::TraverseProjectRulesForSoftConfigs(Configs,
                                                                      [&VisitCount](URuleRangerConfig* InConfig,
                                                                                    URuleRangerRuleSet* InRuleSet,
                                                                                    URuleRangerProjectRule* InRule) {
                                                                          VisitCount++;
                                                                          return true;
                                                                      });

            return TestEqual(TEXT("Soft-config traversal should visit the packaged project rule"), VisitCount, 1)
                && TestEqual(TEXT("CountProjectRulesForSoftConfigs should count the packaged project rule"),
                             RuleRanger::Traversal::CountProjectRulesForSoftConfigs(Configs),
                             1);
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
