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
#include "EnsureStaticMeshCollisionComplexityWithinLimitsAction.h"
#include "Engine/StaticMesh.h"
#include "Interface_CollisionDataProviderCore.h"
#include "PhysicsEngine/BodySetup.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureStaticMeshCollisionComplexityWithinLimitsAction)

namespace RuleRanger::StaticMeshCollisionComplexity
{
    struct FSimpleCollisionMetrics
    {
        int32 ShapeCount{ 0 };
        int32 ConvexHullCount{ 0 };
        int32 ConvexVertexCount{ 0 };
        int32 QueryShapeCount{ 0 };
        int32 PhysicsShapeCount{ 0 };
        int32 ProbeShapeCount{ 0 };
    };

    template <typename TShape>
    void AddShapeMetrics(const TShape& Shape,
                         const ECollisionEnabled::Type BodyCollisionEnabled,
                         FSimpleCollisionMetrics& Metrics)
    {
        const auto EffectiveCollisionEnabled =
            CollisionEnabledIntersection(BodyCollisionEnabled, Shape.GetCollisionEnabled());
        if (EffectiveCollisionEnabled == ECollisionEnabled::NoCollision)
        {
            return;
        }

        ++Metrics.ShapeCount;
        Metrics.QueryShapeCount += CollisionEnabledHasQuery(EffectiveCollisionEnabled) ? 1 : 0;
        Metrics.PhysicsShapeCount += CollisionEnabledHasPhysics(EffectiveCollisionEnabled) ? 1 : 0;
        Metrics.ProbeShapeCount += CollisionEnabledHasProbe(EffectiveCollisionEnabled) ? 1 : 0;
    }

    FSimpleCollisionMetrics MeasureSimpleCollision(const UBodySetup& BodySetup,
                                                   const ECollisionEnabled::Type BodyCollisionEnabled)
    {
        FSimpleCollisionMetrics Metrics;
        for (const auto& Sphere : BodySetup.AggGeom.SphereElems)
        {
            AddShapeMetrics(Sphere, BodyCollisionEnabled, Metrics);
        }
        for (const auto& Box : BodySetup.AggGeom.BoxElems)
        {
            AddShapeMetrics(Box, BodyCollisionEnabled, Metrics);
        }
        for (const auto& Capsule : BodySetup.AggGeom.SphylElems)
        {
            AddShapeMetrics(Capsule, BodyCollisionEnabled, Metrics);
        }
        for (const auto& TaperedCapsule : BodySetup.AggGeom.TaperedCapsuleElems)
        {
            AddShapeMetrics(TaperedCapsule, BodyCollisionEnabled, Metrics);
        }
        for (const auto& Convex : BodySetup.AggGeom.ConvexElems)
        {
            const auto PreviousShapeCount = Metrics.ShapeCount;
            AddShapeMetrics(Convex, BodyCollisionEnabled, Metrics);
            if (Metrics.ShapeCount != PreviousShapeCount)
            {
                ++Metrics.ConvexHullCount;
                Metrics.ConvexVertexCount += Convex.VertexData.Num();
            }
        }
        return Metrics;
    }

    bool UsesSimpleCollision(const ECollisionTraceFlag CollisionTraceFlag)
    {
        return CollisionTraceFlag == CTF_UseSimpleAsComplex || CollisionTraceFlag == CTF_UseSimpleAndComplex;
    }

    bool UsesComplexCollision(const ECollisionTraceFlag CollisionTraceFlag)
    {
        return CollisionTraceFlag == CTF_UseComplexAsSimple || CollisionTraceFlag == CTF_UseSimpleAndComplex;
    }

    int64 MeasureComplexTriangleCount(const UStaticMesh& StaticMesh, const UBodySetup& BodySetup)
    {
        if (!StaticMesh.ContainsPhysicsTriMeshData(BodySetup.bMeshCollideAll))
        {
            return 0;
        }

        FTriMeshCollisionDataEstimates Estimates;
        return StaticMesh.GetTriMeshSizeEstimates(Estimates, BodySetup.bMeshCollideAll) ? Estimates.VerticeCount / 3
                                                                                        : 0;
    }
} // namespace RuleRanger::StaticMeshCollisionComplexity

void UEnsureStaticMeshCollisionComplexityWithinLimitsAction::Apply(URuleRangerActionContext* ActionContext,
                                                                   UObject* Object)
{
    const auto StaticMesh = CastChecked<UStaticMesh>(Object);
    if (const auto BodySetup = StaticMesh->GetBodySetup())
    {
        const auto BodyCollisionEnabled = BodySetup->DefaultInstance.GetCollisionEnabled(false);
        if (ECollisionEnabled::NoCollision != BodyCollisionEnabled)
        {
            const auto EffectiveCollisionTraceFlag = BodySetup->GetCollisionTraceFlag();
            const auto EffectiveMaxSimpleShapes = FMath::Max(1, MaxSimpleShapes);
            const auto EffectiveMaxConvexHulls = FMath::Max(1, MaxConvexHulls);
            const auto EffectiveMaxTotalConvexVertices = FMath::Max(1, MaxTotalConvexVertices);
            const auto EffectiveMaxComplexTriangles = FMath::Max(1, MaxComplexTriangles);

            TArray<FString> Breaches;
            RuleRanger::StaticMeshCollisionComplexity::FSimpleCollisionMetrics SimpleMetrics;
            if (RuleRanger::StaticMeshCollisionComplexity::UsesSimpleCollision(EffectiveCollisionTraceFlag))
            {
                SimpleMetrics =
                    RuleRanger::StaticMeshCollisionComplexity::MeasureSimpleCollision(*BodySetup, BodyCollisionEnabled);
                if (SimpleMetrics.ShapeCount > EffectiveMaxSimpleShapes)
                {
                    Breaches.Add(FString::Printf(TEXT("simple shapes: %d > %d"),
                                                 SimpleMetrics.ShapeCount,
                                                 EffectiveMaxSimpleShapes));
                }
                if (SimpleMetrics.ConvexHullCount > EffectiveMaxConvexHulls)
                {
                    Breaches.Add(FString::Printf(TEXT("convex hulls: %d > %d"),
                                                 SimpleMetrics.ConvexHullCount,
                                                 EffectiveMaxConvexHulls));
                }
                if (SimpleMetrics.ConvexVertexCount > EffectiveMaxTotalConvexVertices)
                {
                    Breaches.Add(FString::Printf(TEXT("total convex vertices: %d > %d"),
                                                 SimpleMetrics.ConvexVertexCount,
                                                 EffectiveMaxTotalConvexVertices));
                }
            }

            int64 ComplexTriangleCount{ 0 };
            if (RuleRanger::StaticMeshCollisionComplexity::UsesComplexCollision(EffectiveCollisionTraceFlag))
            {
                ComplexTriangleCount =
                    RuleRanger::StaticMeshCollisionComplexity::MeasureComplexTriangleCount(*StaticMesh, *BodySetup);
                if (ComplexTriangleCount > EffectiveMaxComplexTriangles)
                {
                    Breaches.Add(FString::Printf(TEXT("complex triangles: %lld > %d"),
                                                 ComplexTriangleCount,
                                                 EffectiveMaxComplexTriangles));
                }
            }

            if (Breaches.IsEmpty())
            {
                return;
            }

            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("Breaches"), FText::FromString(FString::Join(Breaches, TEXT("; "))));
            Arguments.Add(TEXT("AuthoredMode"), FText::FromString(LexToString(BodySetup->CollisionTraceFlag)));
            Arguments.Add(TEXT("EffectiveMode"), FText::FromString(LexToString(EffectiveCollisionTraceFlag)));
            Arguments.Add(TEXT("QueryShapes"), SimpleMetrics.QueryShapeCount);
            Arguments.Add(TEXT("PhysicsShapes"), SimpleMetrics.PhysicsShapeCount);
            Arguments.Add(TEXT("ProbeShapes"), SimpleMetrics.ProbeShapeCount);
            Arguments.Add(TEXT("ComplexTriangles"), ComplexTriangleCount);
            ActionContext->Error(FText::Format(
                NSLOCTEXT(
                    "RuleRanger",
                    "EnsureStaticMeshCollisionComplexityWithinLimitsAction_Error",
                    "StaticMesh collision complexity exceeds configured limits ({Breaches}). Authored mode: {AuthoredMode}; effective mode: {EffectiveMode}; active simple-shape channels: query {QueryShapes}, physics {PhysicsShapes}, probe {ProbeShapes}; effective complex triangles: {ComplexTriangles}."),
                Arguments));
        }
    }
}

UClass* UEnsureStaticMeshCollisionComplexityWithinLimitsAction::GetExpectedType() const
{
    return UStaticMesh::StaticClass();
}
