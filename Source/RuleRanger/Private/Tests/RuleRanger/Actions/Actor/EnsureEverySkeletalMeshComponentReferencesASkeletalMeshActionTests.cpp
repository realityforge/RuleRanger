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

    #include "Components/SkeletalMeshComponent.h"
    #include "Engine/SkeletalMesh.h"
    #include "GameFramework/Actor.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/Actor/EnsureEverySkeletalMeshComponentReferencesASkeletalMeshAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureEverySkeletalMeshComponentReferencesASkeletalMeshActionTests
{
    AActor*
    CreateActorFixture(FAutomationTestBase& Test, RuleRangerTests::FRuleFixture& Fixture, const TCHAR* ObjectName)
    {
        const auto Actor = RuleRangerTests::NewNamedTransientObject<AActor>(ObjectName);
        const FString FixtureObjectName = FString::Printf(TEXT("%s_ContextObject"), ObjectName);
        if (RuleRangerTests::CreateRuleFixture(*&Test, Fixture, *FixtureObjectName)
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

    USkeletalMeshComponent* AddSkeletalMeshComponent(AActor* Actor, const TCHAR* const ComponentName)
    {
        const auto Component = NewObject<USkeletalMeshComponent>(Actor, ComponentName);
        if (Actor && Component)
        {
            Actor->AddOwnedComponent(Component);
        }
        return Component;
    }
} // namespace RuleRangerEnsureEverySkeletalMeshComponentReferencesASkeletalMeshActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureEverySkeletalMeshComponentReferencesASkeletalMeshActionErrorsForMissingMeshTest,
    "RuleRanger.Actions.Actor.EnsureEverySkeletalMeshComponentReferencesASkeletalMesh.ErrorsForMissingMesh",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureEverySkeletalMeshComponentReferencesASkeletalMeshActionErrorsForMissingMeshTest::RunTest(
    const FString&)
{
    const auto Action =
        RuleRangerTests::NewTransientObject<UEnsureEverySkeletalMeshComponentReferencesASkeletalMeshAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Actor = RuleRangerEnsureEverySkeletalMeshComponentReferencesASkeletalMeshActionTests::CreateActorFixture(
        *this,
        Fixture,
        TEXT("MissingSkeletalMeshActor"));
    const auto Component =
        RuleRangerEnsureEverySkeletalMeshComponentReferencesASkeletalMeshActionTests::AddSkeletalMeshComponent(
            Actor,
            TEXT("MissingSkeletalMeshComponent"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Actor fixture should be created"), Actor)
        && TestNotNull(TEXT("Skeletal mesh component should be created"), Component))
    {
        Action->Apply(Fixture.ActionContext, Actor);

        return RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("Missing skeletal mesh references should add an error"),
                                                      TEXT("does not reference a valid SkeletalMesh"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureEverySkeletalMeshComponentReferencesASkeletalMeshActionSkipsValidMeshesTest,
    "RuleRanger.Actions.Actor.EnsureEverySkeletalMeshComponentReferencesASkeletalMesh.SkipsValidMeshes",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureEverySkeletalMeshComponentReferencesASkeletalMeshActionSkipsValidMeshesTest::RunTest(
    const FString&)
{
    const auto Action =
        RuleRangerTests::NewTransientObject<UEnsureEverySkeletalMeshComponentReferencesASkeletalMeshAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Actor = RuleRangerEnsureEverySkeletalMeshComponentReferencesASkeletalMeshActionTests::CreateActorFixture(
        *this,
        Fixture,
        TEXT("ValidSkeletalMeshActor"));
    const auto Component =
        RuleRangerEnsureEverySkeletalMeshComponentReferencesASkeletalMeshActionTests::AddSkeletalMeshComponent(
            Actor,
            TEXT("ValidSkeletalMeshComponent"));
    const auto SkeletalMesh = RuleRangerTests::NewTransientObject<USkeletalMesh>();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Actor fixture should be created"), Actor)
        && TestNotNull(TEXT("Skeletal mesh component should be created"), Component)
        && TestNotNull(TEXT("Skeletal mesh should be created"), SkeletalMesh))
    {
        Component->SetSkeletalMeshAsset(SkeletalMesh);
        Action->Apply(Fixture.ActionContext, Actor);

        return TestTrue(TEXT("Valid skeletal mesh references should not add errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

#endif
