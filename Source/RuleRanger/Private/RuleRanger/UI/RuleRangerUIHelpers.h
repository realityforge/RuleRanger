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

namespace RuleRangerUI
{
    /**
     * Build a compact horizontal widget with icon + label, suitable as button content.
     */
    TSharedRef<SWidget> MakeIconLabel(const FSlateIcon& Icon, const FText& Label, float IconRightPadding = 8.f);

    /**
     * Build a compact horizontal widget with a brush + label, e.g. for severity filters.
     */
    TSharedRef<SWidget>
    MakeIconLabelFromBrush(const FSlateBrush* Brush, const FText& Label, float IconRightPadding = 6.f);

    /**
     * Add a menu entry with icon, label, tooltip and actions.
     */
    void AddMenuEntry(FMenuBuilder& MenuBuilder,
                      const FText& Label,
                      const FText& ToolTip,
                      const FSlateIcon& Icon,
                      const FExecuteAction& Execute,
                      const FCanExecuteAction& CanExecute = FCanExecuteAction());
} // namespace RuleRangerUI
