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
#include "Widgets/Views/STableRow.h"

template <typename ItemType>
class SListView;

struct FRuleRangerMessageRow;
enum class ERuleRangerToolSeverity : uint8;

class SRuleRangerRunRow final : public SMultiColumnTableRow<TSharedPtr<FRuleRangerMessageRow>>
{
public:
    SLATE_BEGIN_ARGS(SRuleRangerRunRow) {}
    SLATE_ARGUMENT(TSharedPtr<FRuleRangerMessageRow>, Item)
    SLATE_ARGUMENT(TWeakPtr<SListView<TSharedPtr<FRuleRangerMessageRow>>>, ListView)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable);

private:
    TSharedPtr<FRuleRangerMessageRow> Item;
    TWeakPtr<SListView<TSharedPtr<FRuleRangerMessageRow>>> OwnerListView;

    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

    static const FSlateBrush* GetSeverityBrush(ERuleRangerToolSeverity Severity);
    static FText GetSeverityText(ERuleRangerToolSeverity Severity);
    static FSlateColor GetSeverityTextColor(ERuleRangerToolSeverity Severity);
};
