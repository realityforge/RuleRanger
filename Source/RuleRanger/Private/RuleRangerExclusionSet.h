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
#include "Engine/DataAsset.h"
#include "RuleRangerExclusionSet.generated.h"

struct FRuleRangerRuleExclusion;
class FObjectPreSaveContext;

/**
 * An asset containing a reusable set of rule exclusions.
 */
UCLASS(MinimalAPI, AutoExpandCategories = ("Rule Ranger"), CollapseCategories, EditInlineNew)
class URuleRangerExclusionSet : public UDataAsset
{
    GENERATED_BODY()

public:
    /** A description of what this exclusion set is for. */
    UPROPERTY(EditDefaultsOnly, Category = "Rule Ranger")
    FText Description;

    /** A set of exclusions. */
    UPROPERTY(EditDefaultsOnly, Category = "Rule Sets", meta = (TitleProperty = "EditorFriendlyTitle"))
    TArray<FRuleRangerRuleExclusion> Exclusions;

    /** Updates the editor only titles for subobjects after the asset is initially loaded. */
    virtual void PostLoad() override;

    /** Updates the editor only titles for subobjects. */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

    /** Updates the editor only titles for subobjects when properties inside structs are modified. */
    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

    // Sort certain properties before saving
    virtual void PreSave(FObjectPreSaveContext SaveContext) override;

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
    void UpdateExclusionsEditorFriendlyTitles();
};
