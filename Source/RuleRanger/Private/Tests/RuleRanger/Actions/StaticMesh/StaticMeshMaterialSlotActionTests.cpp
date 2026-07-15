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
    #include "Materials/Material.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/StaticMesh/EnsureStaticMeshHasNoUnusedMaterialSlotsAction.h"
    #include "RuleRanger/Actions/StaticMesh/EnsureStaticMeshMaterialSlotCountWithinLimitAction.h"
    #include "RuleRanger/Actions/StaticMesh/EnsureStaticMeshSectionsHaveValidMaterialsAction.h"
    #include "StaticMeshResources.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerStaticMeshMaterialSlotActionTests
{
    UStaticMesh* CreateStaticMesh(const int32 MaterialSlotCount,
                                  const std::initializer_list<std::initializer_list<int32>> LODMaterialIndices = {},
                                  const TSet<int32>& NullMaterialIndices = {})
    {
        const auto StaticMesh = RuleRangerTests::NewTransientObject<UStaticMesh>();
        TArray<FStaticMaterial> StaticMaterials;
        for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
        {
            const auto SlotName = FName(*FString::Printf(TEXT("Slot%d"), MaterialIndex));
            StaticMaterials.Add(FStaticMaterial(
                NullMaterialIndices.Contains(MaterialIndex) ? nullptr : UMaterial::GetDefaultMaterial(MD_Surface),
                SlotName,
                SlotName));
        }
        StaticMesh->SetStaticMaterials(StaticMaterials);

        if (LODMaterialIndices.size() > 0)
        {
            auto RenderData = MakeUnique<FStaticMeshRenderData>();
            RenderData->AllocateLODResources(static_cast<int32>(LODMaterialIndices.size()));
            int32 LODIndex = 0;
            for (const auto& MaterialIndices : LODMaterialIndices)
            {
                auto& Sections = RenderData->LODResources[LODIndex++].Sections;
                for (const auto MaterialIndex : MaterialIndices)
                {
                    auto& Section = Sections.AddDefaulted_GetRef();
                    Section.MaterialIndex = MaterialIndex;
                    Section.NumTriangles = 1;
                }
            }
            StaticMesh->SetRenderData(MoveTemp(RenderData));
        }

        return StaticMesh;
    }

    template <typename TAction>
    bool Apply(FAutomationTestBase& Test,
               RuleRangerTests::FRuleFixture& Fixture,
               TAction* Action,
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
} // namespace RuleRangerStaticMeshMaterialSlotActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerStaticMeshMaterialSlotActionsHaveExpectedTypeTest,
                                 "RuleRanger.Actions.StaticMesh.MaterialSlots.HaveExpectedType",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerStaticMeshMaterialSlotActionsHaveExpectedTypeTest::RunTest(const FString&)
{
    const auto SectionsAction =
        RuleRangerTests::NewTransientObject<UEnsureStaticMeshSectionsHaveValidMaterialsAction>();
    const auto CountAction = RuleRangerTests::NewTransientObject<UEnsureStaticMeshMaterialSlotCountWithinLimitAction>();
    const auto UnusedAction = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasNoUnusedMaterialSlotsAction>();
    return TestEqual(TEXT("The section action should expect StaticMesh"),
                     SectionsAction->GetExpectedType(),
                     UStaticMesh::StaticClass())
        && TestEqual(TEXT("The count action should expect StaticMesh"),
                     CountAction->GetExpectedType(),
                     UStaticMesh::StaticClass())
        && TestEqual(TEXT("The unused action should expect StaticMesh"),
                     UnusedAction->GetExpectedType(),
                     UStaticMesh::StaticClass());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshMaterialSlotCountWithinLimitDefaultsToFourTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshMaterialSlotCountWithinLimit.DefaultsToFour",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshMaterialSlotCountWithinLimitDefaultsToFourTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshMaterialSlotCountWithinLimitAction>();
    const auto Property = FindFProperty<FIntProperty>(Action->GetClass(), TEXT("MaxMaterialSlots"));
    return TestNotNull(TEXT("MaxMaterialSlots should exist"), Property)
        && TestEqual(TEXT("The default maximum should be four"), Property->GetPropertyValue_InContainer(Action), 4);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshMaterialSlotCountWithinLimitAcceptsBoundaryTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshMaterialSlotCountWithinLimit.AcceptsBoundary",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshMaterialSlotCountWithinLimitAcceptsBoundaryTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshMaterialSlotCountWithinLimitAction>();
    const auto StaticMesh = RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(4);
    return RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                              Fixture,
                                                              Action,
                                                              StaticMesh,
                                                              TEXT("BoundaryMaterialSlotStaticMesh"))
        && TestTrue(TEXT("Four material slots should pass"), Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshMaterialSlotCountWithinLimitErrorsAboveBoundaryWithoutRenderDataTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshMaterialSlotCountWithinLimit.ErrorsAboveBoundaryWithoutRenderData",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshMaterialSlotCountWithinLimitErrorsAboveBoundaryWithoutRenderDataTest::RunTest(
    const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshMaterialSlotCountWithinLimitAction>();
    const auto StaticMesh = RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(5);
    if (!RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                            Fixture,
                                                            Action,
                                                            StaticMesh,
                                                            TEXT("ExcessMaterialSlotStaticMesh")))
    {
        return false;
    }

    const auto& Errors = Fixture.ActionContext->GetErrorMessages();
    return TestEqual(TEXT("Five material slots should add one error"), Errors.Num(), 1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Errors,
                                                  TEXT("The error should report the actual and allowed counts"),
                                                  TEXT("5 material slot(s)"))
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Errors,
                                                  TEXT("The error should report the configured maximum"),
                                                  TEXT("maximum of 4"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshMaterialSlotCountWithinLimitUsesConfiguredLimitTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshMaterialSlotCountWithinLimit.UsesConfiguredLimit",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshMaterialSlotCountWithinLimitUsesConfiguredLimitTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshMaterialSlotCountWithinLimitAction>();
    const auto StaticMesh = RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(5);
    return RuleRangerTests::SetPropertyValue(*this, Action, TEXT("MaxMaterialSlots"), 5)
        && RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                              Fixture,
                                                              Action,
                                                              StaticMesh,
                                                              TEXT("ConfiguredMaterialSlotLimitStaticMesh"))
        && TestTrue(TEXT("A mesh at the configured maximum should pass"),
                    Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshSectionsHaveValidMaterialsAcceptsValidAssignmentsAcrossLODsTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshSectionsHaveValidMaterials.AcceptsValidAssignmentsAcrossLODs",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshSectionsHaveValidMaterialsAcceptsValidAssignmentsAcrossLODsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshSectionsHaveValidMaterialsAction>();
    const auto StaticMesh = RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(2, { { 0 }, { 1 } });
    return RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                              Fixture,
                                                              Action,
                                                              StaticMesh,
                                                              TEXT("ValidMaterialAssignmentsStaticMesh"))
        && TestTrue(TEXT("Valid assignments across LODs should pass"),
                    Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshSectionsHaveValidMaterialsAggregatesNullAndOutOfRangeAssignmentsTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshSectionsHaveValidMaterials.AggregatesNullAndOutOfRangeAssignments",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshSectionsHaveValidMaterialsAggregatesNullAndOutOfRangeAssignmentsTest::RunTest(
    const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshSectionsHaveValidMaterialsAction>();
    const auto StaticMesh =
        RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(2, { { 0, 1 }, { 5 } }, { 1 });
    if (!RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                            Fixture,
                                                            Action,
                                                            StaticMesh,
                                                            TEXT("InvalidMaterialAssignmentsStaticMesh")))
    {
        return false;
    }

    const auto& Errors = Fixture.ActionContext->GetErrorMessages();
    return TestEqual(TEXT("Multiple invalid assignments should add one error"), Errors.Num(), 1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Errors,
                                                  TEXT("The error should identify the used null slot"),
                                                  TEXT("LOD0 Section1: material slot 1"))
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Errors,
                                                  TEXT("The error should identify the out-of-range index"),
                                                  TEXT("LOD1 Section0: material index 5"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshSectionsHaveValidMaterialsIgnoresUnusedNullSlotTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshSectionsHaveValidMaterials.IgnoresUnusedNullSlot",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshSectionsHaveValidMaterialsIgnoresUnusedNullSlotTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshSectionsHaveValidMaterialsAction>();
    const auto StaticMesh = RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(2, { { 0 } }, { 1 });
    return RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                              Fixture,
                                                              Action,
                                                              StaticMesh,
                                                              TEXT("UnusedNullMaterialSlotStaticMesh"))
        && TestTrue(TEXT("An unused null slot should not be an assignment error"),
                    Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshSectionsHaveValidMaterialsIgnoresMissingRenderDataTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshSectionsHaveValidMaterials.IgnoresMissingRenderData",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshSectionsHaveValidMaterialsIgnoresMissingRenderDataTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshSectionsHaveValidMaterialsAction>();
    const auto StaticMesh = RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(1);
    return RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                              Fixture,
                                                              Action,
                                                              StaticMesh,
                                                              TEXT("MissingRenderDataAssignmentStaticMesh"))
        && TestTrue(TEXT("Missing render data should add no assignment errors"),
                    Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasNoUnusedMaterialSlotsAcceptsUsageInLaterLODTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasNoUnusedMaterialSlots.AcceptsUsageInLaterLOD",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasNoUnusedMaterialSlotsAcceptsUsageInLaterLODTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasNoUnusedMaterialSlotsAction>();
    const auto StaticMesh = RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(2, { { 0 }, { 1 } });
    return RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                              Fixture,
                                                              Action,
                                                              StaticMesh,
                                                              TEXT("LaterLODMaterialUsageStaticMesh"))
        && TestTrue(TEXT("A slot used only in a later LOD should pass"),
                    Fixture.ActionContext->GetWarningMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasNoUnusedMaterialSlotsWarnsForNullAndNonNullSlotsTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasNoUnusedMaterialSlots.WarnsForNullAndNonNullSlots",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasNoUnusedMaterialSlotsWarnsForNullAndNonNullSlotsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasNoUnusedMaterialSlotsAction>();
    const auto StaticMesh = RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(3, { { 0 } }, { 2 });
    if (!RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                            Fixture,
                                                            Action,
                                                            StaticMesh,
                                                            TEXT("UnusedMaterialSlotsStaticMesh")))
    {
        return false;
    }

    const auto& Warnings = Fixture.ActionContext->GetWarningMessages();
    return TestEqual(TEXT("Unused null and non-null slots should add one warning"), Warnings.Num(), 1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Warnings,
                                                  TEXT("The warning should identify the non-null unused slot"),
                                                  TEXT("slot 1 ('Slot1')"))
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Warnings,
                                                  TEXT("The warning should identify the null unused slot"),
                                                  TEXT("slot 2 ('Slot2')"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasNoUnusedMaterialSlotsIgnoresMissingRenderDataTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasNoUnusedMaterialSlots.IgnoresMissingRenderData",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasNoUnusedMaterialSlotsIgnoresMissingRenderDataTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasNoUnusedMaterialSlotsAction>();
    const auto StaticMesh = RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(2);
    return RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                              Fixture,
                                                              Action,
                                                              StaticMesh,
                                                              TEXT("MissingRenderDataUnusedSlotStaticMesh"))
        && TestTrue(TEXT("Missing render data should add no unused-slot warnings"),
                    Fixture.ActionContext->GetWarningMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerStaticMeshMaterialSlotActionsIncludeNaniteMeshesTest,
                                 "RuleRanger.Actions.StaticMesh.MaterialSlots.IncludeNaniteMeshes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerStaticMeshMaterialSlotActionsIncludeNaniteMeshesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto StaticMesh = RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(2, { { 0 } }, { 0 });
    auto NaniteSettings = StaticMesh->GetNaniteSettings();
    NaniteSettings.bEnabled = true;
    StaticMesh->SetNaniteSettings(NaniteSettings);

    const auto SectionsAction =
        RuleRangerTests::NewTransientObject<UEnsureStaticMeshSectionsHaveValidMaterialsAction>();
    if (!RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                            Fixture,
                                                            SectionsAction,
                                                            StaticMesh,
                                                            TEXT("NaniteMaterialSlotStaticMesh"))
        || !TestEqual(TEXT("A Nanite mesh should still report its used null material"),
                      Fixture.ActionContext->GetErrorMessages().Num(),
                      1))
    {
        return false;
    }

    const auto UnusedAction = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasNoUnusedMaterialSlotsAction>();
    RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh);
    UnusedAction->Apply(Fixture.ActionContext, StaticMesh);
    return TestEqual(TEXT("A Nanite mesh should still report its unused slot"),
                     Fixture.ActionContext->GetWarningMessages().Num(),
                     1);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerStaticMeshMaterialSlotActionsDoNotMutateInFixModeTest,
                                 "RuleRanger.Actions.StaticMesh.MaterialSlots.DoNotMutateInFixMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerStaticMeshMaterialSlotActionsDoNotMutateInFixModeTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto StaticMesh =
        RuleRangerStaticMeshMaterialSlotActionTests::CreateStaticMesh(5, { { 0, 1 }, { 4 } }, { 1 });
    const auto OriginalMaterials = StaticMesh->GetStaticMaterials();
    const auto OriginalLOD0MaterialIndex = StaticMesh->GetRenderData()->LODResources[0].Sections[1].MaterialIndex;
    const auto OriginalLOD1MaterialIndex = StaticMesh->GetRenderData()->LODResources[1].Sections[0].MaterialIndex;

    const auto SectionsAction =
        RuleRangerTests::NewTransientObject<UEnsureStaticMeshSectionsHaveValidMaterialsAction>();
    const auto CountAction = RuleRangerTests::NewTransientObject<UEnsureStaticMeshMaterialSlotCountWithinLimitAction>();
    const auto UnusedAction = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasNoUnusedMaterialSlotsAction>();
    if (!RuleRangerStaticMeshMaterialSlotActionTests::Apply(*this,
                                                            Fixture,
                                                            SectionsAction,
                                                            StaticMesh,
                                                            TEXT("FixModeMaterialSlotStaticMesh"),
                                                            ERuleRangerActionTrigger::AT_Fix))
    {
        return false;
    }
    RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh, ERuleRangerActionTrigger::AT_Fix);
    CountAction->Apply(Fixture.ActionContext, StaticMesh);
    RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh, ERuleRangerActionTrigger::AT_Fix);
    UnusedAction->Apply(Fixture.ActionContext, StaticMesh);

    const auto& CurrentMaterials = StaticMesh->GetStaticMaterials();
    if (!TestEqual(TEXT("Fix mode should preserve the material slot count"),
                   CurrentMaterials.Num(),
                   OriginalMaterials.Num()))
    {
        return false;
    }
    for (int32 MaterialIndex = 0; MaterialIndex < CurrentMaterials.Num(); ++MaterialIndex)
    {
        if (!TestEqual(TEXT("Fix mode should preserve material assignments"),
                       CurrentMaterials[MaterialIndex].MaterialInterface,
                       OriginalMaterials[MaterialIndex].MaterialInterface)
            || !TestEqual(TEXT("Fix mode should preserve material slot names"),
                          CurrentMaterials[MaterialIndex].MaterialSlotName,
                          OriginalMaterials[MaterialIndex].MaterialSlotName))
        {
            return false;
        }
    }

    return TestEqual(TEXT("Fix mode should preserve the LOD0 section material index"),
                     StaticMesh->GetRenderData()->LODResources[0].Sections[1].MaterialIndex,
                     OriginalLOD0MaterialIndex)
        && TestEqual(TEXT("Fix mode should preserve the LOD1 section material index"),
                     StaticMesh->GetRenderData()->LODResources[1].Sections[0].MaterialIndex,
                     OriginalLOD1MaterialIndex);
}

#endif
