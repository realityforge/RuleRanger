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
    #include "RuleRanger/Actions/StaticMesh/EnsureStaticMeshHasMinimumLODsAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureStaticMeshHasMinimumLODsActionTests
{
    constexpr TCHAR StaticMeshPath[] = TEXT("/Game/ThirdPerson/LevelPrototyping/Meshes/SM_Cube.SM_Cube");

    UStaticMesh* LoadStaticMesh()
    {
        return LoadObject<UStaticMesh>(nullptr, StaticMeshPath);
    }

    int32 GetMinRequiredLODs(FAutomationTestBase& Test, const UEnsureStaticMeshHasMinimumLODsAction* Action)
    {
        const auto Property = FindFProperty<FIntProperty>(Action->GetClass(), TEXT("MinRequiredLODs"));
        return Test.TestNotNull(TEXT("MinRequiredLODs property should exist"), Property)
            ? Property->GetPropertyValue_InContainer(Action)
            : INDEX_NONE;
    }
} // namespace RuleRangerEnsureStaticMeshHasMinimumLODsActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureStaticMeshHasMinimumLODsActionDefaultsToThreeTest,
                                 "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasMinimumLODs.DefaultsToThree",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasMinimumLODsActionDefaultsToThreeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasMinimumLODsAction>();
    return TestNotNull(TEXT("Action should be created"), Action)
        && TestEqual(TEXT("The default minimum LOD count should be three"),
                     RuleRangerEnsureStaticMeshHasMinimumLODsActionTests::GetMinRequiredLODs(*this, Action),
                     3);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureStaticMeshHasMinimumLODsActionErrorsBelowMinimumTest,
                                 "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasMinimumLODs.ErrorsBelowMinimum",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasMinimumLODsActionErrorsBelowMinimumTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasMinimumLODsAction>();
    const auto StaticMesh = RuleRangerTests::NewTransientObject<UStaticMesh>();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Static mesh should be created"), StaticMesh)
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("BelowMinimumStaticMesh")))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh);
        Action->Apply(Fixture.ActionContext, StaticMesh);

        const auto& Errors = Fixture.ActionContext->GetErrorMessages();
        return TestEqual(TEXT("A below-minimum mesh should add one error"), Errors.Num(), 1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Errors,
                                                      TEXT("The error should include actual and required counts"),
                                                      TEXT("has 0 LOD(s) but requires at least 3"));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureStaticMeshHasMinimumLODsActionAcceptsMinimumTest,
                                 "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasMinimumLODs.AcceptsMinimum",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasMinimumLODsActionAcceptsMinimumTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasMinimumLODsAction>();
    const auto StaticMesh = RuleRangerEnsureStaticMeshHasMinimumLODsActionTests::LoadStaticMesh();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Static mesh fixture should be loaded"), StaticMesh)
        && TestTrue(TEXT("Static mesh fixture should have at least one LOD"), StaticMesh->GetNumLODs() >= 1)
        && RuleRangerTests::SetPropertyValue(*this, Action, TEXT("MinRequiredLODs"), 1)
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("AtMinimumStaticMesh")))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh);
        Action->Apply(Fixture.ActionContext, StaticMesh);
        return TestTrue(TEXT("A mesh meeting the minimum should add no errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureStaticMeshHasMinimumLODsActionClampsInvalidMinimumTest,
                                 "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasMinimumLODs.ClampsInvalidMinimum",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasMinimumLODsActionClampsInvalidMinimumTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasMinimumLODsAction>();
    const auto StaticMesh = RuleRangerTests::NewTransientObject<UStaticMesh>();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Static mesh should be created"), StaticMesh)
        && RuleRangerTests::SetPropertyValue(*this, Action, TEXT("MinRequiredLODs"), 0)
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("InvalidMinimumStaticMesh")))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh);
        Action->Apply(Fixture.ActionContext, StaticMesh);
        const auto& Errors = Fixture.ActionContext->GetErrorMessages();
        return TestEqual(TEXT("An invalid minimum should clamp to one and add one error"), Errors.Num(), 1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Errors,
                                                      TEXT("The error should report the effective minimum"),
                                                      TEXT("requires at least 1"));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureStaticMeshHasMinimumLODsActionDoesNotMutateMeshTest,
                                 "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasMinimumLODs.DoesNotMutateMesh",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasMinimumLODsActionDoesNotMutateMeshTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasMinimumLODsAction>();
    const auto StaticMesh = RuleRangerTests::NewTransientObject<UStaticMesh>();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Static mesh should be created"), StaticMesh)
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("NonMutatingStaticMesh")))
    {
        const auto OriginalLODCount = StaticMesh->GetNumLODs();
        const auto OriginalSourceModelCount = StaticMesh->GetNumSourceModels();
        const auto bWasPackageDirty = StaticMesh->GetPackage()->IsDirty();
        RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh, ERuleRangerActionTrigger::AT_Fix);
        Action->Apply(Fixture.ActionContext, StaticMesh);

        return TestEqual(TEXT("Applying the action should not change the LOD count"),
                         StaticMesh->GetNumLODs(),
                         OriginalLODCount)
            && TestEqual(TEXT("Applying the action should not change source models"),
                         StaticMesh->GetNumSourceModels(),
                         OriginalSourceModelCount)
            && TestEqual(TEXT("Applying the action should not dirty the package"),
                         StaticMesh->GetPackage()->IsDirty(),
                         bWasPackageDirty);
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureStaticMeshHasMinimumLODsActionExpectedTypeIsStaticMeshTest,
    "RuleRanger.Actions.StaticMesh.EnsureStaticMeshHasMinimumLODs.ExpectedTypeIsStaticMesh",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureStaticMeshHasMinimumLODsActionExpectedTypeIsStaticMeshTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureStaticMeshHasMinimumLODsAction>();
    return TestNotNull(TEXT("Action should be created"), Action)
        && TestEqual(TEXT("The expected type should be StaticMesh"),
                     Action->GetExpectedType(),
                     UStaticMesh::StaticClass());
}

#endif
