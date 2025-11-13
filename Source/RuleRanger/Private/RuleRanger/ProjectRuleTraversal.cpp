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
#include "RuleRanger/ProjectRuleTraversal.h"
#include "Logging/StructuredLog.h"
#include "RuleRangerConfig.h"
#include "RuleRangerLogging.h"
#include "RuleRangerProjectRule.h"
#include "RuleRangerRuleSet.h"

/*
 * Shared traversal utilities for project-level rules.
 */
namespace RuleRanger::Traversal
{
    static bool TraverseRuleSet(URuleRangerConfig* const Config,
                                URuleRangerRuleSet* const RuleSet,
                                const FProjectRuleVisitor& Visitor,
                                TSet<const URuleRangerRuleSet*>& Visited)
    {
        if (!IsValid(RuleSet))
        {
            return true; // skip invalid rule sets but keep going
        }
        if (Visited.Contains(RuleSet))
        {
            UE_LOGFMT(LogRuleRanger,
                      Error,
                      "Traversal: Detected cyclic reference involving Rule Set {RuleSet}. Skipping nested traversal.",
                      RuleSet->GetName());
            return true;
        }

        Visited.Add(RuleSet);

        // Recurse nested sets first
        for (const auto& NestedRuleSetPtr : RuleSet->RuleSets)
        {
            if (const auto Nested = NestedRuleSetPtr.Get())
            {
                if (!TraverseRuleSet(Config, Nested, Visitor, Visited))
                {
                    return false;
                }
            }
            else
            {
                UE_LOGFMT(LogRuleRanger,
                          Error,
                          "Traversal: Invalid Nested RuleSet skipped when processing project rules in {RuleSet}",
                          RuleSet->GetName());
            }
        }

        // Visit project rules
        int Index = 0;
        for (const auto RulePtr : RuleSet->ProjectRules)
        {
            if (const auto Rule = RulePtr.Get(); IsValid(Rule))
            {
                if (Rule->bApplyOnDemand)
                {
                    if (!Visitor(Config, RuleSet, Rule))
                    {
                        return false;
                    }
                }
            }
            else
            {
                UE_LOGFMT(LogRuleRanger,
                          Error,
                          "Traversal: Invalid ProjectRule skipped at index {Index} in rule set '{RuleSet}'",
                          Index,
                          RuleSet->GetName());
            }
            ++Index;
        }
        return true;
    }

    void TraverseProjectRulesForConfigs(const TConstArrayView<TWeakObjectPtr<URuleRangerConfig>> Configs,
                                        const FProjectRuleVisitor& Visitor)
    {
        TSet<const URuleRangerRuleSet*> Visited;
        bool bStop = false;
        for (const auto ConfigPtr : Configs)
        {
            if (bStop)
            {
                break;
            }
            if (const auto Config = ConfigPtr.Get())
            {
                for (const auto& RuleSetPtr : Config->RuleSets)
                {
                    if (!TraverseRuleSet(Config, RuleSetPtr.Get(), Visitor, Visited))
                    {
                        bStop = true;
                        break;
                    }
                }
            }
            else
            {
                UE_LOGFMT(LogRuleRanger, Error, "Traversal: Invalid Config skipped when processing project rules");
            }
        }
    }

    void TraverseProjectRulesForSoftConfigs(const TConstArrayView<TSoftObjectPtr<URuleRangerConfig>> Configs,
                                            const FProjectRuleVisitor& Visitor)
    {
        TSet<const URuleRangerRuleSet*> Visited;
        bool bStop = false;
        for (const auto& SoftConfig : Configs)
        {
            if (bStop)
            {
                break;
            }
            if (const auto Config = SoftConfig.LoadSynchronous())
            {
                for (const auto& RuleSetPtr : Config->RuleSets)
                {
                    if (!TraverseRuleSet(Config, RuleSetPtr.Get(), Visitor, Visited))
                    {
                        bStop = true;
                        break;
                    }
                }
            }
            else
            {
                UE_LOGFMT(LogRuleRanger, Error, "Traversal: Invalid Config in DeveloperSettings skipped");
            }
        }
    }

    int32 CountProjectRulesInRuleSet(const URuleRangerRuleSet* RuleSet, TSet<const URuleRangerRuleSet*>& Visited)
    {
        if (!IsValid(RuleSet) || Visited.Contains(RuleSet))
        {
            return 0;
        }
        else
        {
            Visited.Add(RuleSet);
            auto Count{ 0 };
            for (const auto RulePtr : RuleSet->ProjectRules)
            {
                if (const auto Rule = RulePtr.Get(); IsValid(Rule))
                {
                    if (Rule->bApplyOnDemand)
                    {
                        ++Count;
                    }
                }
            }
            for (const auto NestedPtr : RuleSet->RuleSets)
            {
                Count += CountProjectRulesInRuleSet(NestedPtr.Get(), Visited);
            }
            return Count;
        }
    }

    int32 CountProjectRulesForConfigs(const TConstArrayView<TWeakObjectPtr<URuleRangerConfig>> Configs)
    {
        int32 Total = 0;
        TSet<const URuleRangerRuleSet*> Visited;
        for (const auto ConfigPtr : Configs)
        {
            if (const auto Config = ConfigPtr.Get())
            {
                for (const auto& RuleSetPtr : Config->RuleSets)
                {
                    Total += CountProjectRulesInRuleSet(RuleSetPtr.Get(), Visited);
                }
            }
        }
        return Total;
    }

    int32 CountProjectRulesForSoftConfigs(const TConstArrayView<TSoftObjectPtr<URuleRangerConfig>> Configs)
    {
        int32 Total = 0;
        TSet<const URuleRangerRuleSet*> Visited;
        for (const auto& SoftConfig : Configs)
        {
            if (const auto Config = SoftConfig.LoadSynchronous())
            {
                for (const auto& RuleSetPtr : Config->RuleSets)
                {
                    Total += CountProjectRulesInRuleSet(RuleSetPtr.Get(), Visited);
                }
            }
        }
        return Total;
    }
} // namespace RuleRanger::Traversal
