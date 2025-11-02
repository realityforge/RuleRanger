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
#include "Engine/DataTable.h"
#include "GameplayTagsSettings.h"
#include "RuleRangerProjectAction.h"
#include "EnforceGameplayTagRemapsPresentAction.generated.h"

/**
 * Row describing a tag category, a description for UI, and default target categories.
 * The row name is treated as the category key (e.g., InputTagCategory, AbilityTagCategory, etc.).
 */
USTRUCT(BlueprintType)
struct FRuleRangerTagCategoryRow : public FTableRowBase
{
    GENERATED_BODY()

    /** A friendly description for the category. */
    UPROPERTY(EditAnywhere, Category = "Category")
    FText Description;

    /** Default target category names used to initialize GameplayTags Category Remapping if missing. */
    UPROPERTY(EditAnywhere, Category = "Category")
    TArray<FString> DefaultTargets;
};

/**
 * Ensure that all specified GameplayTags CategoryRemapping entries are present by BaseCategory.
 * Presence is validated only by BaseCategory; existing entries are considered compliant as-is.
 *
 * In dry-run (Scan) mode, missing categories generate errors. In Fix mode, missing entries are added
 * using the targets from the first matching source (inline Remaps or a DataTable row).
 */
UCLASS(DisplayName = "Enforce Gameplay Tag Remaps Present", CollapseCategories, DefaultToInstanced, EditInlineNew)
class UEnforceGameplayTagRemapsPresentAction final : public URuleRangerProjectAction
{
    GENERATED_BODY()

public:
    /** Inline list of remaps expected to be present. Only BaseCategory is validated for presence. */
    UPROPERTY(EditAnywhere, Category = "Default", meta = (DisplayName = "Expected Remaps"))
    TArray<FGameplayTagCategoryRemap> Remaps;

    /** DataTables listing expected remaps (rows of type RuleRangerTagCategoryRow). */
    UPROPERTY(EditAnywhere,
              Category = "Default",
              meta = (DisplayName = "Remap Tables",
                      RequiredAssetDataTags = "RowStructure=/Script/RuleRangerGameplayTags.RuleRangerTagCategoryRow",
                      ForceShowPluginContent = "true"))
    TArray<TObjectPtr<UDataTable>> RemapTables;

    virtual void Apply(URuleRangerProjectActionContext* ActionContext) override;
};
