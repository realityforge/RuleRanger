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

    #include "Components/StaticMeshComponent.h"
    #include "Engine/Blueprint.h"
    #include "Engine/StaticMesh.h"
    #include "GameFramework/Actor.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/Actor/EnsureEveryStaticMeshComponentReferencesAStaticMeshAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionTests
{
    AActor*
    CreateActorFixture(FAutomationTestBase& Test, RuleRangerTests::FRuleFixture& Fixture, const TCHAR* ObjectName)
    {
        const auto Actor = RuleRangerTests::NewNamedTransientObject<AActor>(ObjectName);
        const FString FixtureObjectName = FString::Printf(TEXT("%s_ContextObject"), ObjectName);
        if (RuleRangerTests::CreateRuleFixture(Test, Fixture, *FixtureObjectName)
            && Test.TestNotNull(TEXT("Actor fixture should be created"), Actor))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Actor);
            return Actor;
        }
        else
        {
            return nullptr;
        }
    }

    UStaticMeshComponent* AddStaticMeshComponent(AActor* Actor, const TCHAR* const ComponentName)
    {
        const auto Component = NewObject<UStaticMeshComponent>(Actor, ComponentName);
        if (Actor && Component)
        {
            Actor->AddOwnedComponent(Component);
        }
        return Component;
    }
} // namespace RuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionErrorsForMissingMeshTest,
    "RuleRanger.Actions.Actor.EnsureEveryStaticMeshComponentReferencesAStaticMesh.ErrorsForMissingMesh",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionErrorsForMissingMeshTest::RunTest(
    const FString&)
{
    const auto Action =
        RuleRangerTests::NewTransientObject<UEnsureEveryStaticMeshComponentReferencesAStaticMeshAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Actor = RuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionTests::CreateActorFixture(
        *this,
        Fixture,
        TEXT("MissingStaticMeshActor"));
    const auto Component =
        RuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionTests::AddStaticMeshComponent(
            Actor,
            TEXT("MissingStaticMeshComponent"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Actor fixture should be created"), Actor)
        && TestNotNull(TEXT("Static mesh component should be created"), Component))
    {
        Action->Apply(Fixture.ActionContext, Actor);

        return RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("Missing static mesh references should add an error"),
                                                      TEXT("does not reference a valid StaticMesh"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionSkipsValidMeshesTest,
    "RuleRanger.Actions.Actor.EnsureEveryStaticMeshComponentReferencesAStaticMesh.SkipsValidMeshes",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionSkipsValidMeshesTest::RunTest(const FString&)
{
    const auto Action =
        RuleRangerTests::NewTransientObject<UEnsureEveryStaticMeshComponentReferencesAStaticMeshAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Actor = RuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionTests::CreateActorFixture(
        *this,
        Fixture,
        TEXT("ValidStaticMeshActor"));
    const auto Component =
        RuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionTests::AddStaticMeshComponent(
            Actor,
            TEXT("ValidStaticMeshComponent"));
    const auto StaticMesh = RuleRangerTests::NewTransientObject<UStaticMesh>();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Actor fixture should be created"), Actor)
        && TestNotNull(TEXT("Static mesh component should be created"), Component)
        && TestNotNull(TEXT("Static mesh should be created"), StaticMesh))
    {
        Component->SetStaticMesh(StaticMesh);
        Action->Apply(Fixture.ActionContext, Actor);

        return TestTrue(TEXT("Valid static mesh references should not add errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionSkipsAbstractBlueprintsTest,
    "RuleRanger.Actions.Actor.EnsureEveryStaticMeshComponentReferencesAStaticMesh.SkipsAbstractBlueprints",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureEveryStaticMeshComponentReferencesAStaticMeshActionSkipsAbstractBlueprintsTest::RunTest(
    const FString&)
{
    const auto Action =
        RuleRangerTests::NewTransientObject<UEnsureEveryStaticMeshComponentReferencesAStaticMeshAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(AActor::StaticClass(),
                                      TEXT("/Game/Developers/Tests/RuleRanger/Actor/AbstractStaticMeshActor"),
                                      TEXT("AbstractStaticMeshActor"));
    RuleRangerTests::FRuleFixture Fixture;
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Abstract actor blueprint should be created"), Blueprint)
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("AbstractStaticMeshActor")))
    {
        Blueprint->bGenerateAbstractClass = true;
        RuleRangerTests::CompileBlueprint(Blueprint);
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);

        return TestTrue(TEXT("Abstract actor blueprints should be skipped without errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

#endif
