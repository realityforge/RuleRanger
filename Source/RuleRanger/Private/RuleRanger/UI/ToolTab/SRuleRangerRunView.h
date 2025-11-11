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
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"

struct FRuleRangerRun;
struct FRuleRangerMessageRow;

class SRuleRangerRunView final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SRuleRangerRunView) {}
    SLATE_ARGUMENT(TSharedPtr<FRuleRangerRun>, Run)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    TSharedPtr<FRuleRangerRun> Run;
    TSharedPtr<SListView<TSharedPtr<FRuleRangerMessageRow>>> ListView;
    TArray<TSharedPtr<FRuleRangerMessageRow>> FilteredItems;

    bool bShowInfo{ false };
    bool bShowWarning{ true };
    bool bShowError{ true };

    FName SortColumnId{ TEXT("Severity") };
    EColumnSortMode::Type SortMode{ EColumnSortMode::Descending };

    TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FRuleRangerMessageRow> InItem,
                                        const TSharedRef<STableViewBase>& OwnerTable) const;

    void RebuildFiltered();
    void SortFiltered();
    EColumnSortMode::Type GetColumnSortMode(const FName Column) const
    {
        return SortColumnId == Column ? SortMode : EColumnSortMode::None;
    }
    // Header sort indicator helpers
    EColumnSortMode::Type GetSeveritySortMode() const { return GetColumnSortMode(TEXT("Severity")); }
    EColumnSortMode::Type GetMessageSortMode() const { return GetColumnSortMode(TEXT("Message")); }
    EColumnSortMode::Type GetAssetSortMode() const { return GetColumnSortMode(TEXT("Asset")); }
    EColumnSortMode::Type GetRuleSortMode() const { return GetColumnSortMode(TEXT("Rule")); }
    EColumnSortMode::Type GetRuleSetSortMode() const { return GetColumnSortMode(TEXT("RuleSet")); }
    void
    OnColumnSortModeChanged(const EColumnSortPriority::Type, const FName& Column, const EColumnSortMode::Type NewMode)
    {
        SortColumnId = Column;
        SortMode = NewMode;
        SortFiltered();
    }
    ECheckBoxState GetInfoState() const { return bShowInfo ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }
    ECheckBoxState GetWarningState() const
    {
        return bShowWarning ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
    }
    ECheckBoxState GetErrorState() const { return bShowError ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }

    void OnToggleInfo(const ECheckBoxState State)
    {
        bShowInfo = ECheckBoxState::Checked == State;
        RebuildFiltered();
    }
    void OnToggleWarning(const ECheckBoxState State)
    {
        bShowWarning = ECheckBoxState::Checked == State;
        RebuildFiltered();
    }
    void OnToggleError(const ECheckBoxState State)
    {
        bShowError = ECheckBoxState::Checked == State;
        RebuildFiltered();
    }

    // Context menu support
    TSharedPtr<SWidget> OnContextMenuOpening();
    void ExecuteOpenSelected() const;
    void ExecuteShowInContentBrowser() const;
    void ExecuteCopyMessage() const;
    void ExecuteOpenRule() const;
    void ExecuteOpenRuleSet() const;
};
