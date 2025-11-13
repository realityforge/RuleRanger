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
#include "RuleRanger/UI/ToolTab/SRuleRangerRunRow.h"
#include "HAL/PlatformApplicationMisc.h"
#include "InputCoreTypes.h"
#include "RuleRanger/UI/RuleRangerStyle.h"
#include "RuleRanger/UI/ToolTab/SRuleRangerToolPanel.h"
#include "RuleRangerProjectRule.h"
#include "RuleRangerRule.h"
#include "RuleRangerRuleSet.h"
#include "Styling/AppStyle.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Text/STextBlock.h"

void SRuleRangerRunRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
{
    Item = InArgs._Item;
    OwnerListView = InArgs._ListView;
    SMultiColumnTableRow::Construct(FSuperRowType::FArguments().Padding(FMargin(0.f, 2.f)), InOwnerTable);
}

FText SRuleRangerRunRow::GetSeverityText(const ERuleRangerToolSeverity Severity)
{
    switch (Severity)
    {
        case ERuleRangerToolSeverity::Error:
            return NSLOCTEXT("RuleRanger", "SeverityError", "Error");
        case ERuleRangerToolSeverity::Warning:
            return NSLOCTEXT("RuleRanger", "SeverityWarning", "Warning");
        default:
            return NSLOCTEXT("RuleRanger", "SeverityInfo", "Info");
    }
}

FSlateColor SRuleRangerRunRow::GetSeverityTextColor(const ERuleRangerToolSeverity Severity)
{
    switch (Severity)
    {
        case ERuleRangerToolSeverity::Error:
            return FSlateColor(FLinearColor(0.92f, 0.34f, 0.34f));
        case ERuleRangerToolSeverity::Warning:
            return FSlateColor(FLinearColor(0.98f, 0.78f, 0.25f));
        default:
            return FSlateColor(FLinearColor(0.90f, 0.90f, 0.90f));
    }
}

const FSlateBrush* SRuleRangerRunRow::GetSeverityBrush(const ERuleRangerToolSeverity Severity)
{
    switch (Severity)
    {
        case ERuleRangerToolSeverity::Warning:
            return FRuleRangerStyle::GetWarningMessageBrush();
        case ERuleRangerToolSeverity::Error:
            return FRuleRangerStyle::GetErrorMessageBrush();
        case ERuleRangerToolSeverity::Info:
        default:
            return FRuleRangerStyle::GetNoteMessageBrush();
    }
}

FReply SRuleRangerRunRow::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        if (const auto Pinned = OwnerListView.Pin())
        {
            bool bIsAlreadySelected = false;
            for (const auto& Sel : Pinned->GetSelectedItems())
            {
                if (Sel == Item)
                {
                    bIsAlreadySelected = true;
                    break;
                }
            }
            if (!bIsAlreadySelected)
            {
                Pinned->SetSelection(Item, ESelectInfo::OnMouseClick);
            }
        }
    }
    return SMultiColumnTableRow::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SRuleRangerRunRow::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    const auto Key = InKeyEvent.GetKey();
    if (EKeys::Enter == Key || EKeys::Virtual_Accept == Key)
    {
        if (const auto Pinned = OwnerListView.Pin())
        {
            for (const auto& Row : Pinned->GetSelectedItems())
            {
                if (Row.IsValid())
                {
                    if (const auto Object = Row->Asset.IsValid() ? const_cast<UObject*>(Row->Asset.Get()) : nullptr)
                    {
                        if (const auto Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
                        {
                            TArray<UObject*> ToOpen;
                            ToOpen.Add(Object);
                            Subsystem->OpenEditorForAssets(ToOpen);
                        }
                        return FReply::Handled();
                    }
                }
            }
        }
    }
    else if (EKeys::C == Key && InKeyEvent.IsControlDown())
    {
        if (const auto Pinned = OwnerListView.Pin())
        {
            FString Combined;
            bool bFirst = true;
            for (const auto& Row : Pinned->GetSelectedItems())
            {
                if (Row.IsValid())
                {
                    if (!bFirst)
                    {
                        Combined.AppendChar(TEXT('\n'));
                    }
                    bFirst = false;
                    Combined.Append(Row->Text.ToString());
                }
            }
            if (!Combined.IsEmpty())
            {
                FPlatformApplicationMisc::ClipboardCopy(*Combined);
                return FReply::Handled();
            }
        }
    }

    return SMultiColumnTableRow::OnKeyDown(MyGeometry, InKeyEvent);
}

TSharedRef<SWidget> SRuleRangerRunRow::GenerateWidgetForColumn(const FName& ColumnName)
{
    if (TEXT("Severity") == ColumnName)
    {
        return SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            .WidthOverride(20.f)
            .HeightOverride(20.f)
                [SNew(SImage).ToolTipText(GetSeverityText(Item->Severity)).Image(GetSeverityBrush(Item->Severity))];
    }
    else if (TEXT("Message") == ColumnName)
    {
        return SNew(SBox).Padding(FMargin(6.f, 6.f))[SNew(STextBlock)
                                                         .Text(Item->Text)
                                                         .ColorAndOpacity(GetSeverityTextColor(Item->Severity))
                                                         .AutoWrapText(true)];
    }
    else if (TEXT("Asset") == ColumnName)
    {
        const auto Name = Item->Asset.IsValid() ? FText::FromString(Item->Asset->GetName()) : FText::GetEmpty();
        const auto ToolTip =
            Item->Asset.IsValid() ? FText::FromString(Item->Asset.Get()->GetPathName()) : FText::GetEmpty();
        if (!Item->Asset.IsValid())
        {
            return SNew(SBox).Padding(FMargin(6.f, 3.f))[SNew(STextBlock).Text(Name).ToolTipText(ToolTip)];
        }
        else
        {
            return SNew(SBox).Padding(
                FMargin(6.f,
                        3.f))[SNew(SHyperlink).Text(Name).ToolTipText(ToolTip).OnNavigate_Lambda([Weak = Item->Asset] {
                if (const auto Object = Weak.Get())
                {
                    if (const auto Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
                    {
                        TArray<UObject*> Open;
                        Open.Add(const_cast<UObject*>(Object));
                        Subsystem->OpenEditorForAssets(Open);
                    }
                }
            })];
        }
    }
    else if (TEXT("Rule") == ColumnName)
    {
        const auto bHasRule = Item->Rule.IsValid();
        const auto bHasProjectRule = Item->ProjectRule.IsValid();
        FText Name;
        FText ToolTip;
        if (bHasRule)
        {
            Name = FText::FromString(Item->Rule->GetName());
            ToolTip = FText::FromString(Item->Rule.Get()->GetPathName());
        }
        else if (bHasProjectRule)
        {
            Name = FText::FromString(Item->ProjectRule->GetName());
            ToolTip = FText::FromString(Item->ProjectRule.Get()->GetPathName());
        }
        else
        {
            Name = FText::GetEmpty();
            ToolTip = FText::GetEmpty();
        }
        if (!bHasRule && !bHasProjectRule)
        {
            return SNew(SBox).Padding(FMargin(6.f, 3.f))[SNew(STextBlock).Text(Name).ToolTipText(ToolTip)];
        }
        else
        {
            const auto RuleWeak = Item->Rule;
            const auto ProjectRuleWeak = Item->ProjectRule;
            return SNew(SBox).Padding(FMargin(
                6.f,
                3.f))[SNew(SHyperlink).Text(Name).ToolTipText(ToolTip).OnNavigate_Lambda([RuleWeak, ProjectRuleWeak] {
                const auto Rule = RuleWeak.IsValid() ? static_cast<const UObject*>(RuleWeak.Get())
                    : ProjectRuleWeak.IsValid()      ? ProjectRuleWeak.Get()
                                                     : nullptr;
                if (Rule)
                {
                    if (const auto Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
                    {
                        TArray<UObject*> Open;
                        Open.Add(const_cast<UObject*>(Rule));
                        Subsystem->OpenEditorForAssets(Open);
                    }
                }
            })];
        }
    }
    else if (TEXT("RuleSet") == ColumnName)
    {
        const auto Name = Item->RuleSet.IsValid() ? FText::FromString(Item->RuleSet->GetName()) : FText::GetEmpty();
        const auto ToolTip =
            Item->RuleSet.IsValid() ? FText::FromString(Item->RuleSet.Get()->GetPathName()) : FText::GetEmpty();
        if (!Item->RuleSet.IsValid())
        {
            return SNew(SBox).Padding(FMargin(6.f, 3.f))[SNew(STextBlock).Text(Name).ToolTipText(ToolTip)];
        }
        else
        {
            return SNew(SBox).Padding(FMargin(
                6.f,
                3.f))[SNew(SHyperlink).Text(Name).ToolTipText(ToolTip).OnNavigate_Lambda([RuleSetRef = Item->RuleSet] {
                if (const auto RuleSet = RuleSetRef.Get())
                {
                    if (const auto Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
                    {
                        TArray<UObject*> Open;
                        Open.Add(const_cast<URuleRangerRuleSet*>(RuleSet));
                        Subsystem->OpenEditorForAssets(Open);
                    }
                }
            })];
        }
    }
    else
    {
        return SNew(STextBlock).Text(FText::GetEmpty());
    }
}
