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
#include "RuleRangerAction.h"
#include "EnsureNoMetaSoundSourceReferenceAction.generated.h"

class UMetaSoundSource;

/**
 * A specialized rule enforcement action designed to ensure that no direct references to MetaSoundSource
 * instances are used, except for those explicitly allowed via the AllowList.
 *
 * This is used to enforce a particular policy that may or may not be relevant for your project.
 * Only adopt this rule if it works for you.
 */
UCLASS(DisplayName = "Ensure No Direct MetaSoundSource References")
class RULERANGER_API UEnsureNoMetaSoundSourceReferenceAction final : public URuleRangerAction
{
    GENERATED_BODY()

public:
    UEnsureNoMetaSoundSourceReferenceAction();

protected:
    virtual void Apply(URuleRangerActionContext* ActionContext, UObject* Object) override;

private:
    /** Non-Preset MetaSourceSource instances that are allowed to be referenced directly */
    UPROPERTY(EditAnywhere,
              Category = "Rule Ranger",
              meta = (AllowedClasses = "/Script/MetasoundEngine.MetaSoundSource", AllowPrivateAccess = true))
    TArray<FSoftObjectPath> AllowList;

    bool IsReferenceAllowed(const UObject* MetaSoundSource, const UObject* Other) const;
};
