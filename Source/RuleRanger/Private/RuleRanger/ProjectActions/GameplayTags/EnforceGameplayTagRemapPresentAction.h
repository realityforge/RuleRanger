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
#include "EnforceGameplayTagRemapPresentAction.generated.h"

/**
 * Ensure a GameplayTags CategoryRemapping entry exists for the specified Name.
 * In dry-run (Scan) mode it reports an error if missing; in Fix mode and DefaultTargets is configured,
 * it adds a new remapping entry using the provided defaults.
 */
UCLASS(CollapseCategories, DefaultToInstanced, EditInlineNew)
class UEnforceGameplayTagRemapPresentAction final : public URuleRangerProjectAction
{
    GENERATED_BODY()

public:
    /** The category name that must be present in GameplayTags CategoryRemapping. */
    UPROPERTY(EditAnywhere, Category = "Default")
    FString Name;

    /** Default remap targets used when fixing a missing mapping. */
    UPROPERTY(EditAnywhere, Category = "Default")
    TArray<FString> DefaultTargets;

    virtual void Apply(URuleRangerProjectActionContext* ActionContext) override;
};
