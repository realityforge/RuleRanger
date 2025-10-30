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
#include "RuleRangerActionContext.h"
#include "RuleRangerRule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RuleRangerActionContext)

void URuleRangerActionContext::ResetContext(URuleRangerConfig* const InConfig,
                                            URuleRangerRuleSet* const InRuleSet,
                                            URuleRangerRule* const InRule,
                                            UObject* const InObject,
                                            const ERuleRangerActionTrigger InActionTrigger)
{
    check(InConfig);
    check(InRuleSet);
    check(InRule);
    check(InObject);
    Super::ResetContext(InConfig, InRuleSet);
    Rule = InRule;
    Object = InObject;
    ActionTrigger = InActionTrigger;
}

void URuleRangerActionContext::ClearContext()
{
    Super::ClearContext();
    Rule = nullptr;
    Object = nullptr;
    ActionTrigger = ERuleRangerActionTrigger::AT_Report;
}
