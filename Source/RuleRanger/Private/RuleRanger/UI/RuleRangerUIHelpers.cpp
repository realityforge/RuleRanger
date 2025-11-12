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
#include "RuleRanger/UI/RuleRangerUIHelpers.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace RuleRangerUI
{
    TSharedRef<SWidget> MakeIconLabel(const FSlateIcon& Icon, const FText& Label, const float IconRightPadding)
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
                  .AutoWidth()
                  .VAlign(VAlign_Center)
                  .Padding(FMargin(0.f, 2.f, IconRightPadding, 2.f))[SNew(SImage).Image(Icon.GetIcon())]
            + SHorizontalBox::Slot().VAlign(VAlign_Center)[SNew(STextBlock).Text(Label)];
    }

    TSharedRef<SWidget>
    MakeIconLabelFromBrush(const FSlateBrush* const Brush, const FText& Label, const float IconRightPadding)
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
                  .AutoWidth()
                  .VAlign(VAlign_Center)
                  .Padding(FMargin(0.f, 2.f, IconRightPadding, 2.f))[SNew(SImage).Image(Brush)]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(STextBlock).Text(Label)];
    }

    void AddMenuEntry(FMenuBuilder& MenuBuilder,
                      const FText& Label,
                      const FText& ToolTip,
                      const FSlateIcon& Icon,
                      const FExecuteAction& Execute,
                      const FCanExecuteAction& CanExecute)
    {
        MenuBuilder.AddMenuEntry(Label, ToolTip, Icon, FUIAction(Execute, CanExecute));
    }
} // namespace RuleRangerUI
