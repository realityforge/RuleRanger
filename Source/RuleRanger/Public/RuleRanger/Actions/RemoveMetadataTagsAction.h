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
#include "RuleRangerAction.h"
#include "RemoveMetadataTagsAction.generated.h"

/**
 * Action to remove metadata tags with one of the specified keys.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"),
       Blueprintable,
       BlueprintType,
       CollapseCategories,
       DefaultToInstanced,
       EditInlineNew)
class RULERANGER_API URemoveMetadataTagsAction : public URuleRangerAction
{
    GENERATED_BODY()

public:
    virtual void Apply_Implementation(TScriptInterface<IRuleRangerActionContext>& ActionContext,
                                      UObject* Object) override;

private:
    /** The metadata keys to remove */
    UPROPERTY(EditAnywhere,
              BlueprintReadWrite,
              Category = "Rule Ranger",
              meta = (AllowPrivateAccess, ExposeOnSpawn, MultiLine))
    TArray<FName> Keys;
};