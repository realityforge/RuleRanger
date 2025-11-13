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
#include "RuleRanger/UI/ToolTab/RuleRangerToolProjectResultHandler.h"
#include "Misc/ScopedSlowTask.h"
#include "RuleRanger/UI/ToolTab/SRuleRangerToolPanel.h"
#include "RuleRangerProjectActionContext.h"
#include "RuleRangerProjectRule.h"
#include "RuleRangerRuleSet.h"

void URuleRangerToolProjectResultHandler::Init(const TWeakPtr<FRuleRangerRun>& InRun)
{
    Run = InRun;
}
static void AppendMessages(const TSharedPtr<FRuleRangerRun>& Run,
                           const URuleRangerRuleSet* RuleSet,
                           const URuleRangerProjectRule* ProjectRule,
                           const TArray<FText>& InMessages,
                           const ERuleRangerToolSeverity Severity)
{
    for (const auto& Message : InMessages)
    {
        auto Row = MakeShared<FRuleRangerMessageRow>();
        Row->Asset = nullptr;
        Row->RuleSet = RuleSet;
        Row->Rule = nullptr;
        Row->ProjectRule = ProjectRule;
        Row->Severity = Severity;
        Row->Text = Message;
        Run->Messages.Add(Row);
    }
}

void URuleRangerToolProjectResultHandler::OnProjectRuleApplied(URuleRangerProjectActionContext* Context)
{
    const auto RunPtr = Run.Pin();
    if (RunPtr.IsValid())
    {
        const auto RuleSet = Context->GetRuleSet();
        const auto Rule = Context->GetRule();

        AppendMessages(RunPtr, RuleSet, Rule, Context->GetInfoMessages(), ERuleRangerToolSeverity::Info);
        AppendMessages(RunPtr, RuleSet, Rule, Context->GetWarningMessages(), ERuleRangerToolSeverity::Warning);
        AppendMessages(RunPtr, RuleSet, Rule, Context->GetErrorMessages(), ERuleRangerToolSeverity::Error);
        AppendMessages(RunPtr, RuleSet, Rule, Context->GetFatalMessages(), ERuleRangerToolSeverity::Error);
    }

    if (Progress)
    {
        Progress->EnterProgressFrame();
        Progress->TickProgress();
    }
}
