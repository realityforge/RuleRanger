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

#include "RuleRanger/UI/ToolTab/RuleRangerToolTab.h"
#include "Framework/Docking/TabManager.h"
#include "RuleRanger/UI/RuleRangerStyle.h"
#include "RuleRanger/UI/ToolTab/SRuleRangerToolPanel.h"

static const FName RuleRangerToolTabName("RuleRangerTool");

FName FRuleRangerToolTab::GetTabName()
{
    return RuleRangerToolTabName;
}

void FRuleRangerToolTab::Initialize()
{
    FGlobalTabmanager::Get()
        ->RegisterNomadTabSpawner(GetTabName(), FOnSpawnTab::CreateStatic([](const auto&) {
                                      return SNew(SDockTab).TabRole(NomadTab)[SNew(SRuleRangerToolPanel)];
                                  }))
        .SetDisplayName(NSLOCTEXT("RuleRanger", "RuleRangerTool_DisplayName", "Rule Ranger"))
        .SetTooltipText(NSLOCTEXT("RuleRanger", "RuleRangerTool_Tooltip", "Rule Ranger Tool window"))
        .SetIcon(FRuleRangerStyle::GetScanIcon())
        .SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FRuleRangerToolTab::Shutdown()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GetTabName());
}
