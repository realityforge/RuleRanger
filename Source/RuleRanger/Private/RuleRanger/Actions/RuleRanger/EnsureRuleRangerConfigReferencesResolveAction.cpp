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
#include "EnsureRuleRangerConfigReferencesResolveAction.h"
#include "RuleRangerConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureRuleRangerConfigReferencesResolveAction)

void UEnsureRuleRangerConfigReferencesResolveAction::Apply_Implementation(URuleRangerActionContext* ActionContext,
                                                                          UObject* Object)
{
    const auto Config = CastChecked<URuleRangerConfig>(Object);

    for (auto RuleSetIt = Config->RuleSets.CreateIterator(); RuleSetIt; ++RuleSetIt)
    {
        if (!RuleSetIt->Get())
        {
            ActionContext->Error(FText::FromString(
                FString::Printf(TEXT("RuleRangerConfig has a RuleSet entry at index %d that does not resolve."),
                                RuleSetIt.GetIndex())));
        }
    }
}

UClass* UEnsureRuleRangerConfigReferencesResolveAction::GetExpectedType()
{
    return URuleRangerConfig::StaticClass();
}
