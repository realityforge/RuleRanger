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
#include "EnsureStaticMeshHasMinimumLODsAction.h"
#include "Engine/StaticMesh.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureStaticMeshHasMinimumLODsAction)

void UEnsureStaticMeshHasMinimumLODsAction::Apply(URuleRangerActionContext* ActionContext, UObject* Object)
{
    const auto StaticMesh = CastChecked<UStaticMesh>(Object);
    const auto ActualLODCount = StaticMesh->GetNumLODs();
    const auto EffectiveMinRequiredLODs = FMath::Max(1, MinRequiredLODs);
    if (ActualLODCount < EffectiveMinRequiredLODs)
    {
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("ActualLODCount"), ActualLODCount);
        Arguments.Add(TEXT("MinRequiredLODs"), EffectiveMinRequiredLODs);
        ActionContext->Error(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "EnsureStaticMeshHasMinimumLODsAction_Error",
                                    "StaticMesh has {ActualLODCount} LOD(s) but requires at least {MinRequiredLODs}."),
                          Arguments));
    }
}

UClass* UEnsureStaticMeshHasMinimumLODsAction::GetExpectedType() const
{
    return UStaticMesh::StaticClass();
}
