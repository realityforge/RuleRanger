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
#include "RuleRangerDefaultProjectResultHandler.h"
#include "Misc/UObjectToken.h"
#include "RuleRangerMessageLog.h"
#include "RuleRangerProjectActionContext.h"
#include "RuleRangerProjectRule.h"
#include "RuleRangerRuleSet.h"

void URuleRangerDefaultProjectResultHandler::OnProjectRuleApplied(URuleRangerProjectActionContext* Context)
{
    auto Log = FMessageLog(FRuleRangerMessageLog::GetMessageLogName());

    const auto Add = [&](const auto& InMessages, const auto Severity) {
        for (const auto& Msg : InMessages)
        {
            auto Message = FTokenizedMessage::Create(Severity);
            if (const auto RuleSet = Context->GetRuleSet())
            {
                Message->AddToken(FTextToken::Create(NSLOCTEXT("RuleRanger", "RuleSetPrefix", "RuleSet: ")));
                Message->AddToken(FUObjectToken::Create(RuleSet));
            }
            if (const auto Rule = Context->GetRule())
            {
                Message->AddToken(FTextToken::Create(NSLOCTEXT("RuleRanger", "RulePrefix", "  Rule: ")));
                Message->AddToken(FUObjectToken::Create(Rule));
            }
            Message->AddToken(FTextToken::Create(Msg));
            Log.AddMessage(Message);
        }
    };

    Add(Context->GetInfoMessages(), EMessageSeverity::Info);
    Add(Context->GetWarningMessages(), EMessageSeverity::Warning);
    Add(Context->GetErrorMessages(), EMessageSeverity::Error);
    Add(Context->GetFatalMessages(), EMessageSeverity::Error);
}
