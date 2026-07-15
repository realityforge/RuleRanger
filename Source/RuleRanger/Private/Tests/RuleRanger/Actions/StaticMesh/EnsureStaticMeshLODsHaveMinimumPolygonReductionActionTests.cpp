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
    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/StaticMesh/EnsureStaticMeshLODsHaveMinimumPolygonReductionAction.h"
    #include "StaticMeshResources.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests
{
    UStaticMesh* CreateStaticMesh(const std::initializer_list<uint32> TriangleCounts, const bool bNaniteEnabled = false)
    {
        const auto StaticMesh = RuleRangerTests::NewTransientObject<UStaticMesh>();
        auto RenderData = MakeUnique<FStaticMeshRenderData>();
        RenderData->AllocateLODResources(static_cast<int32>(TriangleCounts.size()));

        int32 LODIndex = 0;
        for (const auto TriangleCount : TriangleCounts)
        {
            RenderData->LODResources[LODIndex++].Sections.AddDefaulted_GetRef().NumTriangles = TriangleCount;
        }

        StaticMesh->SetRenderData(MoveTemp(RenderData));
        auto NaniteSettings = StaticMesh->GetNaniteSettings();
        NaniteSettings.bEnabled = bNaniteEnabled;
        StaticMesh->SetNaniteSettings(NaniteSettings);
        return StaticMesh;
    }

    float GetMinReductionPercentage(FAutomationTestBase& Test,
                                    const UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction* Action)
    {
        const auto Property = FindFProperty<FFloatProperty>(Action->GetClass(), TEXT("MinReductionPercentage"));
        return Test.TestNotNull(TEXT("MinReductionPercentage property should exist"), Property)
            ? Property->GetPropertyValue_InContainer(Action)
            : -1.0f;
    }

    bool Apply(FAutomationTestBase& Test,
               RuleRangerTests::FRuleFixture& Fixture,
               UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction* Action,
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
} // namespace RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionDefaultsToThirtyPercentTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.DefaultsToThirtyPercent",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionDefaultsToThirtyPercentTest::RunTest(
    const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    return TestNotNull(TEXT("Action should be created"), Action)
        && TestEqual(
               TEXT("The default minimum reduction should be thirty percent"),
               RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::GetMinReductionPercentage(*this,
                                                                                                               Action),
               30.0f);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionIgnoresMeshesWithoutTransitionsTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.IgnoresMeshesWithoutTransitions",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionIgnoresMeshesWithoutTransitionsTest::RunTest(
    const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::CreateStaticMesh({ 100 });
    return RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::Apply(*this,
                                                                                       Fixture,
                                                                                       Action,
                                                                                       StaticMesh,
                                                                                       TEXT("SingleLODStaticMesh"))
        && TestTrue(TEXT("A mesh with no LOD transition should add no errors"),
                    Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionAcceptsExactMinimumTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.AcceptsExactMinimum",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionAcceptsExactMinimumTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::CreateStaticMesh({ 100, 70 });
    return RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::Apply(*this,
                                                                                       Fixture,
                                                                                       Action,
                                                                                       StaticMesh,
                                                                                       TEXT("ExactReductionStaticMesh"))
        && TestTrue(TEXT("A transition meeting the exact minimum should add no errors"),
                    Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionErrorsBelowMinimumTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.ErrorsBelowMinimum",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionErrorsBelowMinimumTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::CreateStaticMesh({ 100, 71 });
    if (!RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::Apply(*this,
                                                                                     Fixture,
                                                                                     Action,
                                                                                     StaticMesh,
                                                                                     TEXT("BelowReductionStaticMesh")))
    {
        return false;
    }

    const auto& Errors = Fixture.ActionContext->GetErrorMessages();
    return TestEqual(TEXT("A transition below the minimum should add one error"), Errors.Num(), 1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Errors,
                                                  TEXT("The error should report counts and reduction"),
                                                  TEXT("LOD0->LOD1: 100->71 triangles (29.0% reduction"))
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Errors,
                                                  TEXT("The error should report the required reduction"),
                                                  TEXT("requires 30.0% reduction"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionAggregatesFailingTransitionsTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.AggregatesFailingTransitions",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionAggregatesFailingTransitionsTest::RunTest(
    const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::CreateStaticMesh({ 100, 80, 40, 35 });
    if (!RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::Apply(
            *this,
            Fixture,
            Action,
            StaticMesh,
            TEXT("AggregatedReductionStaticMesh")))
    {
        return false;
    }

    const auto& Errors = Fixture.ActionContext->GetErrorMessages();
    return TestEqual(TEXT("Multiple failing transitions should add one error"), Errors.Num(), 1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Errors,
                                                  TEXT("The error should include the first failing transition"),
                                                  TEXT("LOD0->LOD1: 100->80 triangles"))
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Errors,
                                                  TEXT("The error should include the later failing transition"),
                                                  TEXT("LOD2->LOD3: 40->35 triangles"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionReportsPolygonIncreasesTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.ReportsPolygonIncreases",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionReportsPolygonIncreasesTest::RunTest(
    const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::CreateStaticMesh({ 100, 120 });
    if (!RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::Apply(
            *this,
            Fixture,
            Action,
            StaticMesh,
            TEXT("IncreasedPolygonStaticMesh")))
    {
        return false;
    }

    const auto& Errors = Fixture.ActionContext->GetErrorMessages();
    return TestEqual(TEXT("A polygon increase should add one error"), Errors.Num(), 1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Errors,
                                                  TEXT("The error should identify the polygon increase"),
                                                  TEXT("100->120 triangles (20.0% increase"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionErrorsOnZeroTriangleLODTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.ErrorsOnZeroTriangleLOD",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionErrorsOnZeroTriangleLODTest::RunTest(
    const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::CreateStaticMesh({ 100, 0 });
    if (!RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::Apply(*this,
                                                                                     Fixture,
                                                                                     Action,
                                                                                     StaticMesh,
                                                                                     TEXT("ZeroTriangleStaticMesh")))
    {
        return false;
    }

    const auto& Errors = Fixture.ActionContext->GetErrorMessages();
    return TestEqual(TEXT("A zero-triangle LOD should add one error"), Errors.Num(), 1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Errors,
                                                  TEXT("The error should identify the zero-triangle transition"),
                                                  TEXT("100->0 triangles (cannot evaluate reduction"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionClampsLowThresholdTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.ClampsLowThreshold",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionClampsLowThresholdTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::CreateStaticMesh({ 100, 100 });
    return RuleRangerTests::SetPropertyValue(*this, Action, TEXT("MinReductionPercentage"), -10.0f)
        && RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::Apply(*this,
                                                                                       Fixture,
                                                                                       Action,
                                                                                       StaticMesh,
                                                                                       TEXT("LowThresholdStaticMesh"))
        && TestTrue(TEXT("A negative threshold should clamp to zero"),
                    Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionClampsHighThresholdTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.ClampsHighThreshold",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionClampsHighThresholdTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::CreateStaticMesh({ 100, 1 });
    if (!RuleRangerTests::SetPropertyValue(*this, Action, TEXT("MinReductionPercentage"), 150.0f)
        || !RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::Apply(
            *this,
            Fixture,
            Action,
            StaticMesh,
            TEXT("HighThresholdStaticMesh")))
    {
        return false;
    }

    const auto& Errors = Fixture.ActionContext->GetErrorMessages();
    return TestEqual(TEXT("A threshold above one hundred should clamp and add one error"), Errors.Num(), 1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Errors,
                                                  TEXT("The error should report the effective threshold"),
                                                  TEXT("requires 100.0% reduction"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionIgnoresNaniteMeshesTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.IgnoresNaniteMeshes",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionIgnoresNaniteMeshesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::CreateStaticMesh({ 100, 100 }, true);
    return RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::Apply(*this,
                                                                                       Fixture,
                                                                                       Action,
                                                                                       StaticMesh,
                                                                                       TEXT("NaniteStaticMesh"))
        && TestTrue(TEXT("A Nanite mesh should add no errors"), Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionDoesNotMutateMeshTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.DoesNotMutateMesh",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionDoesNotMutateMeshTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    const auto StaticMesh =
        RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::CreateStaticMesh({ 100, 90 });
    if (!TestNotNull(TEXT("Static mesh should be created"), StaticMesh))
    {
        return false;
    }

    const auto OriginalLODCount = StaticMesh->GetNumLODs();
    const auto OriginalLOD0TriangleCount = StaticMesh->GetNumTriangles(0);
    const auto OriginalLOD1TriangleCount = StaticMesh->GetNumTriangles(1);
    const auto OriginalSourceModelCount = StaticMesh->GetNumSourceModels();
    const auto bWasPackageDirty = StaticMesh->GetPackage()->IsDirty();
    return RuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionTests::Apply(
               *this,
               Fixture,
               Action,
               StaticMesh,
               TEXT("NonMutatingReductionStaticMesh"),
               ERuleRangerActionTrigger::AT_Fix)
        && TestEqual(TEXT("Applying the action should not change the LOD count"),
                     StaticMesh->GetNumLODs(),
                     OriginalLODCount)
        && TestEqual(TEXT("Applying the action should not change LOD0 triangles"),
                     StaticMesh->GetNumTriangles(0),
                     OriginalLOD0TriangleCount)
        && TestEqual(TEXT("Applying the action should not change LOD1 triangles"),
                     StaticMesh->GetNumTriangles(1),
                     OriginalLOD1TriangleCount)
        && TestEqual(TEXT("Applying the action should not change source models"),
                     StaticMesh->GetNumSourceModels(),
                     OriginalSourceModelCount)
        && TestEqual(TEXT("Applying the action should not dirty the package"),
                     StaticMesh->GetPackage()->IsDirty(),
                     bWasPackageDirty);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionExpectedTypeIsStaticMeshTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshLODsHaveMinimumPolygonReduction.ExpectedTypeIsStaticMesh",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshLODsHaveMinimumPolygonReductionActionExpectedTypeIsStaticMeshTest::RunTest(
    const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshLODsHaveMinimumPolygonReductionAction>();
    return TestNotNull(TEXT("Action should be created"), Action)
        && TestEqual(TEXT("The expected type should be StaticMesh"),
                     Action->GetExpectedType(),
                     UStaticMesh::StaticClass());
}

#endif
