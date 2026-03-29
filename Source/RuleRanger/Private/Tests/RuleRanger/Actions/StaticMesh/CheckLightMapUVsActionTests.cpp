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
    #include "RuleRanger/Actions/StaticMesh/CheckLightMapUVsAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerCheckLightMapUVsActionTests
{
    constexpr TCHAR StaticMeshPath[] = TEXT("/Game/ThirdPerson/LevelPrototyping/Meshes/SM_Cube.SM_Cube");

    UStaticMesh* LoadStaticMesh()
    {
        return LoadObject<UStaticMesh>(nullptr, StaticMeshPath);
    }
} // namespace RuleRangerCheckLightMapUVsActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckLightMapUVsActionSkipsEmptyTransientMeshTest,
                                 "RuleRanger.Actions.StaticMesh.CheckLightMapUVs.SkipsEmptyTransientMesh",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckLightMapUVsActionSkipsEmptyTransientMeshTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UCheckLightMapUVsAction>();
    const auto StaticMesh = RuleRangerTests::NewTransientObject<UStaticMesh>();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Static mesh fixture should be created"), StaticMesh)
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("MissingLightMapUVsStaticMesh")))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh);
        Action->Apply(Fixture.ActionContext, StaticMesh);

        return TestTrue(TEXT("An empty transient mesh should not add UV errors in this engine setup"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckLightMapUVsActionSkipsValidLightMapUVsTest,
                                 "RuleRanger.Actions.StaticMesh.CheckLightMapUVs.SkipsValidLightMapUVs",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckLightMapUVsActionSkipsValidLightMapUVsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UCheckLightMapUVsAction>();
    const auto StaticMesh = RuleRangerCheckLightMapUVsActionTests::LoadStaticMesh();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Static mesh fixture should be loaded"), StaticMesh)
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("ValidLightMapUVsStaticMesh")))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, StaticMesh);
        Action->Apply(Fixture.ActionContext, StaticMesh);

        return TestTrue(TEXT("Valid lightmap UVs should not add errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckLightMapUVsActionExpectedTypeIsStaticMeshTest,
                                 "RuleRanger.Actions.StaticMesh.CheckLightMapUVs.ExpectedTypeIsStaticMesh",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckLightMapUVsActionExpectedTypeIsStaticMeshTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UCheckLightMapUVsAction>();
    return TestNotNull(TEXT("Action should be created"), Action)
        && TestEqual(TEXT("The expected type should be StaticMesh"),
                     Action->GetExpectedType(),
                     UStaticMesh::StaticClass());
}

#endif
