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
#include "ImportActionContext.h"
#include "RuleRangerActionContext.h"
#include "RuleRangerDeveloperSettings.h"
#include "RuleRangerLogging.h"
#include "RuleRangerRule.h"

URuleRangerEditorValidator::URuleRangerEditorValidator()
{
    bIsEnabled = true;
}

bool URuleRangerEditorValidator::CanValidateAsset_Implementation(UObject* InAsset) const
{
    const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
    check(IsValid(DeveloperSettings));
    UE_LOG(RuleRanger,
           Verbose,
           TEXT("CanValidateAsset(%s) discovered %d Rule Set(s)"),
           *InAsset->GetName(),
           DeveloperSettings->Rules.Num());
    for (auto RuleSetIt = DeveloperSettings->Rules.CreateIterator(); RuleSetIt; ++RuleSetIt)
    {
        const auto RuleSet = RuleSetIt->LoadSynchronous();
        if (const auto Path = InAsset->GetPathName(); Path.StartsWith(RuleSet->Dir.Path))
        {
            UE_LOG(RuleRanger,
                   Verbose,
                   TEXT("CanValidateAsset(%s) processing Rule Set %s"),
                   *InAsset->GetName(),
                   *RuleSet->GetName());
            for (const auto RulePtr : RuleSet->Rules)
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto Rule = RulePtr.Get();
                if (Rule->bApplyOnValidate)
                {
                    UE_LOG(RuleRanger,
                           Verbose,
                           TEXT("CanValidateAsset(%s) detectedapplicable rule %s."),
                           *InAsset->GetName(),
                           *Rule->GetName());
                    return true;
                }
            }
        }
    }
    // If we get here there is no rules that apply to the asset
    return false;
}

EDataValidationResult URuleRangerEditorValidator::ValidateLoadedAsset_Implementation(UObject* InAsset,
                                                                                     TArray<FText>& ValidationErrors)
{
    if (IsValid(InAsset))
    {
        if (!ActionContext)
        {
            UE_LOG(RuleRanger, Verbose, TEXT("RuleRangerEditorSubsystem: Creating the initial ActionContext"));
            ActionContext = NewObject<UImportActionContext>(this, UImportActionContext::StaticClass());
        }

        const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
        check(IsValid(DeveloperSettings));
        UE_LOG(RuleRanger,
               Verbose,
               TEXT("OnAssetValidate(%s) discovered %d Rule Set(s)"),
               *InAsset->GetName(),
               DeveloperSettings->Rules.Num());
        for (auto RuleSetIt = DeveloperSettings->Rules.CreateIterator(); RuleSetIt; ++RuleSetIt)
        {
            const auto RuleSet = RuleSetIt->LoadSynchronous();
            if (const auto Path = InAsset->GetPathName(); Path.StartsWith(RuleSet->Dir.Path))
            {
                UE_LOG(RuleRanger,
                       Verbose,
                       TEXT("OnAssetValidate(%s) processing Rule Set %s"),
                       *InAsset->GetName(),
                       *RuleSet->GetName());
                for (const auto RulePtr : RuleSet->Rules)
                {
                    // ReSharper disable once CppTooWideScopeInitStatement
                    const auto Rule = RulePtr.Get();
                    if (Rule->bApplyOnValidate)
                    {
                        UE_LOG(RuleRanger,
                               Verbose,
                               TEXT("OnAssetValidate(%s) detected applicable rule %s."),
                               *InAsset->GetName(),
                               *Rule->GetName());
                        
                        ActionContext->ResetContext(InAsset, ERuleRangerActionTrigger::AT_Validate);

                        TScriptInterface<IRuleRangerActionContext> ScriptInterfaceActionContext(ActionContext);
                        Rule->Apply(ScriptInterfaceActionContext, InAsset);

                        for (int i = 0; i < ActionContext->GetWarningMessages().Num(); i++)
                        {
                            AssetWarning(InAsset, ActionContext->GetWarningMessages()[i]);
                        }
                        for (int i = 0; i < ActionContext->GetErrorMessages().Num(); i++)
                        {
                            AssetFails(InAsset, ActionContext->GetErrorMessages()[i], ValidationErrors);
                        }
                        for (int i = 0; i < ActionContext->GetFatalMessages().Num(); i++)
                        {
                            AssetFails(InAsset, ActionContext->GetFatalMessages()[i], ValidationErrors);
                        }
                    }
                }
            }
        }

        const auto State = ActionContext->GetState();
        if (ERuleRangerActionState::AS_Success == State || ERuleRangerActionState::AS_Warning == State)
        {
            AssetPasses(InAsset);
            return EDataValidationResult::Valid;
        }
        else
        {
            return EDataValidationResult::Invalid;
        }
    }
    else
    {
        return EDataValidationResult::NotValidated;
    }
}
