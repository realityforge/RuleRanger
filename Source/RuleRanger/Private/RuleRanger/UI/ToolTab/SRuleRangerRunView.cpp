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
#include "RuleRanger/UI/ToolTab/SRuleRangerRunView.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformMisc.h"
#include "IContentBrowserSingleton.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "RuleRanger/UI/ToolTab/SRuleRangerRunRow.h"
#include "RuleRanger/UI/ToolTab/SRuleRangerToolPanel.h"
// ReSharper disable 3 CppUnusedIncludeDirective
#include "RuleRanger/UI/RuleRangerStyle.h"
#include "RuleRanger/UI/RuleRangerUIHelpers.h"
#include "RuleRangerProjectRule.h"
#include "RuleRangerRule.h"
#include "RuleRangerRuleSet.h"
#include "Styling/AppStyle.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"

void SRuleRangerRunView::Construct(const FArguments& InArgs)
{
    Run = InArgs._Run;

    // Load persisted preferences before building the view
    LoadPreferences();
    RebuildFiltered();

    ListView = SNew(SListView<TSharedPtr<FRuleRangerMessageRow>>)
                   .ListItemsSource(&FilteredItems)
                   .OnGenerateRow(this, &SRuleRangerRunView::OnGenerateRow)
                   .SelectionMode(ESelectionMode::Multi)
                   .OnContextMenuOpening(this, &SRuleRangerRunView::OnContextMenuOpening)
                   .HeaderRow(SAssignNew(Header, SHeaderRow)
                              + SHeaderRow::Column("Severity")
                                    .DefaultLabel(NSLOCTEXT("RuleRanger", "ColSeverity", ""))
                                    .FixedWidth(28.f)
                                    .SortMode(this, &SRuleRangerRunView::GetSeveritySortMode)
                                    .OnSort(this, &SRuleRangerRunView::OnColumnSortModeChanged)
                              + SHeaderRow::Column("Asset")
                                    .DefaultLabel(NSLOCTEXT("RuleRanger", "ColAsset", "Asset"))
                                    .FixedWidth(180.f)
                                    .SortMode(this, &SRuleRangerRunView::GetAssetSortMode)
                                    .OnSort(this, &SRuleRangerRunView::OnColumnSortModeChanged)
                              + SHeaderRow::Column("Message")
                                    .DefaultLabel(NSLOCTEXT("RuleRanger", "ColMessage", "Message"))
                                    .SortMode(this, &SRuleRangerRunView::GetMessageSortMode)
                                    .OnSort(this, &SRuleRangerRunView::OnColumnSortModeChanged)
                              + SHeaderRow::Column("RuleSet")
                                    .DefaultLabel(NSLOCTEXT("RuleRanger", "ColRuleSet", "Rule Set"))
                                    .FixedWidth(180.f)
                                    .SortMode(this, &SRuleRangerRunView::GetRuleSetSortMode)
                                    .OnSort(this, &SRuleRangerRunView::OnColumnSortModeChanged)
                              + SHeaderRow::Column("Rule")
                                    .DefaultLabel(NSLOCTEXT("RuleRanger", "ColRule", "Rule"))
                                    .FixedWidth(180.f)
                                    .SortMode(this, &SRuleRangerRunView::GetRuleSortMode)
                                    .OnSort(this, &SRuleRangerRunView::OnColumnSortModeChanged));

    // Apply initial column visibility
    if (Header.IsValid())
    {
        Header->SetShowGeneratedColumn(TEXT("Asset"), bShowAssetColumn);
        Header->SetShowGeneratedColumn(TEXT("Rule"), bShowRuleColumn);
        Header->SetShowGeneratedColumn(TEXT("RuleSet"), bShowRuleSetColumn);
    }

    ChildSlot[SNew(SBorder).Padding(FMargin(
        4.f))[SNew(SVerticalBox)
              + SVerticalBox::Slot()
                    .Padding(FMargin(12, 2, 12, 2))
                    .AutoHeight()
                        [SNew(SHorizontalBox)
                         + SHorizontalBox::Slot()
                               .AutoWidth()
                               .VAlign(VAlign_Center)
                               .Padding(FMargin(0, 0, 12, 0))
                                   [SNew(STextBlock).Text(NSLOCTEXT("RuleRanger", "FilterLabel", "Show: "))]
                         + SHorizontalBox::Slot()
                               .AutoWidth()
                               .VAlign(VAlign_Center)
                               .Padding(FMargin(0, 0, 12, 0))
                                   [SNew(SCheckBox)
                                        .IsChecked(this, &SRuleRangerRunView::GetErrorState)
                                        .OnCheckStateChanged(this, &SRuleRangerRunView::OnToggleError)
                                            [RuleRangerUI::MakeIconLabelFromBrush(
                                                FRuleRangerStyle::GetErrorMessageBrush(),
                                                NSLOCTEXT("RuleRanger", "FilterError", "Errors"))]]
                         + SHorizontalBox::Slot()
                               .AutoWidth()
                               .VAlign(VAlign_Center)
                               .Padding(FMargin(0, 0, 12, 0))
                                   [SNew(SCheckBox)
                                        .IsChecked(this, &SRuleRangerRunView::GetWarningState)
                                        .OnCheckStateChanged(this, &SRuleRangerRunView::OnToggleWarning)
                                            [RuleRangerUI::MakeIconLabelFromBrush(
                                                FRuleRangerStyle::GetWarningMessageBrush(),
                                                NSLOCTEXT("RuleRanger", "FilterWarning", "Warnings"))]]
                         + SHorizontalBox::Slot().AutoWidth().VAlign(
                             VAlign_Center)[SNew(SCheckBox)
                                                .IsChecked(this, &SRuleRangerRunView::GetInfoState)
                                                .OnCheckStateChanged(this, &SRuleRangerRunView::OnToggleInfo)
                                                    [RuleRangerUI::MakeIconLabelFromBrush(
                                                        FRuleRangerStyle::GetNoteMessageBrush(),
                                                        NSLOCTEXT("RuleRanger", "FilterInfo", "Info"))]]
                         + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                               [SNew(SSearchBox)
                                    .HintText(
                                        NSLOCTEXT("RuleRanger", "SearchHint", "Search message, asset, rule, ruleset"))
                                    .OnTextChanged(this, &SRuleRangerRunView::OnSearchChanged)]
                         + SHorizontalBox::Slot()
                               .AutoWidth()
                               .VAlign(VAlign_Center)
                               .Padding(FMargin(8.f, 0.f, 0.f, 0.f))
                                   [SNew(SComboButton)
                                        .ToolTipText(
                                            NSLOCTEXT("RuleRanger", "ColumnsTooltip", "Toggle visible columns"))
                                        .HasDownArrow(true)
                                        .OnGetMenuContent(this, &SRuleRangerRunView::BuildColumnsMenu)
                                        .ButtonContent()[SNew(STextBlock)
                                                             .Text(NSLOCTEXT("RuleRanger", "Columns", "Columns"))]]]
              + SVerticalBox::Slot().FillHeight(1.f).Padding(FMargin(0, 4, 0, 0))[ListView.ToSharedRef()]]];
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppPassValueParameterByConstReference
TSharedRef<ITableRow> SRuleRangerRunView::OnGenerateRow(TSharedPtr<FRuleRangerMessageRow> InItem,
                                                        const TSharedRef<STableViewBase>& OwnerTable) const
{
    return SNew(SRuleRangerRunRow, OwnerTable).Item(InItem).ListView(ListView);
}

void SRuleRangerRunView::RebuildFiltered()
{
    FilteredItems.Reset();
    if (Run.IsValid())
    {
        const bool bHasQuery = !SearchQuery.IsEmpty();
        for (const auto& Message : Run->Messages)
        {
            if (ERuleRangerToolSeverity::Error == Message->Severity && bShowError
                || ERuleRangerToolSeverity::Warning == Message->Severity && bShowWarning
                || ERuleRangerToolSeverity::Info == Message->Severity && bShowInfo)
            {
                bool bMatches = true;
                if (bHasQuery)
                {
                    bMatches = false;
                    const FString Q = SearchQuery;
                    const FString Msg = Message->Text.ToString();
                    const FString AssetName = Message->Asset.IsValid() ? Message->Asset->GetName() : FString();
                    const FString RuleName = Message->Rule.IsValid()
                        ? Message->Rule->GetName()
                        : (Message->ProjectRule.IsValid() ? Message->ProjectRule->GetName() : FString());
                    const FString RuleSetName = Message->RuleSet.IsValid() ? Message->RuleSet->GetName() : FString();

                    if (Msg.Contains(Q, ESearchCase::IgnoreCase) || AssetName.Contains(Q, ESearchCase::IgnoreCase)
                        || RuleName.Contains(Q, ESearchCase::IgnoreCase)
                        || RuleSetName.Contains(Q, ESearchCase::IgnoreCase))
                    {
                        bMatches = true;
                    }
                }

                if (bMatches)
                {
                    FilteredItems.Add(Message);
                }
            }
        }
        SortFiltered();
    }
}

void SRuleRangerRunView::SortFiltered()
{
    auto CmpText = [](const auto& Left, const auto& Right) {
        return FCString::Stricmp(*Left.ToString(), *Right.ToString()) < 0;
    };

    FilteredItems.Sort([&](const auto& Left, const auto& Right) {
        const bool bAsc = EColumnSortMode::Descending != SortMode;
        const auto SevRank = [](const ERuleRangerToolSeverity S) {
            switch (S)
            {
                case ERuleRangerToolSeverity::Error:
                    return 2;
                case ERuleRangerToolSeverity::Warning:
                    return 1;
                default:
                    return 0;
            }
        };

        auto Less = [&](const bool Cond) { return bAsc ? Cond : !Cond; };

        if (TEXT("Severity") == SortColumnId)
        {
            return Less(SevRank(Left->Severity) < SevRank(Right->Severity));
        }
        else if (TEXT("Message") == SortColumnId)
        {
            return Less(CmpText(Left->Text, Right->Text));
        }
        else if (TEXT("Asset") == SortColumnId)
        {
            const auto LeftName = Left->Asset.IsValid() ? Left->Asset->GetName() : FString("~");
            const auto RightName = Right->Asset.IsValid() ? Right->Asset->GetName() : FString("~");
            return Less(FCString::Stricmp(*LeftName, *RightName) < 0);
        }
        else if (TEXT("Rule") == SortColumnId)
        {
            const auto LeftName = Left->Rule.IsValid() ? Left->Rule->GetName()
                : Left->ProjectRule.IsValid()          ? Left->ProjectRule->GetName()
                                                       : FString("~");
            const auto RightName = Right->Rule.IsValid() ? Right->Rule->GetName()
                : Right->ProjectRule.IsValid()           ? Right->ProjectRule->GetName()
                                                         : FString("~");
            return Less(FCString::Stricmp(*LeftName, *RightName) < 0);
        }
        else if (TEXT("RuleSet") == SortColumnId)
        {
            const auto LeftName = Left->RuleSet.IsValid() ? Left->RuleSet->GetName() : FString("~");
            const auto RightName = Right->RuleSet.IsValid() ? Right->RuleSet->GetName() : FString("~");
            return Less(FCString::Stricmp(*LeftName, *RightName) < 0);
        }
        else
        {
            return Less(SevRank(Left->Severity) < SevRank(Right->Severity));
        }
    });

    if (ListView.IsValid())
    {
        ListView->RequestListRefresh();
    }
}

void SRuleRangerRunView::LoadPreferences()
{
    static const TCHAR* Section = TEXT("RuleRanger.Tool.RunView");

    auto bValue{ false };
    if (GConfig->GetBool(Section, TEXT("ShowInfo"), bValue, GEditorPerProjectIni))
    {
        bShowInfo = bValue;
    }
    if (GConfig->GetBool(Section, TEXT("ShowWarning"), bValue, GEditorPerProjectIni))
    {
        bShowWarning = bValue;
    }
    if (GConfig->GetBool(Section, TEXT("ShowError"), bValue, GEditorPerProjectIni))
    {
        bShowError = bValue;
    }

    if (GConfig->GetBool(Section, TEXT("ShowAssetColumn"), bValue, GEditorPerProjectIni))
    {
        bShowAssetColumn = bValue;
    }
    if (GConfig->GetBool(Section, TEXT("ShowRuleColumn"), bValue, GEditorPerProjectIni))
    {
        bShowRuleColumn = bValue;
    }
    if (GConfig->GetBool(Section, TEXT("ShowRuleSetColumn"), bValue, GEditorPerProjectIni))
    {
        bShowRuleSetColumn = bValue;
    }

    FString SortId;
    if (GConfig->GetString(Section, TEXT("SortColumnId"), SortId, GEditorPerProjectIni) && !SortId.IsEmpty())
    {
        SortColumnId = FName(*SortId);
    }
    int32 SortModeInt = static_cast<int32>(SortMode);
    if (GConfig->GetInt(Section, TEXT("SortMode"), SortModeInt, GEditorPerProjectIni))
    {
        SortMode = static_cast<EColumnSortMode::Type>(SortModeInt);
        // Validate against expected values
        if (SortMode != EColumnSortMode::Ascending && SortMode != EColumnSortMode::Descending)
        {
            SortMode = EColumnSortMode::Descending;
        }
    }
}

void SRuleRangerRunView::SavePreferences() const
{
    static const TCHAR* Section = TEXT("RuleRanger.Tool.RunView");

    GConfig->SetBool(Section, TEXT("ShowInfo"), bShowInfo, GEditorPerProjectIni);
    GConfig->SetBool(Section, TEXT("ShowWarning"), bShowWarning, GEditorPerProjectIni);
    GConfig->SetBool(Section, TEXT("ShowError"), bShowError, GEditorPerProjectIni);
    GConfig->SetBool(Section, TEXT("ShowAssetColumn"), bShowAssetColumn, GEditorPerProjectIni);
    GConfig->SetBool(Section, TEXT("ShowRuleColumn"), bShowRuleColumn, GEditorPerProjectIni);
    GConfig->SetBool(Section, TEXT("ShowRuleSetColumn"), bShowRuleSetColumn, GEditorPerProjectIni);
    GConfig->SetString(Section, TEXT("SortColumnId"), *SortColumnId.ToString(), GEditorPerProjectIni);
    GConfig->SetInt(Section, TEXT("SortMode"), SortMode, GEditorPerProjectIni);
    GConfig->Flush(false, GEditorPerProjectIni);
}

TSharedPtr<SWidget> SRuleRangerRunView::OnContextMenuOpening()
{
    if (ListView.IsValid())
    {
        auto Selected = ListView->GetSelectedItems();
        const bool HasAnySelection = Selected.Num() > 0;

        bool bHasAsset = false;
        bool bHasRule = false;
        bool bHasRuleSet = false;
        for (const auto& Row : Selected)
        {
            if (Row.IsValid())
            {
                if (Row->Asset.IsValid())
                {
                    bHasAsset = true;
                }
                if (Row->Rule.IsValid() || Row->ProjectRule.IsValid())
                {
                    bHasRule = true;
                }
                if (Row->RuleSet.IsValid())
                {
                    bHasRuleSet = true;
                }
                if (bHasAsset && bHasRule && bHasRuleSet)
                {
                    break;
                }
            }
        }

        const bool bHasAnyAssetLike = bHasAsset;

        FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection*/ true, /*MenuExtender*/ nullptr);

        MenuBuilder.BeginSection("RuleRanger.RowActions", NSLOCTEXT("RuleRanger", "RRRowActions", "Actions"));
        {
            RuleRangerUI::AddMenuEntry(
                MenuBuilder,
                NSLOCTEXT("RuleRanger", "OpenSelected", "Edit Asset…"),
                NSLOCTEXT("RuleRanger", "OpenSelected_Tooltip", "Open the selected asset(s) in the editor"),
                FRuleRangerStyle::GetEditAssetIcon(),
                FExecuteAction::CreateSP(this, &SRuleRangerRunView::ExecuteOpenSelected),
                FCanExecuteAction::CreateLambda([HasAnySelection, bHasAsset] { return HasAnySelection && bHasAsset; }));

            RuleRangerUI::AddMenuEntry(
                MenuBuilder,
                NSLOCTEXT("RuleRanger", "OpenRuleSet", "Edit RuleSet…"),
                NSLOCTEXT("RuleRanger", "OpenRuleSet_Tooltip", "Open the selected ruleset(s) in the editor"),
                FRuleRangerStyle::GetEditAssetIcon(),
                FExecuteAction::CreateSP(this, &SRuleRangerRunView::ExecuteOpenRuleSet),
                FCanExecuteAction::CreateLambda(
                    [HasAnySelection, bHasRuleSet] { return HasAnySelection && bHasRuleSet; }));

            RuleRangerUI::AddMenuEntry(
                MenuBuilder,
                NSLOCTEXT("RuleRanger", "OpenRule", "Edit Rule…"),
                NSLOCTEXT("RuleRanger", "OpenRule_Tooltip", "Open the selected rule(s) in the editor"),
                FRuleRangerStyle::GetEditAssetIcon(),
                FExecuteAction::CreateSP(this, &SRuleRangerRunView::ExecuteOpenRule),
                FCanExecuteAction::CreateLambda([HasAnySelection, bHasRule] { return HasAnySelection && bHasRule; }));

            RuleRangerUI::AddMenuEntry(
                MenuBuilder,
                NSLOCTEXT("RuleRanger", "ShowInCB", "Show Asset in Content Browser"),
                NSLOCTEXT("RuleRanger", "ShowInCB_Tooltip", "Show the selected assets(s) in the Content Browser"),
                FRuleRangerStyle::GetFindInContentBrowserIcon(),
                FExecuteAction::CreateSP(this, &SRuleRangerRunView::ExecuteShowInContentBrowser),
                FCanExecuteAction::CreateLambda(
                    [HasAnySelection, bHasAnyAssetLike] { return HasAnySelection && bHasAnyAssetLike; }));

            RuleRangerUI::AddMenuEntry(
                MenuBuilder,
                NSLOCTEXT("RuleRanger", "CopyMessage", "Copy Message"),
                NSLOCTEXT("RuleRanger", "CopyMessage_Tooltip", "Copy the selected message text to the clipboard"),
                FRuleRangerStyle::GetCopyMessageIcon(),
                FExecuteAction::CreateSP(this, &SRuleRangerRunView::ExecuteCopyMessage),
                FCanExecuteAction::CreateLambda([HasAnySelection] { return HasAnySelection; }));
        }
        MenuBuilder.EndSection();

        return MenuBuilder.MakeWidget();
    }
    else
    {
        return nullptr;
    }
}

void SRuleRangerRunView::ExecuteOpenSelected() const
{
    if (ListView.IsValid())
    {
        TArray<UObject*> ToOpen;
        for (const auto& Row : ListView->GetSelectedItems())
        {
            if (Row.IsValid())
            {
                if (const auto Object = Row->Asset.IsValid() ? const_cast<UObject*>(Row->Asset.Get()) : nullptr)
                {
                    ToOpen.Add(Object);
                }
            }
        }
        if (ToOpen.Num() > 0)
        {
            if (auto* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
            {
                Subsystem->OpenEditorForAssets(ToOpen);
            }
        }
    }
}

void SRuleRangerRunView::ExecuteShowInContentBrowser() const
{
    if (ListView.IsValid())
    {
        TArray<FAssetData> Assets;
        Assets.Reserve(ListView->GetSelectedItems().Num());
        for (const auto& Row : ListView->GetSelectedItems())
        {
            if (Row.IsValid())
            {
                if (const auto Object = Row->Asset.IsValid() ? Row->Asset.Get() : nullptr)
                {
                    Assets.Emplace(const_cast<UObject*>(Object));
                }
            }
        }
        if (Assets.Num() > 0)
        {
            const auto& Module = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
            Module.Get().SyncBrowserToAssets(Assets);
        }
    }
}

void SRuleRangerRunView::ExecuteCopyMessage() const
{
    if (ListView.IsValid())
    {
        FString Message;
        bool bFirst = true;
        for (const auto& Row : ListView->GetSelectedItems())
        {
            if (Row.IsValid())
            {
                if (!bFirst)
                {
                    Message.AppendChar(TEXT('\n'));
                }
                bFirst = false;
                Message.Append(Row->Text.ToString());
            }
        }
        if (!Message.IsEmpty())
        {
            FPlatformApplicationMisc::ClipboardCopy(*Message);
        }
    }
}

void SRuleRangerRunView::ExecuteOpenRule() const
{
    if (ListView.IsValid())
    {
        TArray<UObject*> ToOpen;
        for (const auto& Row : ListView->GetSelectedItems())
        {
            if (Row.IsValid())
            {
                if (Row->Rule.IsValid())
                {
                    ToOpen.Add(const_cast<URuleRangerRule*>(Row->Rule.Get()));
                }
                else if (Row->ProjectRule.IsValid())
                {
                    ToOpen.Add(const_cast<URuleRangerProjectRule*>(Row->ProjectRule.Get()));
                }
            }
        }
        if (ToOpen.Num() > 0)
        {
            if (const auto Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
            {
                Subsystem->OpenEditorForAssets(ToOpen);
            }
        }
    }
}

void SRuleRangerRunView::ExecuteOpenRuleSet() const
{
    if (ListView.IsValid())
    {
        TArray<UObject*> ToOpen;
        for (const auto& Row : ListView->GetSelectedItems())
        {
            if (Row.IsValid() && Row->RuleSet.IsValid())
            {
                ToOpen.Add(const_cast<URuleRangerRuleSet*>(Row->RuleSet.Get()));
            }
        }
        if (ToOpen.Num() > 0)
        {
            if (const auto Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
            {
                Subsystem->OpenEditorForAssets(ToOpen);
            }
        }
    }
}
TSharedRef<SWidget> SRuleRangerRunView::BuildColumnsMenu()
{
    FMenuBuilder MenuBuilder(true, nullptr);
    MenuBuilder.BeginSection("RuleRanger.Columns", NSLOCTEXT("RuleRanger", "ColumnsMenu", "Columns"));
    {
        MenuBuilder.AddWidget(SNew(SCheckBox)
                                  .Padding(FMargin(6, 2, 2, 2))
                                  .IsChecked(bShowAssetColumn ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
                                  .OnCheckStateChanged(this, &SRuleRangerRunView::ToggleAssetColumn)
                                      [SNew(STextBlock).Text(NSLOCTEXT("RuleRanger", "ColAsset", "Asset"))],
                              FText::GetEmpty());
        MenuBuilder.AddWidget(SNew(SCheckBox)
                                  .Padding(FMargin(6, 2, 2, 2))
                                  .IsChecked(bShowRuleColumn ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
                                  .OnCheckStateChanged(this, &SRuleRangerRunView::ToggleRuleColumn)
                                      [SNew(STextBlock).Text(NSLOCTEXT("RuleRanger", "ColRule", "Rule"))],
                              FText::GetEmpty());
        MenuBuilder.AddWidget(SNew(SCheckBox)
                                  .Padding(FMargin(6, 2, 2, 2))
                                  .IsChecked(bShowRuleSetColumn ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
                                  .OnCheckStateChanged(this, &SRuleRangerRunView::ToggleRuleSetColumn)
                                      [SNew(STextBlock).Text(NSLOCTEXT("RuleRanger", "ColRuleSet", "Rule Set"))],
                              FText::GetEmpty());
    }
    MenuBuilder.EndSection();
    return MenuBuilder.MakeWidget();
}

void SRuleRangerRunView::ToggleAssetColumn(const ECheckBoxState State)
{
    bShowAssetColumn = (State == ECheckBoxState::Checked);
    if (Header.IsValid())
    {
        Header->SetShowGeneratedColumn(TEXT("Asset"), bShowAssetColumn);
    }
    SavePreferences();
}

void SRuleRangerRunView::ToggleRuleColumn(const ECheckBoxState State)
{
    bShowRuleColumn = (State == ECheckBoxState::Checked);
    if (Header.IsValid())
    {
        Header->SetShowGeneratedColumn(TEXT("Rule"), bShowRuleColumn);
    }
    SavePreferences();
}

void SRuleRangerRunView::ToggleRuleSetColumn(const ECheckBoxState State)
{
    bShowRuleSetColumn = (State == ECheckBoxState::Checked);
    if (Header.IsValid())
    {
        Header->SetShowGeneratedColumn(TEXT("RuleSet"), bShowRuleSetColumn);
    }
    SavePreferences();
}
