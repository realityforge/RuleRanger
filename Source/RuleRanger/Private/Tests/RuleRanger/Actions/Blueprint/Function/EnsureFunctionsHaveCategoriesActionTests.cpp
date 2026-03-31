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

    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/Blueprint/Function/EnsureFunctionsHaveCategoriesAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureFunctionsHaveCategoriesActionTests
{
    bool
    SetThreshold(FAutomationTestBase& Test, UEnsureFunctionsHaveCategoriesAction* const Action, const int32 Threshold)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Threshold"), Threshold);
    }
} // namespace RuleRangerEnsureFunctionsHaveCategoriesActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureFunctionsHaveCategoriesActionErrorsAboveThresholdTest,
                                 "RuleRanger.Actions.Blueprint.Function.EnsureCategories.ErrorsAboveThreshold",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureFunctionsHaveCategoriesActionErrorsAboveThresholdTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureFunctionsHaveCategoriesAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/Function/CategoriesError"),
                                      TEXT("BP_FunctionCategoriesError"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureFunctionsHaveCategoriesActionTests::SetThreshold(*this, Action, 2))
    {
        const auto FirstGraph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("PrimaryFunction"));
        const auto SecondGraph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("SecondaryFunction"));
        const auto FirstEntry = FirstGraph ? RuleRangerTests::GetFunctionEntry(FirstGraph) : nullptr;
        const auto SecondEntry = SecondGraph ? RuleRangerTests::GetFunctionEntry(SecondGraph) : nullptr;
        if (TestNotNull(TEXT("First entry node should exist"), FirstEntry)
            && TestNotNull(TEXT("Second entry node should exist"), SecondEntry)
            && TestTrue(TEXT("First function category should be set"),
                        RuleRangerTests::SetFunctionCategory(FirstEntry, TEXT("Gameplay")))
            && TestTrue(TEXT("Second function category should explicitly remain Default"),
                        RuleRangerTests::SetFunctionCategory(SecondEntry, TEXT("Default"))))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestEqual(TEXT("A default category above threshold should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureFunctionsHaveCategoriesActionSkipsBelowThresholdTest,
                                 "RuleRanger.Actions.Blueprint.Function.EnsureCategories.SkipsBelowThreshold",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureFunctionsHaveCategoriesActionSkipsBelowThresholdTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureFunctionsHaveCategoriesAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/Function/CategoriesSkip"),
                                      TEXT("BP_FunctionCategoriesSkip"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureFunctionsHaveCategoriesActionTests::SetThreshold(*this, Action, 2)
        && TestNotNull(TEXT("Function graph should be created"),
                       RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("OnlyFunction"))))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestTrue(TEXT("Below-threshold Blueprints should not add errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

#endif
