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
    #include "RuleRanger/Actions/Blueprint/CheckBlueprintCompileStatusAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerCheckBlueprintCompileStatusActionTests
{
    UBlueprint* CreateBlueprintFixture(const TCHAR* const PackageName, const TCHAR* const ObjectName)
    {
        const auto Blueprint = RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                                             PackageName,
                                                             ObjectName);
        if (Blueprint)
        {
            RuleRangerTests::CompileBlueprint(Blueprint);
        }
        return Blueprint;
    }

    bool SetErrorOnUnknown(FAutomationTestBase& Test,
                           UCheckBlueprintCompileStatusAction* const Action,
                           const bool bErrorOnUnknown)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bErrorOnUnknown"), bErrorOnUnknown);
    }

    bool SetErrorOnWarnings(FAutomationTestBase& Test,
                            UCheckBlueprintCompileStatusAction* const Action,
                            const bool bErrorOnWarnings)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test,
                                                     Action,
                                                     TEXT("bErrorOnUpToDateWithWarnings"),
                                                     bErrorOnWarnings);
    }
} // namespace RuleRangerCheckBlueprintCompileStatusActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckBlueprintCompileStatusActionErrorsForDirtyBlueprintsInReportModeTest,
                                 "RuleRanger.Actions.Blueprint.CheckCompileStatus.ErrorsForDirtyBlueprintsInReportMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckBlueprintCompileStatusActionErrorsForDirtyBlueprintsInReportModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UCheckBlueprintCompileStatusAction>();
    const auto Blueprint = RuleRangerCheckBlueprintCompileStatusActionTests::CreateBlueprintFixture(
        TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/CompileStatus/DirtyReport"),
        TEXT("BP_DirtyReport"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        Blueprint->Status = BS_Dirty;
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestEqual(TEXT("Dirty report-mode Blueprints should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention recompilation"),
                                                      TEXT("needs to be recompiled"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckBlueprintCompileStatusActionCompilesDirtyBlueprintsInSaveModeTest,
                                 "RuleRanger.Actions.Blueprint.CheckCompileStatus.CompilesDirtyBlueprintsInSaveMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckBlueprintCompileStatusActionCompilesDirtyBlueprintsInSaveModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UCheckBlueprintCompileStatusAction>();
    const auto Blueprint = RuleRangerCheckBlueprintCompileStatusActionTests::CreateBlueprintFixture(
        TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/CompileStatus/DirtySave"),
        TEXT("BP_DirtySave"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        Blueprint->Status = BS_Dirty;
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint, ERuleRangerActionTrigger::AT_Save);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestTrue(TEXT("Save-mode recompilation should clear dirty Blueprints without context errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("Compilation should leave the Blueprint up to date"),
                         static_cast<EBlueprintStatus>(Blueprint->Status),
                         BS_UpToDate);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckBlueprintCompileStatusActionHonorsUnknownPolicyTest,
                                 "RuleRanger.Actions.Blueprint.CheckCompileStatus.HonorsUnknownPolicy",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckBlueprintCompileStatusActionHonorsUnknownPolicyTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UCheckBlueprintCompileStatusAction>();
    const auto Blueprint = RuleRangerCheckBlueprintCompileStatusActionTests::CreateBlueprintFixture(
        TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/CompileStatus/Unknown"),
        TEXT("BP_Unknown"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerCheckBlueprintCompileStatusActionTests::SetErrorOnUnknown(*this, Action, false))
    {
        Blueprint->Status = BS_Unknown;
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestTrue(TEXT("Unknown status should be accepted when disabled"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckBlueprintCompileStatusActionErrorsForWarningsWhenConfiguredTest,
                                 "RuleRanger.Actions.Blueprint.CheckCompileStatus.ErrorsForWarningsWhenConfigured",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckBlueprintCompileStatusActionErrorsForWarningsWhenConfiguredTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UCheckBlueprintCompileStatusAction>();
    const auto Blueprint = RuleRangerCheckBlueprintCompileStatusActionTests::CreateBlueprintFixture(
        TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/CompileStatus/Warnings"),
        TEXT("BP_Warnings"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        Blueprint->Status = BS_UpToDateWithWarnings;
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestEqual(TEXT("Warning status should add one error by default"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckBlueprintCompileStatusActionAcceptsWarningsWhenDisabledTest,
                                 "RuleRanger.Actions.Blueprint.CheckCompileStatus.AcceptsWarningsWhenDisabled",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckBlueprintCompileStatusActionAcceptsWarningsWhenDisabledTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UCheckBlueprintCompileStatusAction>();
    const auto Blueprint = RuleRangerCheckBlueprintCompileStatusActionTests::CreateBlueprintFixture(
        TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/CompileStatus/WarningsDisabled"),
        TEXT("BP_WarningsDisabled"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerCheckBlueprintCompileStatusActionTests::SetErrorOnWarnings(*this, Action, false))
    {
        Blueprint->Status = BS_UpToDateWithWarnings;
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestTrue(TEXT("Warnings should be accepted when disabled"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

#endif
