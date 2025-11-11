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
#include "RuleRanger/UI/ToolTab/RuleRangerToolResultHandler.h"
#include "RuleRanger/UI/ToolTab/SRuleRangerToolPanel.h"
#include "RuleRangerActionContext.h"
#include "RuleRangerRule.h"
#include "RuleRangerRuleSet.h"

void URuleRangerToolResultHandler::Init(const TWeakPtr<FRuleRangerRun>& InRun)
{
    Run = InRun;
}

static void AppendMessages(const TSharedPtr<FRuleRangerRun>& Run,
                           const UObject* Asset,
                           const URuleRangerRuleSet* RuleSet,
                           const URuleRangerRule* Rule,
                           const TArray<FText>& InMessages,
                           const ERuleRangerToolSeverity Severity)
{
    for (const auto& Message : InMessages)
    {
        auto Row = MakeShared<FRuleRangerMessageRow>();
        Row->Asset = Asset;
        Row->RuleSet = RuleSet;
        Row->Rule = Rule;
        Row->ProjectRule = nullptr;
        Row->Severity = Severity;
        Row->Text = Message;
        Run->Messages.Add(Row);
    }
}

void URuleRangerToolResultHandler::OnRuleApplied(URuleRangerActionContext* InActionContext)
{
    const auto RunPtr = Run.Pin();
    if (RunPtr.IsValid())
    {
        const auto Asset = InActionContext->GetObject();
        const auto RuleSet = InActionContext->GetRuleSet();
        const auto Rule = InActionContext->GetRule();

        AppendMessages(RunPtr, Asset, RuleSet, Rule, InActionContext->GetInfoMessages(), ERuleRangerToolSeverity::Info);
        AppendMessages(RunPtr,
                       Asset,
                       RuleSet,
                       Rule,
                       InActionContext->GetWarningMessages(),
                       ERuleRangerToolSeverity::Warning);
        AppendMessages(RunPtr,
                       Asset,
                       RuleSet,
                       Rule,
                       InActionContext->GetErrorMessages(),
                       ERuleRangerToolSeverity::Error);
        AppendMessages(RunPtr,
                       Asset,
                       RuleSet,
                       Rule,
                       InActionContext->GetFatalMessages(),
                       ERuleRangerToolSeverity::Error);
    }
}
