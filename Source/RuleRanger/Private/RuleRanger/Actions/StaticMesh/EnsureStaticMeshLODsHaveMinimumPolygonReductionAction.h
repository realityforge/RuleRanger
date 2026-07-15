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
#include "EnsureStaticMeshLODsHaveMinimumPolygonReductionAction.generated.h"

/** Action to ensure that each rendered StaticMesh LOD reduces polygon count by the configured percentage. */
UCLASS(DisplayName = "Ensure Static Mesh LODs Have Minimum Polygon Reduction")
class UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** The minimum polygon reduction required between each adjacent rendered LOD. */
    UPROPERTY(EditAnywhere,
              meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0", Units = "Percent"))
    float MinReductionPercentage{ 30.0f };

public:
    virtual void Apply(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() const override;
};
