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
#include "EnsureStaticMeshLODsHaveMinimumPolygonReductionAction.h"
#include "Engine/StaticMesh.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureStaticMeshLODsHaveMinimumPolygonReductionAction)

void UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction::Apply(URuleRangerActionContext* ActionContext,
                                                                   UObject* Object)
{
    const auto StaticMesh = CastChecked<UStaticMesh>(Object);
    if (!StaticMesh->IsNaniteEnabled())
    {
        const auto LODCount = StaticMesh->GetNumLODs();
        if (LODCount >= 2)
        {
            const auto EffectiveMinReductionPercentage = FMath::Clamp(MinReductionPercentage, 0.0f, 100.0f);
            TArray<FString> Failures;
            for (int32 LODIndex = 1; LODIndex < LODCount; ++LODIndex)
            {
                const auto PreviousTriangleCount = StaticMesh->GetNumTriangles(LODIndex - 1);
                const auto CurrentTriangleCount = StaticMesh->GetNumTriangles(LODIndex);
                if (PreviousTriangleCount == 0 || CurrentTriangleCount == 0)
                {
                    Failures.Add(FString::Printf(
                        TEXT(
                            "LOD%d->LOD%d: %d->%d triangles (cannot evaluate reduction because one or both LODs have zero triangles; requires %.1f%% reduction)"),
                        LODIndex - 1,
                        LODIndex,
                        PreviousTriangleCount,
                        CurrentTriangleCount,
                        EffectiveMinReductionPercentage));
                    continue;
                }

                const auto ReductionPercentage =
                    (static_cast<double>(PreviousTriangleCount) - static_cast<double>(CurrentTriangleCount))
                    / static_cast<double>(PreviousTriangleCount) * 100.0;
                if (ReductionPercentage < EffectiveMinReductionPercentage)
                {
                    if (ReductionPercentage < 0.0)
                    {
                        Failures.Add(FString::Printf(
                            TEXT("LOD%d->LOD%d: %d->%d triangles (%.1f%% increase; requires %.1f%% reduction)"),
                            LODIndex - 1,
                            LODIndex,
                            PreviousTriangleCount,
                            CurrentTriangleCount,
                            -ReductionPercentage,
                            EffectiveMinReductionPercentage));
                    }
                    else
                    {
                        Failures.Add(FString::Printf(
                            TEXT("LOD%d->LOD%d: %d->%d triangles (%.1f%% reduction; requires %.1f%% reduction)"),
                            LODIndex - 1,
                            LODIndex,
                            PreviousTriangleCount,
                            CurrentTriangleCount,
                            ReductionPercentage,
                            EffectiveMinReductionPercentage));
                    }
                }
            }

            if (!Failures.IsEmpty())
            {
                FFormatNamedArguments Arguments;
                Arguments.Add(TEXT("Failures"), FText::FromString(FString::Join(Failures, TEXT("; "))));
                ActionContext->Error(
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "EnsureStaticMeshLODsHaveMinimumPolygonReductionAction_Error",
                                            "StaticMesh LOD polygon reduction violations: {Failures}."),
                                  Arguments));
            }
        }
    }
}

UClass* UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction::GetExpectedType() const
{
    return UStaticMesh::StaticClass();
}
