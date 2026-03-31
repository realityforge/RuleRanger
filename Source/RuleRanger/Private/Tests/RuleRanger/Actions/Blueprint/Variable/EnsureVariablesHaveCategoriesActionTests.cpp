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
    #include "RuleRanger/Actions/Blueprint/Variable/EnsureVariablesHaveCategoriesAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureVariablesHaveCategoriesActionTests
{
    bool
    SetThreshold(FAutomationTestBase& Test, UEnsureVariablesHaveCategoriesAction* const Action, const int32 Threshold)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Threshold"), Threshold);
    }

    bool SetCheckInstanceEditableVariables(FAutomationTestBase& Test,
                                           UEnsureVariablesHaveCategoriesAction* const Action,
                                           const bool bCheckInstanceEditableVariables)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test,
                                                     Action,
                                                     TEXT("bCheckInstanceEditableVariables"),
                                                     bCheckInstanceEditableVariables);
    }

    bool SetCheckLocalVariables(FAutomationTestBase& Test,
                                UEnsureVariablesHaveCategoriesAction* const Action,
                                const bool bCheckLocalVariables)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bCheckLocalVariables"), bCheckLocalVariables);
    }
} // namespace RuleRangerEnsureVariablesHaveCategoriesActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureVariablesHaveCategoriesActionErrorsForBlueprintVariablesAboveThresholdTest,
    "RuleRanger.Actions.Blueprint.Variable.EnsureCategories.ErrorsForBlueprintVariablesAboveThreshold",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureVariablesHaveCategoriesActionErrorsForBlueprintVariablesAboveThresholdTest::RunTest(
    const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureVariablesHaveCategoriesAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/Variable/CategoriesBlueprint"),
                                      TEXT("BP_VariableCategoriesBlueprint"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureVariablesHaveCategoriesActionTests::SetThreshold(*this, Action, 2)
        && TestTrue(TEXT("First Blueprint variable should be added"),
                    RuleRangerTests::AddBlueprintVariable(Blueprint,
                                                          TEXT("FirstVariable"),
                                                          UEdGraphSchema_K2::PC_Boolean,
                                                          FText::FromString(TEXT("Gameplay"))))
        && TestTrue(
            TEXT("Second Blueprint variable should be added"),
            RuleRangerTests::AddBlueprintVariable(Blueprint, TEXT("SecondVariable"), UEdGraphSchema_K2::PC_Boolean)))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestEqual(TEXT("Default Blueprint categories above threshold should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureVariablesHaveCategoriesActionErrorsForLocalVariablesAboveThresholdTest,
    "RuleRanger.Actions.Blueprint.Variable.EnsureCategories.ErrorsForLocalVariablesAboveThreshold",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureVariablesHaveCategoriesActionErrorsForLocalVariablesAboveThresholdTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureVariablesHaveCategoriesAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/Variable/CategoriesLocal"),
                                      TEXT("BP_VariableCategoriesLocal"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureVariablesHaveCategoriesActionTests::SetThreshold(*this, Action, 2)
        && RuleRangerEnsureVariablesHaveCategoriesActionTests::SetCheckLocalVariables(*this, Action, true))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("LocalVariableFunction"));
        const auto Entry = Graph ? RuleRangerTests::GetFunctionEntry(Graph) : nullptr;
        if (TestNotNull(TEXT("Function entry should exist"), Entry)
            && TestTrue(TEXT("First local variable should be added"),
                        RuleRangerTests::AddLocalVariable(Entry,
                                                          TEXT("LocalOne"),
                                                          UEdGraphSchema_K2::PC_Boolean,
                                                          FText::FromString(TEXT("Gameplay"))))
            && TestTrue(TEXT("Second local variable should be added"),
                        RuleRangerTests::AddLocalVariable(Entry, TEXT("LocalTwo"), UEdGraphSchema_K2::PC_Boolean)))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestEqual(TEXT("Default local categories above threshold should add one error"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureVariablesHaveCategoriesActionSkipsInstanceEditableChecksWhenDisabledTest,
    "RuleRanger.Actions.Blueprint.Variable.EnsureCategories.SkipsInstanceEditableChecksWhenDisabled",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureVariablesHaveCategoriesActionSkipsInstanceEditableChecksWhenDisabledTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureVariablesHaveCategoriesAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/Variable/CategoriesSkip"),
                                      TEXT("BP_VariableCategoriesSkip"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureVariablesHaveCategoriesActionTests::SetThreshold(*this, Action, 1)
        && RuleRangerEnsureVariablesHaveCategoriesActionTests::SetCheckInstanceEditableVariables(*this, Action, false)
        && TestTrue(
            TEXT("Instance-editable variable should be added"),
            RuleRangerTests::AddBlueprintVariable(Blueprint, TEXT("EditableVariable"), UEdGraphSchema_K2::PC_Boolean)))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestTrue(TEXT("Instance-editable variables should be skipped when disabled"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

#endif
