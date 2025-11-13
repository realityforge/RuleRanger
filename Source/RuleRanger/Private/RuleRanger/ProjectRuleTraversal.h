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
#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

class URuleRangerConfig;
class URuleRangerRuleSet;
class URuleRangerProjectRule;

namespace RuleRanger::Traversal
{
    using FProjectRuleVisitor =
        TFunctionRef<bool(URuleRangerConfig* Config, URuleRangerRuleSet* RuleSet, URuleRangerProjectRule* Rule)>;

    // Traverse rule sets for already-loaded configs
    void TraverseProjectRulesForConfigs(TConstArrayView<TWeakObjectPtr<URuleRangerConfig>> Configs,
                                        const FProjectRuleVisitor& Visitor);

    // Traverse rule sets for soft config references (loads synchronously)
    void TraverseProjectRulesForSoftConfigs(TConstArrayView<TSoftObjectPtr<URuleRangerConfig>> Configs,
                                            const FProjectRuleVisitor& Visitor);

    // Count utilities
    int32 CountProjectRulesInRuleSet(const URuleRangerRuleSet* RuleSet, TSet<const URuleRangerRuleSet*>& Visited);
    int32 CountProjectRulesForConfigs(TConstArrayView<TWeakObjectPtr<URuleRangerConfig>> Configs);
    int32 CountProjectRulesForSoftConfigs(TConstArrayView<TSoftObjectPtr<URuleRangerConfig>> Configs);
} // namespace RuleRanger::Traversal
