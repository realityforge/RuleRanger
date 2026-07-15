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
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "Engine/StaticMesh.h"
    #include "Interface_CollisionDataProviderCore.h"
    #include "Misc/AutomationTest.h"
    #include "PhysicsEngine/BodySetup.h"
    #include "PhysicsSettingsCore.h"
    #include "RuleRanger/Actions/StaticMesh/EnsureStaticMeshCollisionComplexityWithinLimitsAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests
{
    constexpr TCHAR EngineCubePath[] = TEXT("/Engine/BasicShapes/Cube.Cube");

    class FScopedDefaultShapeComplexity final
    {
    public:
        explicit FScopedDefaultShapeComplexity(const ECollisionTraceFlag CollisionTraceFlag)
            : Settings(UPhysicsSettingsCore::Get()), OriginalValue(Settings->DefaultShapeComplexity)
        {
            Settings->DefaultShapeComplexity = CollisionTraceFlag;
        }

        ~FScopedDefaultShapeComplexity() { Settings->DefaultShapeComplexity = OriginalValue; }

    private:
        UPhysicsSettingsCore* Settings;
        TEnumAsByte<ECollisionTraceFlag> OriginalValue;
    };

    UStaticMesh* CreateStaticMesh(const ECollisionTraceFlag CollisionTraceFlag = CTF_UseSimpleAsComplex,
                                  const ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::QueryAndPhysics)
    {
        const auto StaticMesh = RuleRangerTests::NewTransientObject<UStaticMesh>();
        const auto BodySetup = NewObject<UBodySetup>(StaticMesh);
        BodySetup->CollisionTraceFlag = CollisionTraceFlag;
        BodySetup->DefaultInstance.SetCollisionEnabled(CollisionEnabled, false);
        StaticMesh->SetBodySetup(BodySetup);
        return StaticMesh;
    }

    UStaticMesh* DuplicateEngineCube(const ECollisionTraceFlag CollisionTraceFlag)
    {
        const auto EngineCube = LoadObject<UStaticMesh>(nullptr, EngineCubePath);
        const auto StaticMesh = EngineCube ? DuplicateObject<UStaticMesh>(EngineCube, GetTransientPackage()) : nullptr;
        if (StaticMesh && StaticMesh->GetBodySetup())
        {
            StaticMesh->GetBodySetup()->CollisionTraceFlag = CollisionTraceFlag;
            StaticMesh->GetBodySetup()->DefaultInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics, false);
        }
        return StaticMesh;
    }

    void AddBoxes(UBodySetup* BodySetup,
                  const int32 Count,
                  const ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::QueryAndPhysics)
    {
        for (int32 Index = 0; Index < Count; ++Index)
        {
            auto& Box = BodySetup->AggGeom.BoxElems.AddDefaulted_GetRef();
            Box.X = 100.0f;
            Box.Y = 100.0f;
            Box.Z = 100.0f;
            Box.SetCollisionEnabled(CollisionEnabled);
        }
    }

    void AddConvexes(UBodySetup* BodySetup,
                     const int32 Count,
                     const int32 VertexCount,
                     const ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::QueryAndPhysics)
    {
        for (int32 Index = 0; Index < Count; ++Index)
        {
            auto& Convex = BodySetup->AggGeom.ConvexElems.AddDefaulted_GetRef();
            Convex.SetCollisionEnabled(CollisionEnabled);
            Convex.VertexData.SetNum(VertexCount);
        }
    }

    bool Apply(FAutomationTestBase& Test,
               RuleRangerTests::FRuleFixture& Fixture,
               UEnsureStaticMeshCollisionComplexityWithinLimitsAction* Action,
               UStaticMesh* StaticMesh,
               const TCHAR* ObjectName,
               const ERuleRangerActionTrigger Trigger = ERuleRangerActionTrigger::AT_Report)
    {
        if (!Test.TestNotNull(TEXT("Action should be created"), Action)
            || !Test.TestNotNull(TEXT("Static mesh should be created"), StaticMesh)
            || !RuleRangerTests::CreateRuleFixture(Test, Fixture, ObjectName, Trigger))
        {
            return false;
        }

        RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh, Trigger);
        Action->Apply(Fixture.ActionContext, StaticMesh);
        return true;
    }

    int64 GetComplexTriangleCount(const UStaticMesh& StaticMesh)
    {
        FTriMeshCollisionDataEstimates Estimates;
        const auto BodySetup = StaticMesh.GetBodySetup();
        return BodySetup && StaticMesh.GetTriMeshSizeEstimates(Estimates, BodySetup->bMeshCollideAll)
            ? Estimates.VerticeCount / 3
            : 0;
    }
} // namespace RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionDefaultsAndTypeTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.DefaultsAndType",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionDefaultsAndTypeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    const auto SimpleProperty = FindFProperty<FIntProperty>(Action->GetClass(), TEXT("MaxSimpleShapes"));
    const auto HullProperty = FindFProperty<FIntProperty>(Action->GetClass(), TEXT("MaxConvexHulls"));
    const auto VertexProperty = FindFProperty<FIntProperty>(Action->GetClass(), TEXT("MaxTotalConvexVertices"));
    const auto TriangleProperty = FindFProperty<FIntProperty>(Action->GetClass(), TEXT("MaxComplexTriangles"));
    return TestEqual(TEXT("The action should expect StaticMesh"), Action->GetExpectedType(), UStaticMesh::StaticClass())
        && TestNotNull(TEXT("MaxSimpleShapes should exist"), SimpleProperty)
        && TestNotNull(TEXT("MaxConvexHulls should exist"), HullProperty)
        && TestNotNull(TEXT("MaxTotalConvexVertices should exist"), VertexProperty)
        && TestNotNull(TEXT("MaxComplexTriangles should exist"), TriangleProperty)
        && TestEqual(TEXT("The simple-shape default should be 16"),
                     SimpleProperty->GetPropertyValue_InContainer(Action),
                     16)
        && TestEqual(TEXT("The convex-hull default should be 8"), HullProperty->GetPropertyValue_InContainer(Action), 8)
        && TestEqual(TEXT("The convex-vertex default should be 256"),
                     VertexProperty->GetPropertyValue_InContainer(Action),
                     256)
        && TestEqual(TEXT("The complex-triangle default should be 5000"),
                     TriangleProperty->GetPropertyValue_InContainer(Action),
                     5000);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionEnforcesSimpleShapeBoundaryTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.EnforcesSimpleShapeBoundary",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionEnforcesSimpleShapeBoundaryTest::RunTest(
    const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::CreateStaticMesh();
    RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::AddBoxes(StaticMesh->GetBodySetup(), 16);
    RuleRangerTests::FRuleFixture Fixture;
    if (!RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::Apply(
            *this,
            Fixture,
            Action,
            StaticMesh,
            TEXT("SimpleShapeBoundaryStaticMesh"))
        || !TestTrue(TEXT("Sixteen shapes should pass"), Fixture.ActionContext->GetErrorMessages().IsEmpty()))
    {
        return false;
    }

    RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::AddBoxes(StaticMesh->GetBodySetup(), 1);
    RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh);
    Action->Apply(Fixture.ActionContext, StaticMesh);
    return TestEqual(TEXT("Seventeen shapes should add one error"), Fixture.ActionContext->GetErrorMessages().Num(), 1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Fixture.ActionContext->GetErrorMessages(),
                                                  TEXT("The error should report the shape boundary"),
                                                  TEXT("simple shapes: 17 > 16"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionCountsConventionalShapeTypesTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.CountsConventionalShapeTypes",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionCountsConventionalShapeTypesTest::RunTest(
    const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::CreateStaticMesh();
    const auto BodySetup = StaticMesh->GetBodySetup();
    BodySetup->AggGeom.SphereElems.AddDefaulted_GetRef().SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BodySetup->AggGeom.BoxElems.AddDefaulted_GetRef().SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
    BodySetup->AggGeom.SphylElems.AddDefaulted_GetRef().SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BodySetup->AggGeom.TaperedCapsuleElems.AddDefaulted_GetRef().SetCollisionEnabled(ECollisionEnabled::ProbeOnly);
    BodySetup->AggGeom.ConvexElems.AddDefaulted_GetRef().SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);

    RuleRangerTests::FRuleFixture Fixture;
    return RuleRangerTests::SetPropertyValue(*this, Action, TEXT("MaxSimpleShapes"), 4)
        && RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::Apply(
               *this,
               Fixture,
               Action,
               StaticMesh,
               TEXT("ConventionalShapeTypesStaticMesh"))
        && TestEqual(TEXT("All five conventional shape types should count"),
                     Fixture.ActionContext->GetErrorMessages().Num(),
                     1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Fixture.ActionContext->GetErrorMessages(),
                                                  TEXT("The diagnostic should include channel counts"),
                                                  TEXT("query 3, physics 2, probe 2"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionExcludesDisabledGeometryTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.ExcludesDisabledGeometry",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionExcludesDisabledGeometryTest::RunTest(
    const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto DisabledMesh = RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::CreateStaticMesh(
        CTF_UseSimpleAsComplex,
        ECollisionEnabled::NoCollision);
    RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::AddBoxes(DisabledMesh->GetBodySetup(), 17);
    if (!RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::Apply(*this,
                                                                                     Fixture,
                                                                                     Action,
                                                                                     DisabledMesh,
                                                                                     TEXT("DisabledBodyStaticMesh"))
        || !TestTrue(TEXT("A disabled body should be skipped"), Fixture.ActionContext->GetErrorMessages().IsEmpty()))
    {
        return false;
    }

    const auto ShapeDisabledMesh =
        RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::CreateStaticMesh();
    RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::AddBoxes(ShapeDisabledMesh->GetBodySetup(),
                                                                                   17,
                                                                                   ECollisionEnabled::NoCollision);
    RuleRangerTests::ResetRuleFixtureObject(Fixture, ShapeDisabledMesh);
    Action->Apply(Fixture.ActionContext, ShapeDisabledMesh);
    if (!TestTrue(TEXT("NoCollision shapes should be excluded"), Fixture.ActionContext->GetErrorMessages().IsEmpty()))
    {
        return false;
    }

    const auto NonIntersectingMesh =
        RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::CreateStaticMesh(
            CTF_UseSimpleAsComplex,
            ECollisionEnabled::QueryOnly);
    RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::AddBoxes(NonIntersectingMesh->GetBodySetup(),
                                                                                   17,
                                                                                   ECollisionEnabled::PhysicsOnly);
    RuleRangerTests::ResetRuleFixtureObject(Fixture, NonIntersectingMesh);
    Action->Apply(Fixture.ActionContext, NonIntersectingMesh);
    return TestTrue(TEXT("Shapes disabled by body-shape intersection should be excluded"),
                    Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionEnforcesConvexBoundariesTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.EnforcesConvexBoundaries",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionEnforcesConvexBoundariesTest::RunTest(
    const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::CreateStaticMesh();
    RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::AddConvexes(StaticMesh->GetBodySetup(),
                                                                                      8,
                                                                                      32);
    RuleRangerTests::FRuleFixture Fixture;
    if (!RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::Apply(*this,
                                                                                     Fixture,
                                                                                     Action,
                                                                                     StaticMesh,
                                                                                     TEXT("ConvexBoundaryStaticMesh"))
        || !TestTrue(TEXT("Eight hulls and 256 vertices should pass"),
                     Fixture.ActionContext->GetErrorMessages().IsEmpty()))
    {
        return false;
    }

    RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::AddConvexes(StaticMesh->GetBodySetup(), 1, 1);
    RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh);
    Action->Apply(Fixture.ActionContext, StaticMesh);
    return TestEqual(TEXT("Multiple convex breaches should aggregate into one error"),
                     Fixture.ActionContext->GetErrorMessages().Num(),
                     1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Fixture.ActionContext->GetErrorMessages(),
                                                  TEXT("The hull breach should be reported"),
                                                  TEXT("convex hulls: 9 > 8"))
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Fixture.ActionContext->GetErrorMessages(),
                                                  TEXT("The vertex breach should be reported"),
                                                  TEXT("total convex vertices: 257 > 256"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionIgnoresConvexIndicesTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.IgnoresConvexIndices",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionIgnoresConvexIndicesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::CreateStaticMesh();
    auto& Convex = StaticMesh->GetBodySetup()->AggGeom.ConvexElems.AddDefaulted_GetRef();
    Convex.VertexData.SetNum(1);
    Convex.IndexData.SetNum(1000);
    RuleRangerTests::FRuleFixture Fixture;
    return RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::Apply(
               *this,
               Fixture,
               Action,
               StaticMesh,
               TEXT("ConvexIndexDataStaticMesh"))
        && TestTrue(TEXT("IndexData should not affect convex complexity"),
                    Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionFollowsEffectiveTraceModeTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.FollowsEffectiveTraceMode",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionFollowsEffectiveTraceModeTest::RunTest(
    const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::DuplicateEngineCube(
        CTF_UseComplexAsSimple);
    if (!TestNotNull(TEXT("The engine cube should be duplicated"), StaticMesh))
    {
        return false;
    }
    RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::AddBoxes(StaticMesh->GetBodySetup(), 17);
    RuleRangerTests::FRuleFixture Fixture;
    if (!RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::Apply(
            *this,
            Fixture,
            Action,
            StaticMesh,
            TEXT("ComplexModeIgnoresSimpleStaticMesh"))
        || !TestTrue(TEXT("ComplexAsSimple should ignore excessive simple geometry"),
                     Fixture.ActionContext->GetErrorMessages().IsEmpty()))
    {
        return false;
    }

    StaticMesh->GetBodySetup()->CollisionTraceFlag = CTF_UseSimpleAndComplex;
    RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh);
    Action->Apply(Fixture.ActionContext, StaticMesh);
    return TestEqual(TEXT("SimpleAndComplex should evaluate simple geometry"),
                     Fixture.ActionContext->GetErrorMessages().Num(),
                     1);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionUsesProjectDefaultTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.UsesProjectDefault",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionUsesProjectDefaultTest::RunTest(const FString&)
{
    const RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::FScopedDefaultShapeComplexity
        ScopedDefault(CTF_UseSimpleAsComplex);
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::CreateStaticMesh(CTF_UseDefault);
    RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::AddBoxes(StaticMesh->GetBodySetup(), 17);
    RuleRangerTests::FRuleFixture Fixture;
    return RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::Apply(*this,
                                                                                       Fixture,
                                                                                       Action,
                                                                                       StaticMesh,
                                                                                       TEXT("ProjectDefaultStaticMesh"))
        && TestEqual(TEXT("The resolved project default should be enforced"),
                     Fixture.ActionContext->GetErrorMessages().Num(),
                     1)
        && RuleRangerTests::TestTextArrayContains(
               *this,
               Fixture.ActionContext->GetErrorMessages(),
               TEXT("The diagnostic should name authored and effective modes"),
               TEXT("Authored mode: CTF_UseDefault; effective mode: CTF_UseSimpleAsComplex"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionEnforcesComplexTriangleBoundaryTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.EnforcesComplexTriangleBoundary",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionEnforcesComplexTriangleBoundaryTest::RunTest(
    const FString&)
{
    const auto StaticMesh = RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::DuplicateEngineCube(
        CTF_UseComplexAsSimple);
    if (!TestNotNull(TEXT("The engine cube should be duplicated"), StaticMesh))
    {
        return false;
    }
    const auto TriangleCount =
        RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::GetComplexTriangleCount(*StaticMesh);
    if (!TestTrue(TEXT("The engine cube should have triangle collision"), TriangleCount > 1))
    {
        return false;
    }

    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    RuleRangerTests::FRuleFixture Fixture;
    if (!RuleRangerTests::SetPropertyValue(*this,
                                           Action,
                                           TEXT("MaxComplexTriangles"),
                                           static_cast<int32>(TriangleCount))
        || !RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::Apply(
            *this,
            Fixture,
            Action,
            StaticMesh,
            TEXT("ComplexTriangleBoundaryStaticMesh"))
        || !TestTrue(TEXT("The exact triangle limit should pass"), Fixture.ActionContext->GetErrorMessages().IsEmpty()))
    {
        return false;
    }

    if (!RuleRangerTests::SetPropertyValue(*this,
                                           Action,
                                           TEXT("MaxComplexTriangles"),
                                           static_cast<int32>(TriangleCount - 1)))
    {
        return false;
    }
    RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh);
    Action->Apply(Fixture.ActionContext, StaticMesh);
    const auto ExpectedBreach =
        FString::Printf(TEXT("complex triangles: %lld > %lld"), TriangleCount, TriangleCount - 1);
    return TestEqual(TEXT("One triangle above the limit should add one error"),
                     Fixture.ActionContext->GetErrorMessages().Num(),
                     1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Fixture.ActionContext->GetErrorMessages(),
                                                  TEXT("The complex triangle breach should be reported"),
                                                  *ExpectedBreach);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionMeasuresCustomComplexCollisionTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.MeasuresCustomComplexCollision",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionMeasuresCustomComplexCollisionTest::RunTest(
    const FString&)
{
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::CreateStaticMesh(CTF_UseComplexAsSimple);
    StaticMesh->ComplexCollisionMesh =
        LoadObject<UStaticMesh>(nullptr,
                                RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::EngineCubePath);
    const auto TriangleCount =
        RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::GetComplexTriangleCount(*StaticMesh);
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    RuleRangerTests::FRuleFixture Fixture;
    return TestTrue(TEXT("The custom collision mesh should supply triangles"), TriangleCount > 1)
        && RuleRangerTests::SetPropertyValue(*this,
                                             Action,
                                             TEXT("MaxComplexTriangles"),
                                             static_cast<int32>(TriangleCount - 1))
        && RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::Apply(
               *this,
               Fixture,
               Action,
               StaticMesh,
               TEXT("CustomComplexCollisionStaticMesh"))
        && TestEqual(TEXT("Custom complex collision should be budgeted"),
                     Fixture.ActionContext->GetErrorMessages().Num(),
                     1);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionDelegatesMissingCollisionTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.DelegatesMissingCollision",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionDelegatesMissingCollisionTest::RunTest(
    const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto MissingBodySetup = RuleRangerTests::NewTransientObject<UStaticMesh>();
    if (!RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::Apply(*this,
                                                                                     Fixture,
                                                                                     Action,
                                                                                     MissingBodySetup,
                                                                                     TEXT("MissingBodySetupStaticMesh"))
        || !TestTrue(TEXT("A missing BodySetup should be delegated"),
                     Fixture.ActionContext->GetErrorMessages().IsEmpty()))
    {
        return false;
    }

    const auto WrongRepresentation =
        RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::CreateStaticMesh(CTF_UseComplexAsSimple);
    RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::AddBoxes(WrongRepresentation->GetBodySetup(),
                                                                                   17);
    RuleRangerTests::ResetRuleFixtureObject(Fixture, WrongRepresentation);
    Action->Apply(Fixture.ActionContext, WrongRepresentation);
    return TestTrue(TEXT("Wrong collision representation should be delegated"),
                    Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionDoesNotMutateInFixModeTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshCollisionComplexityWithinLimits.DoesNotMutateInFixMode",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionDoesNotMutateInFixModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshCollisionComplexityWithinLimitsAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::CreateStaticMesh();
    RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::AddBoxes(StaticMesh->GetBodySetup(), 17);
    const auto OriginalElementCount = StaticMesh->GetBodySetup()->AggGeom.GetElementCount();
    const auto OriginalTraceFlag = StaticMesh->GetBodySetup()->CollisionTraceFlag;
    const auto bWasPackageDirty = StaticMesh->GetPackage()->IsDirty();
    RuleRangerTests::FRuleFixture Fixture;
    return RuleRangerEnsureStaticMeshCollisionComplexityWithinLimitsActionTests::Apply(*this,
                                                                                       Fixture,
                                                                                       Action,
                                                                                       StaticMesh,
                                                                                       TEXT("FixModeStaticMesh"),
                                                                                       ERuleRangerActionTrigger::AT_Fix)
        && TestEqual(TEXT("Fix mode should report the breach"), Fixture.ActionContext->GetErrorMessages().Num(), 1)
        && TestEqual(TEXT("Fix mode should preserve collision elements"),
                     StaticMesh->GetBodySetup()->AggGeom.GetElementCount(),
                     OriginalElementCount)
        && TestEqual(TEXT("Fix mode should preserve the trace flag"),
                     StaticMesh->GetBodySetup()->CollisionTraceFlag,
                     OriginalTraceFlag)
        && TestEqual(TEXT("Fix mode should preserve package dirty state"),
                     StaticMesh->GetPackage()->IsDirty(),
                     bWasPackageDirty);
}

#endif
