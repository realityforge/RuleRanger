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
    SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTable);
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

TSharedRef<SWidget> SRuleRangerRunRow::GenerateWidgetForColumn(const FName& ColumnName)
{
    if (TEXT("Severity") == ColumnName)
    {
        return SNew(SImage).Image(GetSeverityBrush(Item->Severity));
    }
    else if (TEXT("Message") == ColumnName)
    {
        return SNew(STextBlock).Text(Item->Text).WrapTextAt(600.f);
    }
    else if (TEXT("Asset") == ColumnName)
    {
        const auto Name = Item->Asset.IsValid() ? FText::FromString(Item->Asset->GetName()) : FText::GetEmpty();
        if (!Item->Asset.IsValid())
        {
            return SNew(STextBlock).Text(Name);
        }
        else
        {
            return SNew(SHyperlink).Text(Name).OnNavigate_Lambda([Weak = Item->Asset] {
                if (const auto* Obj = Weak.Get())
                {
                    if (auto* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
                    {
                        TArray<UObject*> Open;
                        Open.Add(const_cast<UObject*>(Obj));
                        Subsystem->OpenEditorForAssets(Open);
                    }
                }
            });
        }
    }
    else if (TEXT("Rule") == ColumnName)
    {
        const auto bHasRule = Item->Rule.IsValid();
        const auto bHasProjectRule = Item->ProjectRule.IsValid();
        FText Name;
        if (bHasRule)
        {
            Name = FText::FromString(Item->Rule->GetName());
        }
        else if (bHasProjectRule)
        {
            Name = FText::FromString(Item->ProjectRule->GetName());
        }
        else
        {
            Name = FText::GetEmpty();
        }
        if (!bHasRule && !bHasProjectRule)
        {
            return SNew(STextBlock).Text(Name);
        }
        else
        {
            const auto RuleWeak = Item->Rule;
            const auto ProjectRuleWeak = Item->ProjectRule;
            return SNew(SHyperlink).Text(Name).OnNavigate_Lambda([RuleWeak, ProjectRuleWeak] {
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
            });
        }
    }
    else if (TEXT("RuleSet") == ColumnName)
    {
        const auto Name = Item->RuleSet.IsValid() ? FText::FromString(Item->RuleSet->GetName()) : FText::GetEmpty();
        if (!Item->RuleSet.IsValid())
        {
            return SNew(STextBlock).Text(Name);
        }
        else
        {
            return SNew(SHyperlink).Text(Name).OnNavigate_Lambda([RuleSetRef = Item->RuleSet] {
                if (const auto RuleSet = RuleSetRef.Get())
                {
                    if (const auto Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
                    {
                        TArray<UObject*> Open;
                        Open.Add(const_cast<URuleRangerRuleSet*>(RuleSet));
                        Subsystem->OpenEditorForAssets(Open);
                    }
                }
            });
        }
    }
    else
    {
        return SNew(STextBlock).Text(FText::GetEmpty());
    }
}
