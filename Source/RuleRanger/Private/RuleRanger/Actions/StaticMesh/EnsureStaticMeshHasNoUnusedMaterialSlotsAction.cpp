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
#include "EnsureStaticMeshHasNoUnusedMaterialSlotsAction.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureStaticMeshHasNoUnusedMaterialSlotsAction)

void UEnsureStaticMeshHasNoUnusedMaterialSlotsAction::Apply(URuleRangerActionContext* ActionContext, UObject* Object)
{
    const auto StaticMesh = CastChecked<UStaticMesh>(Object);
    const auto RenderData = StaticMesh->GetRenderData();
    if (RenderData && !RenderData->LODResources.IsEmpty())
    {
        TSet<int32> ReferencedMaterialIndices;
        for (const auto& LODResource : RenderData->LODResources)
        {
            for (const auto& Section : LODResource.Sections)
            {
                ReferencedMaterialIndices.Add(Section.MaterialIndex);
            }
        }

        const auto& StaticMaterials = StaticMesh->GetStaticMaterials();
        TArray<FString> UnusedSlots;
        for (int32 MaterialIndex = 0; MaterialIndex < StaticMaterials.Num(); ++MaterialIndex)
        {
            if (!ReferencedMaterialIndices.Contains(MaterialIndex))
            {
                const auto SlotName = StaticMaterials[MaterialIndex].MaterialSlotName.IsNone()
                    ? TEXT("<unnamed>")
                    : StaticMaterials[MaterialIndex].MaterialSlotName.ToString();
                UnusedSlots.Add(FString::Printf(TEXT("slot %d ('%s')"), MaterialIndex, *SlotName));
            }
        }

        if (!UnusedSlots.IsEmpty())
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("UnusedSlots"), FText::FromString(FString::Join(UnusedSlots, TEXT(", "))));
            ActionContext->Warning(
                FText::Format(NSLOCTEXT("RuleRanger",
                                        "EnsureStaticMeshHasNoUnusedMaterialSlotsAction_Warning",
                                        "StaticMesh has material slots unused by every rendered LOD: {UnusedSlots}."),
                              Arguments));
        }
    }
}

UClass* UEnsureStaticMeshHasNoUnusedMaterialSlotsAction::GetExpectedType() const
{
    return UStaticMesh::StaticClass();
}
