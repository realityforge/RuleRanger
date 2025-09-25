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
#include "RuleRangerDefaultResultHandler.h"
#include "Misc/UObjectToken.h"
#include "RuleRangerActionContext.h"
#include "RuleRangerMessageLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RuleRangerDefaultResultHandler)

void URuleRangerDefaultResultHandler::OnRuleApplied(URuleRangerActionContext* ActionContext)
{
    auto InfoMessages = ActionContext->GetInfoMessages();
    for (auto i = 0; i < InfoMessages.Num(); i++)
    {
        FMessageLog(FRuleRangerMessageLog::GetMessageLogName())
            .Info()
            ->AddToken(FUObjectToken::Create(ActionContext->GetObject()))
            ->AddToken(FTextToken::Create(InfoMessages[i]));
    }
    auto WarningMessages = ActionContext->GetWarningMessages();
    for (auto i = 0; i < WarningMessages.Num(); i++)
    {
        FMessageLog(FRuleRangerMessageLog::GetMessageLogName())
            .Warning()
            ->AddToken(FUObjectToken::Create(ActionContext->GetObject()))
            ->AddToken(FTextToken::Create(WarningMessages[i]));
    }
    auto ErrorMessages = ActionContext->GetErrorMessages();
    for (auto i = 0; i < ErrorMessages.Num(); i++)
    {
        FMessageLog(FRuleRangerMessageLog::GetMessageLogName())
            .Error()
            ->AddToken(FUObjectToken::Create(ActionContext->GetObject()))
            ->AddToken(FTextToken::Create(ErrorMessages[i]));
    }
    auto FatalMessages = ActionContext->GetFatalMessages();
    for (auto i = 0; i < FatalMessages.Num(); i++)
    {
        FMessageLog(FRuleRangerMessageLog::GetMessageLogName())
            .Error()
            ->AddToken(FUObjectToken::Create(ActionContext->GetObject()))
            ->AddToken(FTextToken::Create(FatalMessages[i]));
    }
}
