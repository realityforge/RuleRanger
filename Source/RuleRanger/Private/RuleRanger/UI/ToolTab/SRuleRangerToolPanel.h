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
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class URuleRangerEditorSubsystem;
class URuleRangerRule;
class URuleRangerProjectRule;
class URuleRangerRuleSet;

enum class ERuleRangerToolSeverity : uint8
{
    Info,
    Warning,
    Error
};

struct FRuleRangerMessageRow
{
    TWeakObjectPtr<const UObject> Asset;
    TWeakObjectPtr<const URuleRangerRuleSet> RuleSet;
    TWeakObjectPtr<const URuleRangerProjectRule> ProjectRule;
    TWeakObjectPtr<const URuleRangerRule> Rule;
    ERuleRangerToolSeverity Severity{ ERuleRangerToolSeverity::Info };
    FText Text;
};

struct FRuleRangerRun
{
    FText Title;
    FDateTime StartedAt{ FDateTime::Now() };
    TArray<TSharedPtr<FRuleRangerMessageRow>> Messages;
};

/**
 * Shell widget for the RuleRanger Tool tab.
 */
class SRuleRangerToolPanel final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SRuleRangerToolPanel) {}
    SLATE_END_ARGS()

    /** Constructs the widget. */
    void Construct(const FArguments& InArgs);

private:
    TArray<TSharedPtr<FRuleRangerRun>> Runs;
    int32 ActiveRunIndex{ INDEX_NONE };

    TSharedPtr<SHorizontalBox> RunTabBar;
    TSharedPtr<SWidgetSwitcher> RunContentSwitcher;
    TArray<TSharedRef<SWidget>> RunPageWidgets;

    void StartRun(const FText& Title);
    void RebuildRunsUI();
    void RebuildRunContents();

    void RunContentScan(const TSharedPtr<FRuleRangerRun>& Run, bool bFix);
    void RunSelectedScan(const TSharedPtr<FRuleRangerRun>& Run, bool bFix);

    void RunAssetScan(const TSharedPtr<FRuleRangerRun>& Run, const bool bFix, TArray<FAssetData> Assets);
};
