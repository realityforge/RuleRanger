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
#include "EnsureStaticMeshHasMinimumLODsAction.generated.h"

/**
 * Action to ensure that a StaticMesh has the configured minimum number of LODs.
 */
UCLASS(DisplayName = "Ensure Static Mesh Has Minimum LODs")
class UEnsureStaticMeshHasMinimumLODsAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** The minimum number of LODs required by the StaticMesh. */
    UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
    int32 MinRequiredLODs{ 3 };

public:
    virtual void Apply(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() const override;
};
