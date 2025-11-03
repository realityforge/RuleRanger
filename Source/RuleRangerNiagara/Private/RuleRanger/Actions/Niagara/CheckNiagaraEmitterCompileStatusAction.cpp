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
#include "CheckNiagaraEmitterCompileStatusAction.h"
#include "NiagaraScript.h"
#include "NiagaraScriptSource.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CheckNiagaraEmitterCompileStatusAction)

bool UCheckNiagaraEmitterCompileStatusAction::ValidateScript(URuleRangerActionContext* ActionContext,
                                                             const UObject* Object,
                                                             UNiagaraScript* Script) const
{
    const auto VMExecutableData = Script->GetVMExecutableData();
    const auto Name = Script->GetName();
    // ReSharper disable once CppTooWideScope
    const auto CompileStatus =
        VMExecutableData.IsValid() ? VMExecutableData.LastCompileStatus : Script->GetLastCompileStatus();
    switch (CompileStatus)
    {
        case ENiagaraScriptCompileStatus::NCS_BeingCreated:
            LogInfo(Object,
                    FString::Printf(TEXT("NiagaraEmitter status is BeingCreated for "
                                         "script %s. Status valid."),
                                    *Name));
            break;

        case ENiagaraScriptCompileStatus::NCS_Dirty:
            ActionContext->Error(
                FText::Format(NSLOCTEXT("RuleRanger",
                                        "NiagaraEmitterDirty",
                                        "NiagaraEmitter status is dirty for script {0} and needs to be recompiled"),
                              FText::FromString(Name)));
            return false;

        case ENiagaraScriptCompileStatus::NCS_UpToDate:
            LogInfo(Object,
                    FString::Printf(TEXT("NiagaraEmitter status is UpToDate for "
                                         "script %s. Status valid."),
                                    *Name));
            break;
        case ENiagaraScriptCompileStatus::NCS_Error:
            ActionContext->Error(FText::Format(NSLOCTEXT("RuleRanger",
                                                         "NiagaraEmitterError",
                                                         "NiagaraEmitter has an error status for script {0}. "
                                                         "Fix errors and recompile NiagaraEmitter."),
                                               FText::FromString(Name)));
            return false;
            ;
        case ENiagaraScriptCompileStatus::NCS_UpToDateWithWarnings:
        case ENiagaraScriptCompileStatus::NCS_ComputeUpToDateWithWarnings:
            if (bErrorOnUpToDateWithWarnings)
            {
                ActionContext->Error(FText::Format(
                    NSLOCTEXT("RuleRanger",
                              "NiagaraEmitterWarnings",
                              "NiagaraEmitter is UpToDate but has warnings for script {0}. "
                              "Fix warnings and recompile NiagaraEmitter or set bErrorOnUpToDateWithWarnings=false"),
                    FText::FromString(Name)));
                return false;
            }
            else
            {
                LogInfo(Object,
                        FString::Printf(TEXT("NiagaraEmitter status is UpToDate but has warnings "
                                             "for script %s and bErrorOnUpToDateWithWarnings=false. Status valid."),
                                        *Name));
            }
            break;
        case ENiagaraScriptCompileStatus::NCS_Unknown:
        default:
            if (bErrorOnUnknown)
            {
                ActionContext->Error(FText::Format(NSLOCTEXT("RuleRanger",
                                                             "NiagaraEmitterUnknown",
                                                             "NiagaraEmitter has an Unknown status for script {0}. "
                                                             "Recompile NiagaraEmitter or set bErrorOnUnknown to true"),
                                                   FText::FromString(Name)));
                return false;
            }
            else
            {
                LogInfo(Object,
                        FString::Printf(TEXT("NiagaraEmitter status is Unknown for script "
                                             "%s and bErrorOnUnknown=false. Status valid."),
                                        *Name));
            }
    }
    return true;
}

void UCheckNiagaraEmitterCompileStatusAction::Apply(URuleRangerActionContext* ActionContext, UObject* Object)
{
    const auto Emitter = CastChecked<UNiagaraEmitter>(Object);
    if (const auto& EmitterData = Emitter->GetLatestEmitterData())
    {
        TArray<UNiagaraScript*> Scripts;

        EmitterData->GetScripts(Scripts, true, true);
        for (const auto Script : Scripts)
        {
            if (!ValidateScript(ActionContext, Object, Script))
            {
                return;
            }
        }
    }
}

UClass* UCheckNiagaraEmitterCompileStatusAction::GetExpectedType() const
{
    return UNiagaraEmitter::StaticClass();
}
