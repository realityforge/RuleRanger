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

    #include "Materials/Material.h"
    #include "Materials/MaterialExpressionScalarParameter.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/Material/EnsureMaterialHasNoCompileErrorAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureMaterialHasNoCompileErrorActionTests
{
    UMaterial* CreateMaterialWithCompileIssue()
    {
        const auto Material = RuleRangerTests::NewTransientObject<UMaterial>();
        if (nullptr != Material)
        {
            // A material with at least one expression but no compiled shader map exercises the compile-error branch
            // without relying on a checked-in host-project fixture asset.
            const auto Expression = NewObject<UMaterialExpressionScalarParameter>(Material);
            if (nullptr != Expression)
            {
                Expression->Material = Material;
                Expression->ParameterName = TEXT("RuleRangerCompileIssue");
                Expression->Desc = TEXT("Synthetic compile-issue fixture");
                Material->GetExpressionCollection().AddExpression(Expression);
                Material->BuildEditorParameterList();
                Material->UpdateCachedExpressionData();
                return Material;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }

    bool SetErrorIfEmpty(FAutomationTestBase& Test,
                         UEnsureMaterialHasNoCompileErrorAction* const Action,
                         const bool bErrorIfEmpty)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bErrorIfEmpty"), bErrorIfEmpty);
    }
} // namespace RuleRangerEnsureMaterialHasNoCompileErrorActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureMaterialHasNoCompileErrorActionErrorsWhenEmptyMaterialDisallowedTest,
    "RuleRanger.Actions.Material.EnsureMaterialHasNoCompileError.ErrorsWhenEmptyMaterialDisallowed",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaterialHasNoCompileErrorActionErrorsWhenEmptyMaterialDisallowedTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaterialHasNoCompileErrorAction>();
    const auto Material = RuleRangerTests::NewTransientObject<UMaterial>();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("EmptyMaterialDisallowed")))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Material);
        Action->Apply(Fixture.ActionContext, Material);

        return TestEqual(TEXT("Empty materials should add one error when empty materials are disallowed"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention that the material is empty"),
                                                      TEXT("0 expressions"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureMaterialHasNoCompileErrorActionSkipsEmptyMaterialWhenAllowedTest,
    "RuleRanger.Actions.Material.EnsureMaterialHasNoCompileError.SkipsEmptyMaterialWhenAllowed",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaterialHasNoCompileErrorActionSkipsEmptyMaterialWhenAllowedTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaterialHasNoCompileErrorAction>();
    const auto Material = RuleRangerTests::NewTransientObject<UMaterial>();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("EmptyMaterialAllowed"))
        && RuleRangerEnsureMaterialHasNoCompileErrorActionTests::SetErrorIfEmpty(*this, Action, false))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Material);
        Action->Apply(Fixture.ActionContext, Material);

        return TestTrue(TEXT("Allowed empty materials should not add errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureMaterialHasNoCompileErrorActionErrorsWhenLoadedMaterialHasCompileIssueTest,
    "RuleRanger.Actions.Material.EnsureMaterialHasNoCompileError.ErrorsWhenLoadedMaterialHasCompileIssue",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaterialHasNoCompileErrorActionErrorsWhenLoadedMaterialHasCompileIssueTest::RunTest(
    const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaterialHasNoCompileErrorAction>();
    const auto Material = RuleRangerEnsureMaterialHasNoCompileErrorActionTests::CreateMaterialWithCompileIssue();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("CompiledMaterial")))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Material);
        Action->Apply(Fixture.ActionContext, Material);

        return TestEqual(TEXT("The synthesized material should add one compile-status error in automation"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMaterialHasNoCompileErrorActionExpectedTypeIsMaterialTest,
                                 "RuleRanger.Actions.Material.EnsureMaterialHasNoCompileError.ExpectedTypeIsMaterial",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaterialHasNoCompileErrorActionExpectedTypeIsMaterialTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaterialHasNoCompileErrorAction>();
    return TestNotNull(TEXT("Action should be created"), Action)
        && TestEqual(TEXT("The expected type should be Material"), Action->GetExpectedType(), UMaterial::StaticClass());
}

#endif
