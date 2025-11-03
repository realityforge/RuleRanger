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
#include "RunDataValidationAction.h"
#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RunDataValidationAction)

void URunDataValidationAction::Apply(URuleRangerActionContext* ActionContext, UObject* Object)
{
    // Avoid recursion: when invoked from Data Validation (Validate/Save),
    // calling IsDataValid here would re-invoke the RuleRanger validator.
    const auto Trigger = ActionContext->GetActionTrigger();
    if (ERuleRangerActionTrigger::AT_Validate != Trigger && ERuleRangerActionTrigger::AT_Save != Trigger)
    {

        FDataValidationContext ValidationContext(true, EDataValidationUsecase::Manual, TConstArrayView<FAssetData>{});

        const UObject* ConstObject = Object;
        const EDataValidationResult Result = ConstObject->IsDataValid(ValidationContext);

        auto ErrorCount{ 0 };

        TArray<FDataValidationContext::FIssue> Issues = ValidationContext.GetIssues();
        for (const auto Issue : Issues)
        {
            if (EMessageSeverity::Info == Issue.Severity)
            {
                ActionContext->Info(Issue.Message);
            }
            else if (EMessageSeverity::Warning == Issue.Severity
                     || EMessageSeverity::PerformanceWarning == Issue.Severity)
            {
                ActionContext->Warning(Issue.Message);
            }
            else
            {
                ActionContext->Error(Issue.Message);
                ErrorCount++;
            }
        }

        if (EDataValidationResult::Invalid == Result && 0 == ErrorCount)
        {
            ActionContext->Error(NSLOCTEXT("RuleRanger",
                                           "RunDataValidation_GenericFailure",
                                           "Data Validation reported failure for this asset."));
        }
    }
}
