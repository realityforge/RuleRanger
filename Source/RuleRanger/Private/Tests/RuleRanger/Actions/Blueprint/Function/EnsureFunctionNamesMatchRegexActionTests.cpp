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
    #include "RuleRanger/Actions/Blueprint/Function/EnsureFunctionNamesMatchRegexAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureFunctionNamesMatchRegexActionTests
{
    UBlueprint* CreateBlueprintFixture(const TCHAR* const PackageName, const TCHAR* const ObjectName)
    {
        return RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                             PackageName,
                                             ObjectName);
    }

    bool SetPattern(FAutomationTestBase& Test,
                    UEnsureFunctionNamesMatchRegexAction* const Action,
                    const TCHAR* const Pattern)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Pattern"), FString(Pattern));
    }

    bool SetCaseSensitive(FAutomationTestBase& Test,
                          UEnsureFunctionNamesMatchRegexAction* const Action,
                          const bool bCaseSensitive)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bCaseSensitive"), bCaseSensitive);
    }
} // namespace RuleRangerEnsureFunctionNamesMatchRegexActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureFunctionNamesMatchRegexActionErrorsForInvalidNamesTest,
                                 "RuleRanger.Actions.Blueprint.Function.EnsureNamesMatchRegex.ErrorsForInvalidNames",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureFunctionNamesMatchRegexActionErrorsForInvalidNamesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureFunctionNamesMatchRegexAction>();
    const auto Blueprint = RuleRangerEnsureFunctionNamesMatchRegexActionTests::CreateBlueprintFixture(
        TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/Function/RegexInvalid"),
        TEXT("BP_FunctionRegexInvalid"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && TestNotNull(TEXT("Function graph should be created"),
                       RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("bad_function")))
        && RuleRangerEnsureFunctionNamesMatchRegexActionTests::SetPattern(*this, Action, TEXT("^[A-Z].*$")))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestEqual(TEXT("Invalid function names should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureFunctionNamesMatchRegexActionSupportsCaseInsensitiveMatchesTest,
    "RuleRanger.Actions.Blueprint.Function.EnsureNamesMatchRegex.SupportsCaseInsensitiveMatches",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureFunctionNamesMatchRegexActionSupportsCaseInsensitiveMatchesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureFunctionNamesMatchRegexAction>();
    const auto Blueprint = RuleRangerEnsureFunctionNamesMatchRegexActionTests::CreateBlueprintFixture(
        TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/Function/RegexCaseInsensitive"),
        TEXT("BP_FunctionRegexCaseInsensitive"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && TestNotNull(TEXT("Function graph should be created"),
                       RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("camelcasefunction")))
        && RuleRangerEnsureFunctionNamesMatchRegexActionTests::SetPattern(*this, Action, TEXT("^[A-Z].*$"))
        && RuleRangerEnsureFunctionNamesMatchRegexActionTests::SetCaseSensitive(*this, Action, false))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestTrue(TEXT("Case-insensitive matching should allow lowercase function names"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

#endif
