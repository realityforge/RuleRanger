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
#include "EnsureStaticMeshCollisionComplexityWithinLimitsAction.generated.h"

/** Action to ensure that a StaticMesh's effective collision geometry stays within configured complexity limits. */
UCLASS(DisplayName = "Ensure Static Mesh Collision Complexity Within Limits")
class UEnsureStaticMeshCollisionComplexityWithinLimitsAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** The maximum number of active conventional simple collision shapes. */
    UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
    int32 MaxSimpleShapes{ 16 };

    /** The maximum number of active convex collision hulls. */
    UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
    int32 MaxConvexHulls{ 8 };

    /** The maximum total number of authored vertices across active convex collision hulls. */
    UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
    int32 MaxTotalConvexVertices{ 256 };

    /** The maximum number of triangles in the effective complex collision mesh. */
    UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
    int32 MaxComplexTriangles{ 5000 };

public:
    virtual void Apply(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() const override;
};
