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
#include "EnsureStaticMeshMaterialSlotCountWithinLimitAction.h"
#include "Engine/StaticMesh.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureStaticMeshMaterialSlotCountWithinLimitAction)

void UEnsureStaticMeshMaterialSlotCountWithinLimitAction::Apply(URuleRangerActionContext* ActionContext,
                                                                UObject* Object)
{
    const auto StaticMesh = CastChecked<UStaticMesh>(Object);
    const auto ActualMaterialSlots = StaticMesh->GetStaticMaterials().Num();
    const auto EffectiveMaxMaterialSlots = FMath::Max(0, MaxMaterialSlots);
    if (ActualMaterialSlots > EffectiveMaxMaterialSlots)
    {
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("ActualMaterialSlots"), ActualMaterialSlots);
        Arguments.Add(TEXT("MaxMaterialSlots"), EffectiveMaxMaterialSlots);
        ActionContext->Error(FText::Format(
            NSLOCTEXT(
                "RuleRanger",
                "EnsureStaticMeshMaterialSlotCountWithinLimitAction_Error",
                "StaticMesh has {ActualMaterialSlots} material slot(s), exceeding the allowed maximum of {MaxMaterialSlots}."),
            Arguments));
    }
}

UClass* UEnsureStaticMeshMaterialSlotCountWithinLimitAction::GetExpectedType() const
{
    return UStaticMesh::StaticClass();
}
