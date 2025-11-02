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
#include "RuleRangerProjectAction.h"
#include "EnforceGameplayTagRemapsSortedAction.generated.h"

/**
 * Ensure UGameplayTagsSettings.CategoryRemapping is sorted by BaseCategory (ascending, case-sensitive)
 * and contains no duplicates (keeping only the last occurrence of each BaseCategory).
 */
UCLASS(DisplayName = "Enforce Gameplay Tag Remaps Sorted", CollapseCategories, DefaultToInstanced, EditInlineNew)
class UEnforceGameplayTagRemapsSortedAction final : public URuleRangerProjectAction
{
    GENERATED_BODY()

public:
    virtual void Apply(URuleRangerProjectActionContext* ActionContext) override;
};
