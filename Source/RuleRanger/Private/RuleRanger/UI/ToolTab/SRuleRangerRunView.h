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
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SCompoundWidget.h"

struct FRuleRangerRun;
struct FRuleRangerMessageRow;

class SHeaderRow;
class ITableRow;
template <typename ItemType>
class SListView;

class SRuleRangerRunView final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SRuleRangerRunView) {}
    SLATE_ARGUMENT(TSharedPtr<FRuleRangerRun>, Run)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // Preferences (persisted between sessions)
    void LoadPreferences();
    void SavePreferences() const;

    TSharedPtr<FRuleRangerRun> Run;
    TSharedPtr<SListView<TSharedPtr<FRuleRangerMessageRow>>> ListView;
    TSharedPtr<SHeaderRow> Header;
    TArray<TSharedPtr<FRuleRangerMessageRow>> FilteredItems;

    bool bShowInfo{ false };
    bool bShowWarning{ true };
    bool bShowError{ true };

    // Quick column visibility
    bool bShowAssetColumn{ true };
    bool bShowRuleColumn{ true };
    bool bShowRuleSetColumn{ true };

    // Text search filter (case-insensitive substring)
    FString SearchQuery;

    FName SortColumnId{ TEXT("Severity") };
    EColumnSortMode::Type SortMode{ EColumnSortMode::Descending };

    TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FRuleRangerMessageRow> InItem,
                                        const TSharedRef<STableViewBase>& OwnerTable) const;

    void RebuildFiltered();
    void SortFiltered();
    void OnSearchChanged(const FText& NewText)
    {
        SearchQuery = NewText.ToString();
        RebuildFiltered();
    }
    TSharedRef<SWidget> BuildColumnsMenu();
    void ToggleAssetColumn(ECheckBoxState State);
    void ToggleRuleColumn(ECheckBoxState State);
    void ToggleRuleSetColumn(ECheckBoxState State);
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
        SavePreferences();
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
        SavePreferences();
    }
    void OnToggleWarning(const ECheckBoxState State)
    {
        bShowWarning = ECheckBoxState::Checked == State;
        RebuildFiltered();
        SavePreferences();
    }
    void OnToggleError(const ECheckBoxState State)
    {
        bShowError = ECheckBoxState::Checked == State;
        RebuildFiltered();
        SavePreferences();
    }

    // Context menu support
    TSharedPtr<SWidget> OnContextMenuOpening();
    void ExecuteOpenSelected() const;
    void ExecuteShowInContentBrowser() const;
    void ExecuteCopyMessage() const;
    void ExecuteOpenRule() const;
    void ExecuteOpenRuleSet() const;
};
