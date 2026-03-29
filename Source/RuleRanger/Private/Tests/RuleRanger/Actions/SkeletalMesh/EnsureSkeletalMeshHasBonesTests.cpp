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

    #include "Engine/SkeletalMesh.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/SkeletalMesh/EnsureSkeletalMeshHasBones.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureSkeletalMeshHasBonesTests
{
    constexpr TCHAR SkeletalMeshPath[] = TEXT("/Game/Mixamo/Brute/Mesh/SKM_Brute.SKM_Brute");

    USkeletalMesh* LoadSkeletalMesh()
    {
        return LoadObject<USkeletalMesh>(nullptr, SkeletalMeshPath);
    }

    bool SetBones(FAutomationTestBase& Test, UEnsureSkeletalMeshHasBones* const Action, const TSet<FName>& Bones)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Bones"), Bones);
    }

    bool SetReason(FAutomationTestBase& Test, UEnsureSkeletalMeshHasBones* const Action, const TCHAR* const Reason)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Reason"), FString(Reason));
    }
} // namespace RuleRangerEnsureSkeletalMeshHasBonesTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureSkeletalMeshHasBonesErrorsWhenBoneMissingTest,
                                 "RuleRanger.Actions.SkeletalMesh.EnsureSkeletalMeshHasBones.ErrorsWhenBoneMissing",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureSkeletalMeshHasBonesErrorsWhenBoneMissingTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureSkeletalMeshHasBones>();
    const auto SkeletalMesh = RuleRangerEnsureSkeletalMeshHasBonesTests::LoadSkeletalMesh();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Skeletal mesh fixture should be loaded"), SkeletalMesh)
        && TestNotNull(TEXT("Skeletal mesh skeleton should exist"), SkeletalMesh->GetSkeleton())
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("MissingBoneSkeletalMesh"))
        && RuleRangerEnsureSkeletalMeshHasBonesTests::SetReason(*this, Action, TEXT("Required for validation"))
        && RuleRangerEnsureSkeletalMeshHasBonesTests::SetBones(*this, Action, { FName(TEXT("RuleRangerMissingBone")) }))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, SkeletalMesh);
        Action->Apply(Fixture.ActionContext, SkeletalMesh);

        return TestEqual(TEXT("Missing bones should add exactly one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The missing-bone error should mention the bone name"),
                                                      TEXT("RuleRangerMissingBone"))
            && RuleRangerTests::TestTextArrayContains(
                   *this,
                   Fixture.ActionContext->GetErrorMessages(),
                   TEXT("The missing-bone error should include the configured reason"),
                   TEXT("Required for validation"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureSkeletalMeshHasBonesSkipsPresentBonesTest,
                                 "RuleRanger.Actions.SkeletalMesh.EnsureSkeletalMeshHasBones.SkipsPresentBones",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureSkeletalMeshHasBonesSkipsPresentBonesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureSkeletalMeshHasBones>();
    const auto SkeletalMesh = RuleRangerEnsureSkeletalMeshHasBonesTests::LoadSkeletalMesh();
    const auto Skeleton = SkeletalMesh ? SkeletalMesh->GetSkeleton() : nullptr;
    const auto& ReferenceSkeleton = Skeleton ? Skeleton->GetReferenceSkeleton() : FReferenceSkeleton();
    const auto ExistingBone = ReferenceSkeleton.GetRawBoneNum() > 0 ? ReferenceSkeleton.GetBoneName(0) : NAME_None;
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Skeletal mesh fixture should be loaded"), SkeletalMesh)
        && TestNotNull(TEXT("Skeletal mesh skeleton should exist"), Skeleton)
        && TestTrue(TEXT("The skeletal mesh should expose at least one bone"), ExistingBone != NAME_None)
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("PresentBoneSkeletalMesh"))
        && RuleRangerEnsureSkeletalMeshHasBonesTests::SetBones(*this, Action, { ExistingBone }))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, SkeletalMesh);
        Action->Apply(Fixture.ActionContext, SkeletalMesh);

        return TestTrue(TEXT("Present bones should not add errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureSkeletalMeshHasBonesExpectedTypeIsSkeletalMeshTest,
    "RuleRanger.Actions.SkeletalMesh.EnsureSkeletalMeshHasBones.ExpectedTypeIsSkeletalMesh",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureSkeletalMeshHasBonesExpectedTypeIsSkeletalMeshTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureSkeletalMeshHasBones>();
    return TestNotNull(TEXT("Action should be created"), Action)
        && TestEqual(TEXT("The expected type should be SkeletalMesh"),
                     Action->GetExpectedType(),
                     USkeletalMesh::StaticClass());
}

#endif
