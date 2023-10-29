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
#include "RemoveNamePrefixAction.generated.h"

/**
 * Action to remove a prefix from a name.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"),
       Blueprintable,
       BlueprintType,
       CollapseCategories,
       DefaultToInstanced,
       EditInlineNew)
class RULERANGER_API URemoveNamePrefixAction : public URuleRangerAction
{
    GENERATED_BODY()

public:
    virtual void Apply_Implementation(TScriptInterface<IRuleRangerActionContext>& ActionContext,
                                      UObject* Object) override;

private:
    /** The prefix to remove. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    FString Prefix;
    /** A flag controlling whether matching is Case Sensitive or not. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    bool bCaseSensitive{ true };
};
