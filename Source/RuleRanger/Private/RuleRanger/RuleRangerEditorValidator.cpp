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
#include "RuleRanger/RuleRangerEditorValidator.h"
#include "Logging/StructuredLog.h"
#include "Misc/DataValidation.h"
#include "RuleRanger/RuleRangerEditorSubsystem.h"
#include "RuleRangerActionContext.h"
#include "RuleRangerLogging.h"
#include "RuleRangerRule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RuleRangerEditorValidator)

class UEditorAssetSubsystem;

URuleRangerEditorValidator::URuleRangerEditorValidator()
{
    bIsEnabled = true;
}

EDataValidationResult URuleRangerEditorValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData,
                                                                                     UObject* InAsset,
                                                                                     FDataValidationContext& Context)
{
    // ReSharper disable once CppTooWideScope
    if (const auto SubSystem = GEditor ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>() : nullptr)
    {
        const bool bIsSave = EDataValidationUsecase::Save == Context.GetValidationUsecase();
        SubSystem->ValidateObject(InAsset, bIsSave, this);
    }
    else
    {
        UE_LOGFMT(LogRuleRanger,
                  Error,
                  "OnAssetValidate({Asset}) unable to locate RuleRangerEditorSubsystem.",
                  InAsset->GetName());
    }

    if (EDataValidationResult::NotValidated == GetValidationResult())
    {
        AssetPasses(InAsset);
    }
    return GetValidationResult();
}

void URuleRangerEditorValidator::OnRuleApplied(URuleRangerActionContext* InActionContext)
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        if (const auto ResultHandler = Subsystem->GetDefaultResultHandler())
        {
            ResultHandler->OnRuleApplied(InActionContext);
        }
    }

    const auto InObject = InActionContext->GetObject();
    const auto WarningMessages = InActionContext->GetWarningMessages();
    for (int i = 0; i < WarningMessages.Num(); i++)
    {
        AssetWarning(InObject, WarningMessages[i]);
    }
    const auto ErrorMessages = InActionContext->GetErrorMessages();
    for (int i = 0; i < ErrorMessages.Num(); i++)
    {
        AssetFails(InObject, ErrorMessages[i]);
    }
    const auto FatalMessages = InActionContext->GetFatalMessages();
    for (int i = 0; i < FatalMessages.Num(); i++)
    {
        AssetFails(InObject, FatalMessages[i]);
    }
}

bool URuleRangerEditorValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData,
                                                                 UObject* InAsset,
                                                                 FDataValidationContext& InContext) const
{
    if (const auto SubSystem = GEditor ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>() : nullptr)
    {
        const bool bIsSave = EDataValidationUsecase::Save == InContext.GetValidationUsecase();
        return SubSystem->CanValidateObject(InAsset, bIsSave);
    }
    else
    {
        UE_LOGFMT(LogRuleRanger,
                  Error,
                  "CanValidateAsset_Implementation({Asset}) unable to locate RuleRangerEditorSubsystem.",
                  InAsset->GetName());
    }
    return false;
}
