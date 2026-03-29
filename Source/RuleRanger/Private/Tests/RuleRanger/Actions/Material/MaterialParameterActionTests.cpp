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
    #include "RuleRanger/Actions/Material/EnsureMaterialParameterNamesMatchRegexAction.h"
    #include "RuleRanger/Actions/Material/EnsureMaterialParametersHaveDescriptionsAction.h"
    #include "RuleRanger/Actions/Material/EnsureMaterialParametersHaveGroupsAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerMaterialParameterActionTests
{
    UMaterial* CreateMaterialFixture(FAutomationTestBase& Test,
                                     RuleRangerTests::FRuleFixture& Fixture,
                                     const TCHAR* const ObjectName)
    {
        const auto Material = RuleRangerTests::NewTransientObject<UMaterial>();
        if (RuleRangerTests::CreateRuleFixture(Test, Fixture, ObjectName)
            && Test.TestNotNull(TEXT("Material fixture should be created"), Material))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Material);
            return Material;
        }
        else
        {
            return nullptr;
        }
    }

    UMaterialExpressionScalarParameter* AddScalarParameter(UMaterial* const Material,
                                                           const TCHAR* const Name,
                                                           const TCHAR* const Description = TEXT(""),
                                                           const TCHAR* const Group = nullptr)
    {
        if (nullptr == Material)
        {
            return nullptr;
        }
        else
        {
            const auto Expression = NewObject<UMaterialExpressionScalarParameter>(Material);
            if (Expression)
            {
                Expression->Material = Material;
                Expression->ParameterName = FName(Name);
                Expression->Desc = Description;
                Expression->Group = Group ? FName(Group) : NAME_None;
                Material->GetExpressionCollection().AddExpression(Expression);
                Material->BuildEditorParameterList();
                Material->UpdateCachedExpressionData();
            }
            return Expression;
        }
    }

    bool SetPattern(FAutomationTestBase& Test,
                    UEnsureMaterialParameterNamesMatchRegexAction* const Action,
                    const TCHAR* const Pattern)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Pattern"), FString(Pattern));
    }

    bool SetCaseSensitive(FAutomationTestBase& Test,
                          UEnsureMaterialParameterNamesMatchRegexAction* const Action,
                          const bool bCaseSensitive)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bCaseSensitive"), bCaseSensitive);
    }

    template <typename TAction>
    bool SetThreshold(FAutomationTestBase& Test, TAction* const Action, const int32 Threshold)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Threshold"), Threshold);
    }
} // namespace RuleRangerMaterialParameterActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMaterialParameterNamesMatchRegexErrorsForNonMatchingNamesTest,
                                 "RuleRanger.Actions.Material.ParameterNamesMatchRegex.ErrorsForNonMatchingNames",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaterialParameterNamesMatchRegexErrorsForNonMatchingNamesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaterialParameterNamesMatchRegexAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Material =
        RuleRangerMaterialParameterActionTests::CreateMaterialFixture(*this,
                                                                      Fixture,
                                                                      TEXT("MaterialParameterRegexError"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && TestNotNull(TEXT("Valid parameter should be created"),
                       RuleRangerMaterialParameterActionTests::AddScalarParameter(Material, TEXT("GoodName")))
        && TestNotNull(TEXT("Invalid parameter should be created"),
                       RuleRangerMaterialParameterActionTests::AddScalarParameter(Material, TEXT("bad_name"))))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return TestEqual(TEXT("Only the non-matching parameter should add an error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention the invalid parameter name"),
                                                      TEXT("bad_name"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMaterialParameterNamesMatchRegexHonorsCaseInsensitivePatternsTest,
                                 "RuleRanger.Actions.Material.ParameterNamesMatchRegex.HonorsCaseInsensitivePatterns",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaterialParameterNamesMatchRegexHonorsCaseInsensitivePatternsTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaterialParameterNamesMatchRegexAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Material =
        RuleRangerMaterialParameterActionTests::CreateMaterialFixture(*this,
                                                                      Fixture,
                                                                      TEXT("MaterialParameterRegexCaseInsensitive"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerMaterialParameterActionTests::SetPattern(*this, Action, TEXT("^param_[a-z]+$"))
        && RuleRangerMaterialParameterActionTests::SetCaseSensitive(*this, Action, false)
        && TestNotNull(TEXT("Case-insensitive parameter should be created"),
                       RuleRangerMaterialParameterActionTests::AddScalarParameter(Material, TEXT("Param_Test"))))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return TestTrue(TEXT("A case-insensitive regex should accept mixed-case names"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMaterialParametersHaveDescriptionsErrorsWhenDescriptionsMissingTest,
                                 "RuleRanger.Actions.Material.ParametersHaveDescriptions.ErrorsWhenDescriptionsMissing",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaterialParametersHaveDescriptionsErrorsWhenDescriptionsMissingTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaterialParametersHaveDescriptionsAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Material =
        RuleRangerMaterialParameterActionTests::CreateMaterialFixture(*this,
                                                                      Fixture,
                                                                      TEXT("MaterialParameterDescriptionsMissing"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerMaterialParameterActionTests::SetThreshold(*this, Action, 2)
        && TestNotNull(TEXT("Described parameter should be created"),
                       RuleRangerMaterialParameterActionTests::AddScalarParameter(Material,
                                                                                  TEXT("BaseColor"),
                                                                                  TEXT("Controls the base color")))
        && TestNotNull(TEXT("Undescribed parameter should be created"),
                       RuleRangerMaterialParameterActionTests::AddScalarParameter(Material, TEXT("Roughness"))))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return TestEqual(TEXT("Missing descriptions should add one error once the threshold is met"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention the undescribed parameter"),
                                                      TEXT("Roughness"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMaterialParametersHaveDescriptionsSkipsWhenBelowThresholdTest,
                                 "RuleRanger.Actions.Material.ParametersHaveDescriptions.SkipsWhenBelowThreshold",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaterialParametersHaveDescriptionsSkipsWhenBelowThresholdTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaterialParametersHaveDescriptionsAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Material = RuleRangerMaterialParameterActionTests::CreateMaterialFixture(
        *this,
        Fixture,
        TEXT("MaterialParameterDescriptionsBelowThreshold"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerMaterialParameterActionTests::SetThreshold(*this, Action, 3)
        && TestNotNull(TEXT("Single parameter should be created"),
                       RuleRangerMaterialParameterActionTests::AddScalarParameter(Material, TEXT("BaseColor"))))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return TestTrue(TEXT("Below-threshold materials should skip description enforcement"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMaterialParametersHaveGroupsErrorsWhenGroupsMissingTest,
                                 "RuleRanger.Actions.Material.ParametersHaveGroups.ErrorsWhenGroupsMissing",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaterialParametersHaveGroupsErrorsWhenGroupsMissingTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaterialParametersHaveGroupsAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Material =
        RuleRangerMaterialParameterActionTests::CreateMaterialFixture(*this,
                                                                      Fixture,
                                                                      TEXT("MaterialParameterGroupsMissing"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerMaterialParameterActionTests::SetThreshold(*this, Action, 2)
        && TestNotNull(TEXT("Grouped parameter should be created"),
                       RuleRangerMaterialParameterActionTests::AddScalarParameter(Material,
                                                                                  TEXT("BaseColor"),
                                                                                  TEXT("Controls the base color"),
                                                                                  TEXT("Surface")))
        && TestNotNull(TEXT("Ungrouped parameter should be created"),
                       RuleRangerMaterialParameterActionTests::AddScalarParameter(Material,
                                                                                  TEXT("Roughness"),
                                                                                  TEXT("Controls roughness"))))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return TestEqual(TEXT("Missing groups should add one error once the threshold is met"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention the ungrouped parameter"),
                                                      TEXT("Roughness"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMaterialParametersHaveGroupsSkipsWhenBelowThresholdTest,
                                 "RuleRanger.Actions.Material.ParametersHaveGroups.SkipsWhenBelowThreshold",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaterialParametersHaveGroupsSkipsWhenBelowThresholdTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaterialParametersHaveGroupsAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Material =
        RuleRangerMaterialParameterActionTests::CreateMaterialFixture(*this,
                                                                      Fixture,
                                                                      TEXT("MaterialParameterGroupsBelowThreshold"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerMaterialParameterActionTests::SetThreshold(*this, Action, 3)
        && TestNotNull(TEXT("Single parameter should be created"),
                       RuleRangerMaterialParameterActionTests::AddScalarParameter(Material, TEXT("BaseColor"))))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return TestTrue(TEXT("Below-threshold materials should skip group enforcement"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

#endif
