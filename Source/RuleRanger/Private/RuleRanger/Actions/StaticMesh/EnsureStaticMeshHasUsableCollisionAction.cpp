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
#include "EnsureStaticMeshHasUsableCollisionAction.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureStaticMeshHasUsableCollisionAction)

namespace RuleRanger::StaticMeshCollision
{
    bool HasEnabledSimpleCollision(const UBodySetup& BodySetup)
    {
        const auto ElementCount = BodySetup.AggGeom.GetElementCount();
        for (int32 ElementIndex = 0; ElementIndex < ElementCount; ++ElementIndex)
        {
            const auto Element = BodySetup.AggGeom.GetElement(ElementIndex);
            if (Element && Element->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                return true;
            }
        }
        return false;
    }
} // namespace RuleRanger::StaticMeshCollision

void UEnsureStaticMeshHasUsableCollisionAction::Apply(URuleRangerActionContext* ActionContext, UObject* Object)
{
    const auto StaticMesh = CastChecked<UStaticMesh>(Object);
    const auto BodySetup = StaticMesh->GetBodySetup();
    if (!BodySetup)
    {
        ActionContext->Error(NSLOCTEXT("RuleRanger",
                                       "EnsureStaticMeshHasUsableCollisionAction_MissingBodySetup",
                                       "StaticMesh has no BodySetup and therefore no usable collision."));
        return;
    }

    const auto CollisionTraceFlag = BodySetup->GetCollisionTraceFlag();
    const auto bHasSimpleCollision = RuleRanger::StaticMeshCollision::HasEnabledSimpleCollision(*BodySetup);
    const auto bHasComplexCollision = StaticMesh->ContainsPhysicsTriMeshData(BodySetup->bMeshCollideAll);

    auto bHasUsableCollision{ false };
    FText MissingCollisionReason;
    switch (CollisionTraceFlag)
    {
        case CTF_UseSimpleAsComplex:
            bHasUsableCollision = bHasSimpleCollision;
            MissingCollisionReason = NSLOCTEXT(
                "RuleRanger",
                "EnsureStaticMeshHasUsableCollisionAction_MissingSimpleCollision",
                "effective collision trace mode requires simple collision, but the mesh has no enabled aggregate geometry");
            break;
        case CTF_UseComplexAsSimple:
            bHasUsableCollision = bHasComplexCollision;
            MissingCollisionReason = NSLOCTEXT(
                "RuleRanger",
                "EnsureStaticMeshHasUsableCollisionAction_MissingComplexCollision",
                "effective collision trace mode requires complex collision, but the mesh has no valid triangle collision data");
            break;
        case CTF_UseSimpleAndComplex:
            bHasUsableCollision = bHasSimpleCollision || bHasComplexCollision;
            MissingCollisionReason =
                NSLOCTEXT("RuleRanger",
                          "EnsureStaticMeshHasUsableCollisionAction_MissingSimpleAndComplexCollision",
                          "the mesh has neither enabled aggregate geometry nor valid triangle collision data");
            break;
        default:
            MissingCollisionReason = NSLOCTEXT("RuleRanger",
                                               "EnsureStaticMeshHasUsableCollisionAction_InvalidCollisionTraceMode",
                                               "the effective collision trace mode is invalid");
            break;
    }

    if (!bHasUsableCollision)
    {
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("CollisionTraceMode"), FText::FromString(LexToString(CollisionTraceFlag)));
        Arguments.Add(TEXT("Reason"), MissingCollisionReason);
        ActionContext->Error(FText::Format(
            NSLOCTEXT(
                "RuleRanger",
                "EnsureStaticMeshHasUsableCollisionAction_MissingCollision",
                "StaticMesh has no usable collision for effective collision trace mode '{CollisionTraceMode}': {Reason}."),
            Arguments));
    }

    if (BodySetup->DefaultInstance.GetCollisionEnabled(false) == ECollisionEnabled::NoCollision)
    {
        ActionContext->Error(
            NSLOCTEXT("RuleRanger",
                      "EnsureStaticMeshHasUsableCollisionAction_CollisionDisabled",
                      "StaticMesh collision is disabled: effective collision enabled state is NoCollision."));
    }
}

UClass* UEnsureStaticMeshHasUsableCollisionAction::GetExpectedType() const
{
    return UStaticMesh::StaticClass();
}
