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
#include "EnsureStaticMeshMaterialSlotCountWithinLimitAction.generated.h"

/** Action to ensure that a StaticMesh material slot count does not exceed the configured limit. */
UCLASS(DisplayName = "Ensure Static Mesh Material Slot Count Within Limit")
class UEnsureStaticMeshMaterialSlotCountWithinLimitAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** The maximum number of material slots allowed on the StaticMesh. */
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0"))
    int32 MaxMaterialSlots{ 4 };

public:
    virtual void Apply(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() const override;
};
