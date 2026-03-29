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
    #include "RuleRanger/Actions/Name/RemoveNameSuffixAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerRemoveNameSuffixActionTests
{
    bool CleanupRenamedAsset(FAutomationTestBase& Test)
    {
        return Test.TestTrue(
            TEXT("The renamed suffix-save asset should be deleted during test cleanup"),
            RuleRangerTests::DeleteAssetIfExists(TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/SuffixSaveAsset"),
                                                 TEXT("SuffixSaveAsset")));
    }

    bool SetSuffix(FAutomationTestBase& Test, URemoveNameSuffixAction* const Action, const TCHAR* const Suffix)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Suffix"), FString(Suffix));
    }

    bool SetCaseSensitive(FAutomationTestBase& Test, URemoveNameSuffixAction* const Action, const bool bCaseSensitive)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bCaseSensitive"), bCaseSensitive);
    }

    UDataTable* CreateDataTableFixture(FAutomationTestBase& Test,
                                       RuleRangerTests::FRuleFixture& Fixture,
                                       const ERuleRangerActionTrigger Trigger,
                                       const TCHAR* const PackageName,
                                       const TCHAR* const ObjectName)
    {
        const auto DataTable =
            RuleRangerTests::NewDataTable(PackageName, ObjectName, FDataOnlyBlueprintEntry::StaticStruct());
        if (RuleRangerTests::CreateRuleFixture(Test, Fixture, ObjectName, Trigger)
            && Test.TestNotNull(TEXT("DataTable should be created"), DataTable))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, DataTable, Trigger);
            RuleRangerTests::ClearPackageDirtyFlag(DataTable);
            return DataTable;
        }
        else
        {
            return nullptr;
        }
    }
} // namespace RuleRangerRemoveNameSuffixActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveNameSuffixActionErrorsWhenSuffixEmptyTest,
                                 "RuleRanger.Actions.Name.RemoveNameSuffix.ErrorsWhenSuffixEmpty",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveNameSuffixActionErrorsWhenSuffixEmptyTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveNameSuffixAction>();
    const auto DataTable = RuleRangerRemoveNameSuffixActionTests::CreateDataTableFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/SuffixEmpty"),
        TEXT("SuffixEmptyMaterial"));
    if (TestNotNull(TEXT("RemoveNameSuffixAction should be created"), Action)
        && TestNotNull(TEXT("DataTable fixture should be created"), DataTable)
        && RuleRangerRemoveNameSuffixActionTests::SetSuffix(*this, Action, TEXT("")))
    {
        AddExpectedMessagePlain(TEXT("Empty Suffix specified when attempting to remove Suffix."),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        Action->Apply(Fixture.ActionContext, DataTable);

        return TestTrue(TEXT("Empty suffixes should not add action-context errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("The object name should remain unchanged"),
                         DataTable->GetName(),
                         FString(TEXT("SuffixEmptyMaterial")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveNameSuffixActionSkipsCaseSensitiveMismatchesTest,
                                 "RuleRanger.Actions.Name.RemoveNameSuffix.SkipsCaseSensitiveMismatches",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveNameSuffixActionSkipsCaseSensitiveMismatchesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveNameSuffixAction>();
    const auto DataTable = RuleRangerRemoveNameSuffixActionTests::CreateDataTableFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/SuffixMismatch"),
        TEXT("MismatchMaterial_M"));
    if (TestNotNull(TEXT("RemoveNameSuffixAction should be created"), Action)
        && TestNotNull(TEXT("DataTable fixture should be created"), DataTable)
        && RuleRangerRemoveNameSuffixActionTests::SetSuffix(*this, Action, TEXT("_m"))
        && RuleRangerRemoveNameSuffixActionTests::SetCaseSensitive(*this, Action, true))
    {
        Action->Apply(Fixture.ActionContext, DataTable);

        return TestTrue(TEXT("Case-sensitive mismatches should not emit info"),
                        Fixture.ActionContext->GetInfoMessages().IsEmpty())
            && TestTrue(TEXT("Case-sensitive mismatches should not emit warnings"),
                        Fixture.ActionContext->GetWarningMessages().IsEmpty())
            && TestTrue(TEXT("Case-sensitive mismatches should not emit errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("The object name should remain unchanged"),
                         DataTable->GetName(),
                         FString(TEXT("MismatchMaterial_M")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveNameSuffixActionWarnsInDryRunModeTest,
                                 "RuleRanger.Actions.Name.RemoveNameSuffix.WarnsInDryRunMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveNameSuffixActionWarnsInDryRunModeTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveNameSuffixAction>();
    const auto DataTable = RuleRangerRemoveNameSuffixActionTests::CreateDataTableFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/SuffixDryRun"),
        TEXT("DryRunMaterial_M"));
    if (TestNotNull(TEXT("RemoveNameSuffixAction should be created"), Action)
        && TestNotNull(TEXT("DataTable fixture should be created"), DataTable)
        && RuleRangerRemoveNameSuffixActionTests::SetSuffix(*this, Action, TEXT("_M")))
    {
        Action->Apply(Fixture.ActionContext, DataTable);

        return TestEqual(TEXT("Dry-run suffix removal should add one warning"),
                         Fixture.ActionContext->GetWarningMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetWarningMessages(),
                                                      TEXT("Dry-run warnings should mention the rename target"),
                                                      TEXT("DryRunMaterial"))
            && TestEqual(TEXT("Dry-run should not rename the object"),
                         DataTable->GetName(),
                         FString(TEXT("DryRunMaterial_M")))
            && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                     DataTable,
                                                     false,
                                                     TEXT("Dry-run should not dirty the package"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveNameSuffixActionRenamesInSaveModeTest,
                                 "RuleRanger.Actions.Name.RemoveNameSuffix.RenamesInSaveMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveNameSuffixActionRenamesInSaveModeTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    RuleRangerTests::DeleteAssetIfExists(TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/SuffixSaveAsset"),
                                         TEXT("SuffixSaveAsset"));
    const auto Action = RuleRangerTests::NewTransientObject<URemoveNameSuffixAction>();
    const auto DataTable = RuleRangerRemoveNameSuffixActionTests::CreateDataTableFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/SuffixSave"),
        TEXT("SuffixSaveAsset_SFX"));
    if (TestNotNull(TEXT("RemoveNameSuffixAction should be created"), Action)
        && TestNotNull(TEXT("DataTable fixture should be created"), DataTable)
        && RuleRangerRemoveNameSuffixActionTests::SetSuffix(*this, Action, TEXT("_sfx"))
        && RuleRangerRemoveNameSuffixActionTests::SetCaseSensitive(*this, Action, false))
    {
        Action->Apply(Fixture.ActionContext, DataTable);

        const auto bResult = TestEqual(TEXT("Save mode should rename the object"),
                                       DataTable->GetName(),
                                       FString(TEXT("SuffixSaveAsset")))
            && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                     DataTable,
                                                     true,
                                                     TEXT("Successful suffix removal should dirty the package"));
        const auto bCleanupResult = RuleRangerRemoveNameSuffixActionTests::CleanupRenamedAsset(*this);
        return bResult && bCleanupResult;
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveNameSuffixActionErrorsWhenRenameFailsTest,
                                 "RuleRanger.Actions.Name.RemoveNameSuffix.ErrorsWhenRenameFails",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveNameSuffixActionErrorsWhenRenameFailsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveNameSuffixAction>();
    const auto DataTable = RuleRangerRemoveNameSuffixActionTests::CreateDataTableFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/SuffixFailure"),
        TEXT("_M"));
    if (TestNotNull(TEXT("RemoveNameSuffixAction should be created"), Action)
        && TestNotNull(TEXT("DataTable fixture should be created"), DataTable)
        && RuleRangerRemoveNameSuffixActionTests::SetSuffix(*this, Action, TEXT("_M")))
    {
        AddExpectedMessagePlain(TEXT("Invalid object name"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        Action->Apply(Fixture.ActionContext, DataTable);

        return TestFalse(TEXT("Failed renames should add an action-context error"),
                         Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The action-context error should mention the failed rename"),
                                                      TEXT("Attempt to rename object"))
            && TestEqual(TEXT("The failed rename should leave the original name intact"),
                         DataTable->GetName(),
                         FString(TEXT("_M")));
    }
    else
    {
        return false;
    }
}

#endif
