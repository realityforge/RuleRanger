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
#include "RuleRanger/RuleRangerEditorSubsystem.h"
#include "RuleRangerActionContext.h"
#include "RuleRangerLogging.h"
#include "RuleRangerRule.h"

URuleRangerEditorValidator::URuleRangerEditorValidator()
{
    bIsEnabled = true;
}

static FString DescribeDataValidationUsecase(const EDataValidationUsecase InDataValidationUsecase)
{
    switch (InDataValidationUsecase)
    {
        case EDataValidationUsecase::None:
            return TEXT("None");
        case EDataValidationUsecase::Manual:
            return TEXT("Manual");
        case EDataValidationUsecase::Commandlet:
            return TEXT("Commandlet");
        case EDataValidationUsecase::Save:
            return TEXT("Save");
        case EDataValidationUsecase::PreSubmit:
            return TEXT("PreSubmit");
        case EDataValidationUsecase::Script:
        default:
            return TEXT("Script");
    }
}

EDataValidationResult URuleRangerEditorValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData,
                                                                                     UObject* InAsset,
                                                                                     FDataValidationContext& Context)
{
    if (!ActionContext)
    {
        UE_LOG(RuleRanger, VeryVerbose, TEXT("RuleRangerEditorSubsystem: Creating the initial ActionContext"));
        ActionContext = NewObject<URuleRangerActionContext>(this, URuleRangerActionContext::StaticClass());
    }

    // ReSharper disable once CppTooWideScope
    const auto SubSystem = GEditor ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>() : nullptr;
    if (SubSystem)
    {
        SubSystem->ProcessRule(InAsset, [this, &Context](URuleRangerRule* Rule, UObject* InObject) mutable {
            return ProcessRule(Rule, InObject, Context);
        });
    }
    else
    {
        UE_LOG(RuleRanger,
               Error,
               TEXT("OnAssetValidate(%s) unable to locate RuleRangerEditorSubsystem."),
               *InAsset->GetName());
    }

    if (EDataValidationResult::NotValidated == GetValidationResult())
    {
        AssetPasses(InAsset);
    }
    return GetValidationResult();
}

bool URuleRangerEditorValidator::ProcessRule(URuleRangerRule* Rule, UObject* InObject, FDataValidationContext& Context)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const bool bIsSave = EDataValidationUsecase::Save == Context.GetValidationUsecase();
    if ((!bIsSave && Rule->bApplyOnValidate) || (bIsSave && Rule->bApplyOnSave))
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("OnAssetValidate(%s) detected applicable rule %s."),
               *InObject->GetName(),
               *Rule->GetName());

        ActionContext->ResetContext(Rule,
                                    InObject,
                                    bIsSave ? ERuleRangerActionTrigger::AT_Save
                                            : ERuleRangerActionTrigger::AT_Validate);

        Rule->Apply(ActionContext, InObject);

        ActionContext->EmitMessageLogs();

        for (int i = 0; i < ActionContext->GetWarningMessages().Num(); i++)
        {
            AssetWarning(InObject, ActionContext->GetWarningMessages()[i]);
        }
        for (int i = 0; i < ActionContext->GetErrorMessages().Num(); i++)
        {
            AssetFails(InObject, ActionContext->GetErrorMessages()[i]);
        }
        for (int i = 0; i < ActionContext->GetFatalMessages().Num(); i++)
        {
            AssetFails(InObject, ActionContext->GetFatalMessages()[i]);
        }
        return ActionContext->GetFatalMessages().Num() <= 0;
    }
    else
    {
        return true;
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool URuleRangerEditorValidator::WillRuleRunInDataValidationUsecase(
    const URuleRangerRule* Rule,
    const UObject* InObject,
    const EDataValidationUsecase DataValidationUsecase) const
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const bool bIsSave = EDataValidationUsecase::Save == DataValidationUsecase;
    if (!bIsSave && Rule->bApplyOnValidate)
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("CanValidateAsset(%s) detected applicable rule %s in usecase %s."),
               *InObject->GetName(),
               *Rule->GetName(),
               *DescribeDataValidationUsecase(DataValidationUsecase));
        return true;
    }
    else if (bIsSave && Rule->bApplyOnSave)
    {
        UE_LOG(RuleRanger,
               VeryVerbose,
               TEXT("CanValidateAsset(%s) detected applicable rule %s in usecase %s."),
               *InObject->GetName(),
               *Rule->GetName(),
               *DescribeDataValidationUsecase(DataValidationUsecase));
        return true;
    }
    else
    {
        return false;
    }
}

bool URuleRangerEditorValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData,
                                                                 UObject* InAsset,
                                                                 FDataValidationContext& InContext) const
{
    bool Result = false;
    if (const auto SubSystem = GEditor ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>() : nullptr)
    {
        SubSystem->ProcessRule(InAsset, [this, &InContext, &Result](const URuleRangerRule* Rule, UObject* InObject) {
            if (WillRuleRunInDataValidationUsecase(Rule, InObject, InContext.GetValidationUsecase())
                && Rule->Match(ActionContext, InObject))
            {
                Result = true;
            }
            return true;
        });
    }
    else
    {
        UE_LOG(RuleRanger,
               Error,
               TEXT("CanValidateAsset_Implementation(%s) unable to locate RuleRangerEditorSubsystem."),
               *InAsset->GetName());
    }

    return Result;
}
