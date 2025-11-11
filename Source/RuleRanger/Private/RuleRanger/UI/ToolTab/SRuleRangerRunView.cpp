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
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "RuleRanger/UI/ToolTab/SRuleRangerRunRow.h"
#include "RuleRanger/UI/ToolTab/SRuleRangerToolPanel.h"
// ReSharper disable 3 CppUnusedIncludeDirective
#include "RuleRanger/UI/RuleRangerStyle.h"
#include "RuleRangerProjectRule.h"
#include "RuleRangerRule.h"
#include "RuleRangerRuleSet.h"
#include "Styling/AppStyle.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"

void SRuleRangerRunView::Construct(const FArguments& InArgs)
{
    Run = InArgs._Run;

    RebuildFiltered();

    ListView = SNew(SListView<TSharedPtr<FRuleRangerMessageRow>>)
                   .ListItemsSource(&FilteredItems)
                   .OnGenerateRow(this, &SRuleRangerRunView::OnGenerateRow)
                   .SelectionMode(ESelectionMode::Multi)
                   .OnContextMenuOpening(this, &SRuleRangerRunView::OnContextMenuOpening)
                   .HeaderRow(SNew(SHeaderRow)
                              + SHeaderRow::Column("Severity")
                                    .DefaultLabel(NSLOCTEXT("RuleRanger", "ColSeverity", ""))
                                    .FixedWidth(28.f)
                                    .SortMode(this, &SRuleRangerRunView::GetSeveritySortMode)
                                    .OnSort(this, &SRuleRangerRunView::OnColumnSortModeChanged)
                              + SHeaderRow::Column("Message")
                                    .DefaultLabel(NSLOCTEXT("RuleRanger", "ColMessage", "Message"))
                                    .SortMode(this, &SRuleRangerRunView::GetMessageSortMode)
                                    .OnSort(this, &SRuleRangerRunView::OnColumnSortModeChanged)
                              + SHeaderRow::Column("Asset")
                                    .DefaultLabel(NSLOCTEXT("RuleRanger", "ColAsset", "Asset"))
                                    .FixedWidth(180.f)
                                    .SortMode(this, &SRuleRangerRunView::GetAssetSortMode)
                                    .OnSort(this, &SRuleRangerRunView::OnColumnSortModeChanged)
                              + SHeaderRow::Column("Rule")
                                    .DefaultLabel(NSLOCTEXT("RuleRanger", "ColRule", "Rule"))
                                    .FixedWidth(180.f)
                                    .SortMode(this, &SRuleRangerRunView::GetRuleSortMode)
                                    .OnSort(this, &SRuleRangerRunView::OnColumnSortModeChanged)
                              + SHeaderRow::Column("RuleSet")
                                    .DefaultLabel(NSLOCTEXT("RuleRanger", "ColRuleSet", "RuleSet"))
                                    .FixedWidth(180.f)
                                    .SortMode(this, &SRuleRangerRunView::GetRuleSetSortMode)
                                    .OnSort(this, &SRuleRangerRunView::OnColumnSortModeChanged));

    ChildSlot[SNew(SBorder).Padding(FMargin(
        4.f))[SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()
                    [SNew(SHorizontalBox)
                     + SHorizontalBox::Slot()
                           .AutoWidth()
                           .VAlign(VAlign_Center)
                           .Padding(FMargin(0, 0, 12, 0))[SNew(STextBlock)
                                                              .Text(NSLOCTEXT("RuleRanger", "FilterLabel", "Show: "))]
                     + SHorizontalBox::Slot()
                           .AutoWidth()
                           .VAlign(VAlign_Center)
                           .Padding(FMargin(0, 0, 12, 0))
                               [SNew(SCheckBox)
                                    .IsChecked(this, &SRuleRangerRunView::GetErrorState)
                                    .OnCheckStateChanged(this, &SRuleRangerRunView::OnToggleError)
                                        [SNew(SHorizontalBox)
                                         + SHorizontalBox::Slot()
                                               .AutoWidth()
                                               .VAlign(VAlign_Center)
                                               .Padding(FMargin(0, 2, 6, 2))[SNew(SImage).Image(
                                                   FRuleRangerStyle::GetErrorMessageBrush())]
                                         + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                               [SNew(STextBlock)
                                                    .Text(NSLOCTEXT("RuleRanger", "FilterError", "Errors"))]]]
                     + SHorizontalBox::Slot()
                           .AutoWidth()
                           .VAlign(VAlign_Center)
                           .Padding(FMargin(0, 0, 12, 0))
                               [SNew(SCheckBox)
                                    .IsChecked(this, &SRuleRangerRunView::GetWarningState)
                                    .OnCheckStateChanged(this, &SRuleRangerRunView::OnToggleWarning)
                                        [SNew(SHorizontalBox)
                                         + SHorizontalBox::Slot()
                                               .AutoWidth()
                                               .VAlign(VAlign_Center)
                                               .Padding(FMargin(0, 2, 6, 2))[SNew(SImage).Image(
                                                   FRuleRangerStyle::GetWarningMessageBrush())]
                                         + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                               [SNew(STextBlock)
                                                    .Text(NSLOCTEXT("RuleRanger", "FilterWarning", "Warnings"))]]]
                     + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                           [SNew(SCheckBox)
                                .IsChecked(this, &SRuleRangerRunView::GetInfoState)
                                .OnCheckStateChanged(this, &SRuleRangerRunView::OnToggleInfo)
                                    [SNew(SHorizontalBox)
                                     + SHorizontalBox::Slot()
                                           .AutoWidth()
                                           .VAlign(VAlign_Center)
                                           .Padding(FMargin(0, 2, 6, 2))[SNew(SImage).Image(
                                               FRuleRangerStyle::GetNoteMessageBrush())]
                                     + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                           [SNew(STextBlock).Text(NSLOCTEXT("RuleRanger", "FilterInfo", "Info"))]]]]
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
        for (const auto& Message : Run->Messages)
        {
            if (ERuleRangerToolSeverity::Error == Message->Severity && bShowError
                || ERuleRangerToolSeverity::Warning == Message->Severity && bShowWarning
                || ERuleRangerToolSeverity::Info == Message->Severity && bShowInfo)
            {
                FilteredItems.Add(Message);
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
            const auto LeftName = Left->Rule.IsValid() ? Left->Rule->GetName() : FString("~");
            const auto RightName = Right->Rule.IsValid() ? Right->Rule->GetName() : FString("~");
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
            MenuBuilder.AddMenuEntry(
                NSLOCTEXT("RuleRanger", "OpenSelected", "Edit Asset…"),
                NSLOCTEXT("RuleRanger", "OpenSelected_Tooltip", "Open the selected asset(s) in the editor"),
                FRuleRangerStyle::GetEditAssetIcon(),
                FUIAction(FExecuteAction::CreateSP(this, &SRuleRangerRunView::ExecuteOpenSelected),
                          FCanExecuteAction::CreateLambda(
                              [HasAnySelection, bHasAsset] { return HasAnySelection && bHasAsset; })));

            MenuBuilder.AddMenuEntry(
                NSLOCTEXT("RuleRanger", "OpenRuleSet", "Edit RuleSet…"),
                NSLOCTEXT("RuleRanger", "OpenRuleSet_Tooltip", "Open the selected ruleset(s) in the editor"),
                FRuleRangerStyle::GetEditAssetIcon(),
                FUIAction(FExecuteAction::CreateSP(this, &SRuleRangerRunView::ExecuteOpenRuleSet),
                          FCanExecuteAction::CreateLambda(
                              [HasAnySelection, bHasRuleSet] { return HasAnySelection && bHasRuleSet; })));

            MenuBuilder.AddMenuEntry(
                NSLOCTEXT("RuleRanger", "OpenRule", "Edit Rule…"),
                NSLOCTEXT("RuleRanger", "OpenRule_Tooltip", "Open the selected rule(s) in the editor"),
                FRuleRangerStyle::GetEditAssetIcon(),
                FUIAction(FExecuteAction::CreateSP(this, &SRuleRangerRunView::ExecuteOpenRule),
                          FCanExecuteAction::CreateLambda(
                              [HasAnySelection, bHasRule] { return HasAnySelection && bHasRule; })));

            MenuBuilder.AddMenuEntry(
                NSLOCTEXT("RuleRanger", "ShowInCB", "Show Asset in Content Browser"),
                NSLOCTEXT("RuleRanger", "ShowInCB_Tooltip", "Show the selected assets(s) in the Content Browser"),
                FRuleRangerStyle::GetFindInContentBrowserIcon(),
                FUIAction(FExecuteAction::CreateSP(this, &SRuleRangerRunView::ExecuteShowInContentBrowser),
                          FCanExecuteAction::CreateLambda(
                              [HasAnySelection, bHasAnyAssetLike] { return HasAnySelection && bHasAnyAssetLike; })));

            MenuBuilder.AddMenuEntry(
                NSLOCTEXT("RuleRanger", "CopyMessage", "Copy Message"),
                NSLOCTEXT("RuleRanger", "CopyMessage_Tooltip", "Copy the selected message text to the clipboard"),
                FRuleRangerStyle::GetCopyMessageIcon(),
                FUIAction(FExecuteAction::CreateSP(this, &SRuleRangerRunView::ExecuteCopyMessage),
                          FCanExecuteAction::CreateLambda([HasAnySelection] { return HasAnySelection; })));
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
    if (!ListView.IsValid())
    {
        return;
    }
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
        if (auto* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
        {
            Subsystem->OpenEditorForAssets(ToOpen);
        }
    }
}

void SRuleRangerRunView::ExecuteOpenRuleSet() const
{
    if (!ListView.IsValid())
    {
        return;
    }
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
        if (auto* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
        {
            Subsystem->OpenEditorForAssets(ToOpen);
        }
    }
}
