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
#include "RuleRangerProjectRule.h"
#include "Logging/StructuredLog.h"
#include "RuleRangerLogging.h"
#include "RuleRangerProjectAction.h"
#include "RuleRangerProjectActionContext.h"
#include "UObject/ObjectSaveContext.h"
#if WITH_EDITOR
    #include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(RuleRangerProjectRule)

void URuleRangerProjectRule::Apply(URuleRangerProjectActionContext* ActionContext)
{
    int32 ActionIndex = 0;
    for (const auto Action : Actions)
    {
        if (!IsValid(Action))
        {
            ActionContext->Error(FText::Format(
                NSLOCTEXT("RuleRanger", "InvalidActionAtIndex", "Invalid Action detected at index {0} in rule '{1}'"),
                FText::AsNumber(ActionIndex),
                FText::FromString(GetName())));
        }
        else
        {
            Action->Apply(ActionContext);
            const auto State = ActionContext->GetState();
            if (ERuleRangerActionState::AS_Fatal == State)
            {
                UE_LOGFMT(LogRuleRanger,
                          Verbose,
                          "ApplyRule() on rule {Rule} applied action {Action} which "
                          "resulted in fatal error. Processing rules will not continue.",
                          GetName(),
                          Action->GetName());
                return;
            }
            else if (!bContinueOnError && ERuleRangerActionState::AS_Error == State)
            {
                UE_LOGFMT(LogRuleRanger,
                          Verbose,
                          "ApplyRule() on rule {Rule} applied action {Action} which "
                          "resulted in error. Processing rules will not continue as ContinueOnError=False.",
                          GetName(),
                          Action->GetName());
                return;
            }
        }
        ActionIndex++;
    }
}

void URuleRangerProjectRule::PreSave(const FObjectPreSaveContext SaveContext)
{
    // Remove invalid entries but preserve author-specified order
    Actions.RemoveAll([](const auto& Value) { return Value == nullptr; });

    // If there is exactly one action and the rule has no description,
    // derive the description from the single action's display name.
    if (Description.TrimStartAndEnd().IsEmpty() && 1 == Actions.Num())
    {
        const auto& Action = Actions[0];
        if (IsValid(Action))
        {
            Description = Action->GetClass()->GetDisplayNameText().ToString().TrimStartAndEnd();
        }
    }

    Super::PreSave(SaveContext);
}

#if WITH_EDITOR
EDataValidationResult URuleRangerProjectRule::IsDataValid(FDataValidationContext& Context) const
{
    auto Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

    // Count valid actions only
    int32 ValidActionCount{ 0 };
    for (const auto& Action : Actions)
    {
        if (IsValid(Action))
        {
            ++ValidActionCount;
        }
    }

    if (0 == ValidActionCount)
    {
        Context.AddError(
            NSLOCTEXT("RuleRanger",
                      "RuleHasNoActions",
                      "RuleRangerProjectRule has no actions. Please add at least one action to the rule."));
        Result = EDataValidationResult::Invalid;
    }

    return Result;
}
#endif
