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
    #include "PhysicsEngine/BodySetup.h"
    #include "PhysicsEngine/BoxElem.h"
    #include "PhysicsSettingsCore.h"
    #include "RuleRanger/Actions/StaticMesh/EnsureStaticMeshHasUsableCollisionAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureStaticMeshHasUsableCollisionActionTests
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

    UStaticMesh* CreateStaticMesh()
    {
        return RuleRangerTests::NewTransientObject<UStaticMesh>();
    }

    UStaticMesh* DuplicateEngineCube()
    {
        const auto EngineCube = LoadObject<UStaticMesh>(nullptr, EngineCubePath);
        return EngineCube ? DuplicateObject<UStaticMesh>(EngineCube, GetTransientPackage()) : nullptr;
    }

    UBodySetup* AddBodySetup(UStaticMesh* StaticMesh,
                             const ECollisionTraceFlag CollisionTraceFlag,
                             const ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::QueryAndPhysics)
    {
        const auto BodySetup = NewObject<UBodySetup>(StaticMesh);
        BodySetup->CollisionTraceFlag = CollisionTraceFlag;
        BodySetup->DefaultInstance.SetCollisionEnabled(CollisionEnabled, false);
        StaticMesh->SetBodySetup(BodySetup);
        return BodySetup;
    }

    void AddBox(UBodySetup* BodySetup,
                const ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::QueryAndPhysics)
    {
        FKBoxElem Box;
        Box.X = 100.0f;
        Box.Y = 100.0f;
        Box.Z = 100.0f;
        Box.SetCollisionEnabled(CollisionEnabled);
        BodySetup->AggGeom.BoxElems.Add(Box);
    }

    bool Apply(FAutomationTestBase& Test,
               RuleRangerTests::FRuleFixture& Fixture,
               UEnsureStaticMeshHasUsableCollisionAction* Action,
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
} // namespace RuleRangerEnsureStaticMeshHasUsableCollisionActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasUsableCollisionActionErrorsWithoutBodySetupTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasUsableCollision.ErrorsWithoutBodySetup",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasUsableCollisionActionErrorsWithoutBodySetupTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasUsableCollisionAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::CreateStaticMesh();
    return RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::Apply(*this,
                                                                          Fixture,
                                                                          Action,
                                                                          StaticMesh,
                                                                          TEXT("MissingBodySetupStaticMesh"))
        && TestEqual(TEXT("A mesh without a BodySetup should add one error"),
                     Fixture.ActionContext->GetErrorMessages().Num(),
                     1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Fixture.ActionContext->GetErrorMessages(),
                                                  TEXT("The error should identify the missing BodySetup"),
                                                  TEXT("has no BodySetup"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasUsableCollisionActionAcceptsSimpleCollisionTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasUsableCollision.AcceptsSimpleCollision",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasUsableCollisionActionAcceptsSimpleCollisionTest::RunTest(const FString&)
{
    for (const auto CollisionTraceFlag : { CTF_UseSimpleAsComplex, CTF_UseSimpleAndComplex })
    {
        RuleRangerTests::FRuleFixture Fixture;
        const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasUsableCollisionAction>();
        const auto StaticMesh = RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::CreateStaticMesh();
        const auto BodySetup =
            RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::AddBodySetup(StaticMesh, CollisionTraceFlag);
        RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::AddBox(BodySetup);
        if (!RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::Apply(*this,
                                                                            Fixture,
                                                                            Action,
                                                                            StaticMesh,
                                                                            TEXT("SimpleCollisionStaticMesh"))
            || !TestTrue(TEXT("Enabled simple collision should be accepted"),
                         Fixture.ActionContext->GetErrorMessages().IsEmpty()))
        {
            return false;
        }
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasUsableCollisionActionRejectsDisabledSimpleCollisionTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasUsableCollision.RejectsDisabledSimpleCollision",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasUsableCollisionActionRejectsDisabledSimpleCollisionTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasUsableCollisionAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::CreateStaticMesh();
    const auto BodySetup =
        RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::AddBodySetup(StaticMesh, CTF_UseSimpleAsComplex);
    RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::AddBox(BodySetup, ECollisionEnabled::NoCollision);
    return RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::Apply(*this,
                                                                          Fixture,
                                                                          Action,
                                                                          StaticMesh,
                                                                          TEXT("DisabledSimpleCollisionStaticMesh"))
        && TestEqual(TEXT("A disabled simple shape should not count as usable collision"),
                     Fixture.ActionContext->GetErrorMessages().Num(),
                     1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Fixture.ActionContext->GetErrorMessages(),
                                                  TEXT("The error should identify missing simple collision"),
                                                  TEXT("no enabled aggregate geometry"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasUsableCollisionActionAcceptsRenderComplexCollisionTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasUsableCollision.AcceptsRenderComplexCollision",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasUsableCollisionActionAcceptsRenderComplexCollisionTest::RunTest(const FString&)
{
    for (const auto CollisionTraceFlag : { CTF_UseComplexAsSimple, CTF_UseSimpleAndComplex })
    {
        RuleRangerTests::FRuleFixture Fixture;
        const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasUsableCollisionAction>();
        const auto StaticMesh = RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::DuplicateEngineCube();
        if (!TestNotNull(TEXT("The duplicated engine cube should have a BodySetup"),
                         StaticMesh ? StaticMesh->GetBodySetup() : nullptr))
        {
            return false;
        }
        StaticMesh->GetBodySetup()->CollisionTraceFlag = CollisionTraceFlag;
        StaticMesh->GetBodySetup()->AggGeom.EmptyElements();
        StaticMesh->GetBodySetup()->DefaultInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics, false);
        if (!RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::Apply(*this,
                                                                            Fixture,
                                                                            Action,
                                                                            StaticMesh,
                                                                            TEXT("RenderComplexCollisionStaticMesh"))
            || !TestTrue(TEXT("Valid render-complex collision should be accepted"),
                         Fixture.ActionContext->GetErrorMessages().IsEmpty()))
        {
            return false;
        }
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasUsableCollisionActionAcceptsCustomComplexCollisionTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasUsableCollision.AcceptsCustomComplexCollision",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasUsableCollisionActionAcceptsCustomComplexCollisionTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasUsableCollisionAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::CreateStaticMesh();
    const auto ComplexCollisionMesh =
        LoadObject<UStaticMesh>(nullptr, RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::EngineCubePath);
    RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::AddBodySetup(StaticMesh, CTF_UseComplexAsSimple);
    StaticMesh->ComplexCollisionMesh = ComplexCollisionMesh;
    return TestNotNull(TEXT("The engine cube should load as custom complex collision"), ComplexCollisionMesh)
        && RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::Apply(*this,
                                                                          Fixture,
                                                                          Action,
                                                                          StaticMesh,
                                                                          TEXT("CustomComplexCollisionStaticMesh"))
        && TestTrue(TEXT("Valid custom complex collision should be accepted"),
                    Fixture.ActionContext->GetErrorMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasUsableCollisionActionRejectsWrongRepresentationTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasUsableCollision.RejectsWrongRepresentation",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasUsableCollisionActionRejectsWrongRepresentationTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture SimpleFixture;
    const auto SimpleAction = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasUsableCollisionAction>();
    const auto SimpleMesh = RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::DuplicateEngineCube();
    if (!TestNotNull(TEXT("The duplicated engine cube should have a BodySetup"),
                     SimpleMesh ? SimpleMesh->GetBodySetup() : nullptr))
    {
        return false;
    }
    SimpleMesh->GetBodySetup()->CollisionTraceFlag = CTF_UseSimpleAsComplex;
    SimpleMesh->GetBodySetup()->AggGeom.EmptyElements();
    SimpleMesh->GetBodySetup()->DefaultInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics, false);

    RuleRangerTests::FRuleFixture ComplexFixture;
    const auto ComplexAction = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasUsableCollisionAction>();
    const auto ComplexMesh = RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::CreateStaticMesh();
    const auto ComplexBodySetup =
        RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::AddBodySetup(ComplexMesh, CTF_UseComplexAsSimple);
    RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::AddBox(ComplexBodySetup);

    return RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::Apply(*this,
                                                                          SimpleFixture,
                                                                          SimpleAction,
                                                                          SimpleMesh,
                                                                          TEXT("ComplexOnlySimpleModeStaticMesh"))
        && TestEqual(TEXT("SimpleAsComplex should reject complex-only geometry"),
                     SimpleFixture.ActionContext->GetErrorMessages().Num(),
                     1)
        && RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::Apply(*this,
                                                                          ComplexFixture,
                                                                          ComplexAction,
                                                                          ComplexMesh,
                                                                          TEXT("SimpleOnlyComplexModeStaticMesh"))
        && TestEqual(TEXT("ComplexAsSimple should reject simple-only geometry"),
                     ComplexFixture.ActionContext->GetErrorMessages().Num(),
                     1);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureStaticMeshHasUsableCollisionActionUsesProjectDefaultTest,
                                 "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasUsableCollision.UsesProjectDefault",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasUsableCollisionActionUsesProjectDefaultTest::RunTest(const FString&)
{
    const RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::FScopedDefaultShapeComplexity ScopedDefault(
        CTF_UseComplexAsSimple);
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasUsableCollisionAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::CreateStaticMesh();
    const auto BodySetup =
        RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::AddBodySetup(StaticMesh, CTF_UseDefault);
    RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::AddBox(BodySetup);
    return RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::Apply(*this,
                                                                          Fixture,
                                                                          Action,
                                                                          StaticMesh,
                                                                          TEXT("ProjectDefaultCollisionStaticMesh"))
        && TestEqual(TEXT("Project default should resolve to ComplexAsSimple and reject simple-only geometry"),
                     Fixture.ActionContext->GetErrorMessages().Num(),
                     1)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Fixture.ActionContext->GetErrorMessages(),
                                                  TEXT("The error should name the effective project mode"),
                                                  TEXT("UseComplexAsSimple"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasUsableCollisionActionReportsDisabledCollisionSeparatelyTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasUsableCollision.ReportsDisabledCollisionSeparately",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasUsableCollisionActionReportsDisabledCollisionSeparatelyTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasUsableCollisionAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::CreateStaticMesh();
    RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::AddBodySetup(StaticMesh,
                                                                          CTF_UseSimpleAsComplex,
                                                                          ECollisionEnabled::NoCollision);
    return RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::Apply(*this,
                                                                          Fixture,
                                                                          Action,
                                                                          StaticMesh,
                                                                          TEXT("DisabledAndMissingCollisionStaticMesh"))
        && TestEqual(TEXT("Missing and disabled collision should add two independent errors"),
                     Fixture.ActionContext->GetErrorMessages().Num(),
                     2)
        && RuleRangerTests::TestTextArrayContains(*this,
                                                  Fixture.ActionContext->GetErrorMessages(),
                                                  TEXT("One error should identify disabled collision"),
                                                  TEXT("state is NoCollision"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureStaticMeshHasUsableCollisionActionDoesNotMutateMeshTest,
                                 "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasUsableCollision.DoesNotMutateMesh",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasUsableCollisionActionDoesNotMutateMeshTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasUsableCollisionAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::CreateStaticMesh();
    const auto BodySetup =
        RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::AddBodySetup(StaticMesh,
                                                                              CTF_UseSimpleAsComplex,
                                                                              ECollisionEnabled::NoCollision);
    const auto OriginalBodySetup = StaticMesh->GetBodySetup();
    const auto OriginalElementCount = BodySetup->AggGeom.GetElementCount();
    const auto OriginalCollisionTraceFlag = BodySetup->CollisionTraceFlag;
    const auto OriginalCollisionEnabled = BodySetup->DefaultInstance.GetCollisionEnabled(false);
    const auto bWasPackageDirty = StaticMesh->GetPackage()->IsDirty();

    return RuleRangerEnsureStaticMeshHasUsableCollisionActionTests::Apply(*this,
                                                                          Fixture,
                                                                          Action,
                                                                          StaticMesh,
                                                                          TEXT("NonMutatingCollisionStaticMesh"),
                                                                          ERuleRangerActionTrigger::AT_Fix)
        && TestEqual(TEXT("Fix should produce the same two diagnostics"),
                     Fixture.ActionContext->GetErrorMessages().Num(),
                     2)
        && TestEqual(TEXT("The BodySetup should not change"), StaticMesh->GetBodySetup(), OriginalBodySetup)
        && TestEqual(TEXT("Aggregate geometry should not change"),
                     BodySetup->AggGeom.GetElementCount(),
                     OriginalElementCount)
        && TestEqual(TEXT("Collision trace mode should not change"),
                     BodySetup->CollisionTraceFlag,
                     OriginalCollisionTraceFlag)
        && TestEqual(TEXT("Collision enabled state should not change"),
                     BodySetup->DefaultInstance.GetCollisionEnabled(false),
                     OriginalCollisionEnabled)
        && TestEqual(TEXT("The package dirty state should not change"),
                     StaticMesh->GetPackage()->IsDirty(),
                     bWasPackageDirty);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasUsableCollisionActionExpectedTypeIsStaticMeshTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasUsableCollision.ExpectedTypeIsStaticMesh",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasUsableCollisionActionExpectedTypeIsStaticMeshTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasUsableCollisionAction>();
    return TestNotNull(TEXT("Action should be created"), Action)
        && TestEqual(TEXT("The expected type should be StaticMesh"),
                     Action->GetExpectedType(),
                     UStaticMesh::StaticClass());
}

#endif
