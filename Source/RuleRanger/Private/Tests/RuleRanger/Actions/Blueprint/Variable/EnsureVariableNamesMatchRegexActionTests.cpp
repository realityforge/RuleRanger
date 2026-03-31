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
    #include "RuleRanger/Actions/Blueprint/Variable/EnsureVariableNamesMatchRegexAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureVariableNamesMatchRegexActionTests
{
    bool SetPattern(FAutomationTestBase& Test,
                    UEnsureVariableNamesMatchRegexAction* const Action,
                    const TCHAR* const Pattern)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Pattern"), FString(Pattern));
    }

    bool SetCaseSensitive(FAutomationTestBase& Test,
                          UEnsureVariableNamesMatchRegexAction* const Action,
                          const bool bCaseSensitive)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bCaseSensitive"), bCaseSensitive);
    }
} // namespace RuleRangerEnsureVariableNamesMatchRegexActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureVariableNamesMatchRegexActionErrorsForBlueprintVariablesTest,
    "RuleRanger.Actions.Blueprint.Variable.EnsureNamesMatchRegex.ErrorsForBlueprintVariables",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureVariableNamesMatchRegexActionErrorsForBlueprintVariablesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureVariableNamesMatchRegexAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/Variable/RegexBlueprint"),
                                      TEXT("BP_VariableRegexBlueprint"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && TestTrue(
            TEXT("Blueprint variable should be added"),
            RuleRangerTests::AddBlueprintVariable(Blueprint, TEXT("bad_variable"), UEdGraphSchema_K2::PC_Boolean))
        && RuleRangerEnsureVariableNamesMatchRegexActionTests::SetPattern(*this, Action, TEXT("^[A-Z].*$")))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestEqual(TEXT("Invalid Blueprint variables should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureVariableNamesMatchRegexActionErrorsForLocalVariablesTest,
                                 "RuleRanger.Actions.Blueprint.Variable.EnsureNamesMatchRegex.ErrorsForLocalVariables",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureVariableNamesMatchRegexActionErrorsForLocalVariablesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureVariableNamesMatchRegexAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/Variable/RegexLocal"),
                                      TEXT("BP_VariableRegexLocal"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureVariableNamesMatchRegexActionTests::SetPattern(*this, Action, TEXT("^[A-Z].*$")))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("RegexFunction"));
        const auto Entry = Graph ? RuleRangerTests::GetFunctionEntry(Graph) : nullptr;
        if (TestNotNull(TEXT("Function entry should exist"), Entry)
            && TestTrue(TEXT("Local variable should be added"),
                        RuleRangerTests::AddLocalVariable(Entry, TEXT("bad_local"), UEdGraphSchema_K2::PC_Boolean)))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestEqual(TEXT("Invalid local variables should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should mention the function name"),
                                                          TEXT("RegexFunction"));
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
    FRuleRangerEnsureVariableNamesMatchRegexActionSupportsCaseInsensitiveMatchesTest,
    "RuleRanger.Actions.Blueprint.Variable.EnsureNamesMatchRegex.SupportsCaseInsensitiveMatches",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureVariableNamesMatchRegexActionSupportsCaseInsensitiveMatchesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureVariableNamesMatchRegexAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/Variable/RegexCaseInsensitive"),
                                      TEXT("BP_VariableRegexCaseInsensitive"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && TestTrue(
            TEXT("Blueprint variable should be added"),
            RuleRangerTests::AddBlueprintVariable(Blueprint, TEXT("camelcasevariable"), UEdGraphSchema_K2::PC_Boolean))
        && RuleRangerEnsureVariableNamesMatchRegexActionTests::SetPattern(*this, Action, TEXT("^[A-Z].*$"))
        && RuleRangerEnsureVariableNamesMatchRegexActionTests::SetCaseSensitive(*this, Action, false))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestTrue(TEXT("Case-insensitive matching should allow lowercase variable names"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

#endif
