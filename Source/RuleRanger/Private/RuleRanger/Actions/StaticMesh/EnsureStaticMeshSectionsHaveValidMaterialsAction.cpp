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
#include "EnsureStaticMeshSectionsHaveValidMaterialsAction.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureStaticMeshSectionsHaveValidMaterialsAction)

void UEnsureStaticMeshSectionsHaveValidMaterialsAction::Apply(URuleRangerActionContext* ActionContext, UObject* Object)
{
    const auto StaticMesh = CastChecked<UStaticMesh>(Object);
    const auto RenderData = StaticMesh->GetRenderData();
    if (RenderData && !RenderData->LODResources.IsEmpty())
    {
        const auto& StaticMaterials = StaticMesh->GetStaticMaterials();
        TArray<FString> Failures;
        for (int32 LODIndex = 0; LODIndex < RenderData->LODResources.Num(); ++LODIndex)
        {
            const auto& Sections = RenderData->LODResources[LODIndex].Sections;
            for (int32 SectionIndex = 0; SectionIndex < Sections.Num(); ++SectionIndex)
            {
                const auto MaterialIndex = Sections[SectionIndex].MaterialIndex;
                if (!StaticMaterials.IsValidIndex(MaterialIndex))
                {
                    Failures.Add(
                        FString::Printf(TEXT("LOD%d Section%d: material index %d is out of range for %d slot(s)"),
                                        LODIndex,
                                        SectionIndex,
                                        MaterialIndex,
                                        StaticMaterials.Num()));
                }
                else if (!StaticMaterials[MaterialIndex].MaterialInterface)
                {
                    const auto SlotName = StaticMaterials[MaterialIndex].MaterialSlotName.IsNone()
                        ? TEXT("<unnamed>")
                        : StaticMaterials[MaterialIndex].MaterialSlotName.ToString();
                    Failures.Add(FString::Printf(TEXT("LOD%d Section%d: material slot %d ('%s') has a null material"),
                                                 LODIndex,
                                                 SectionIndex,
                                                 MaterialIndex,
                                                 *SlotName));
                }
            }
        }

        if (!Failures.IsEmpty())
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("Failures"), FText::FromString(FString::Join(Failures, TEXT("; "))));
            ActionContext->Error(
                FText::Format(NSLOCTEXT("RuleRanger",
                                        "EnsureStaticMeshSectionsHaveValidMaterialsAction_Error",
                                        "StaticMesh rendered sections have invalid material assignments: {Failures}."),
                              Arguments));
        }
    }
}

UClass* UEnsureStaticMeshSectionsHaveValidMaterialsAction::GetExpectedType() const
{
    return UStaticMesh::StaticClass();
}
