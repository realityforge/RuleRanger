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
    #include "RuleRanger/Actions/Name/RemoveNamePrefixAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerRemoveNamePrefixActionTests
{
    bool CleanupRenamedAsset(FAutomationTestBase& Test)
    {
        return Test.TestTrue(
            TEXT("The renamed prefix-save asset should be deleted during test cleanup"),
            RuleRangerTests::DeleteAssetIfExists(TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/PrefixSaveAsset"),
                                                 TEXT("PrefixSaveAsset")));
    }

    bool SetPrefix(FAutomationTestBase& Test, URemoveNamePrefixAction* const Action, const TCHAR* const Prefix)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Prefix"), FString(Prefix));
    }

    bool SetCaseSensitive(FAutomationTestBase& Test, URemoveNamePrefixAction* const Action, const bool bCaseSensitive)
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
} // namespace RuleRangerRemoveNamePrefixActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveNamePrefixActionErrorsWhenPrefixEmptyTest,
                                 "RuleRanger.Actions.Name.RemoveNamePrefix.ErrorsWhenPrefixEmpty",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveNamePrefixActionErrorsWhenPrefixEmptyTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveNamePrefixAction>();
    const auto DataTable = RuleRangerRemoveNamePrefixActionTests::CreateDataTableFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/PrefixEmpty"),
        TEXT("PrefixEmptyMaterial"));
    if (TestNotNull(TEXT("RemoveNamePrefixAction should be created"), Action)
        && TestNotNull(TEXT("DataTable fixture should be created"), DataTable)
        && RuleRangerRemoveNamePrefixActionTests::SetPrefix(*this, Action, TEXT("")))
    {
        AddExpectedMessagePlain(TEXT("Empty Prefix specified when attempting to remove Prefix."),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        Action->Apply(Fixture.ActionContext, DataTable);

        return TestTrue(TEXT("Empty prefixes should not add action-context errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("The object name should remain unchanged"),
                         DataTable->GetName(),
                         FString(TEXT("PrefixEmptyMaterial")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveNamePrefixActionSkipsCaseSensitiveMismatchesTest,
                                 "RuleRanger.Actions.Name.RemoveNamePrefix.SkipsCaseSensitiveMismatches",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveNamePrefixActionSkipsCaseSensitiveMismatchesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveNamePrefixAction>();
    const auto DataTable = RuleRangerRemoveNamePrefixActionTests::CreateDataTableFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/PrefixMismatch"),
        TEXT("M_MismatchMaterial"));
    if (TestNotNull(TEXT("RemoveNamePrefixAction should be created"), Action)
        && TestNotNull(TEXT("DataTable fixture should be created"), DataTable)
        && RuleRangerRemoveNamePrefixActionTests::SetPrefix(*this, Action, TEXT("m_"))
        && RuleRangerRemoveNamePrefixActionTests::SetCaseSensitive(*this, Action, true))
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
                         FString(TEXT("M_MismatchMaterial")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveNamePrefixActionWarnsInDryRunModeTest,
                                 "RuleRanger.Actions.Name.RemoveNamePrefix.WarnsInDryRunMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveNamePrefixActionWarnsInDryRunModeTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveNamePrefixAction>();
    const auto DataTable = RuleRangerRemoveNamePrefixActionTests::CreateDataTableFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/PrefixDryRun"),
        TEXT("M_DryRunMaterial"));
    if (TestNotNull(TEXT("RemoveNamePrefixAction should be created"), Action)
        && TestNotNull(TEXT("DataTable fixture should be created"), DataTable)
        && RuleRangerRemoveNamePrefixActionTests::SetPrefix(*this, Action, TEXT("M_")))
    {
        Action->Apply(Fixture.ActionContext, DataTable);

        return TestEqual(TEXT("Dry-run prefix removal should add one warning"),
                         Fixture.ActionContext->GetWarningMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetWarningMessages(),
                                                      TEXT("Dry-run warnings should mention the rename target"),
                                                      TEXT("DryRunMaterial"))
            && TestEqual(TEXT("Dry-run should not rename the object"),
                         DataTable->GetName(),
                         FString(TEXT("M_DryRunMaterial")))
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveNamePrefixActionRenamesInSaveModeTest,
                                 "RuleRanger.Actions.Name.RemoveNamePrefix.RenamesInSaveMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveNamePrefixActionRenamesInSaveModeTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    RuleRangerTests::DeleteAssetIfExists(TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/PrefixSaveAsset"),
                                         TEXT("PrefixSaveAsset"));
    const auto Action = RuleRangerTests::NewTransientObject<URemoveNamePrefixAction>();
    const auto DataTable = RuleRangerRemoveNamePrefixActionTests::CreateDataTableFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/PrefixSave"),
        TEXT("PFX_PrefixSaveAsset"));
    if (TestNotNull(TEXT("RemoveNamePrefixAction should be created"), Action)
        && TestNotNull(TEXT("DataTable fixture should be created"), DataTable)
        && RuleRangerRemoveNamePrefixActionTests::SetPrefix(*this, Action, TEXT("pfx_"))
        && RuleRangerRemoveNamePrefixActionTests::SetCaseSensitive(*this, Action, false))
    {
        Action->Apply(Fixture.ActionContext, DataTable);

        const auto bResult = TestEqual(TEXT("Save mode should rename the object"),
                                       DataTable->GetName(),
                                       FString(TEXT("PrefixSaveAsset")))
            && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                     DataTable,
                                                     true,
                                                     TEXT("Successful prefix removal should dirty the package"));
        const auto bCleanupResult = RuleRangerRemoveNamePrefixActionTests::CleanupRenamedAsset(*this);
        return bResult && bCleanupResult;
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveNamePrefixActionErrorsWhenRenameFailsTest,
                                 "RuleRanger.Actions.Name.RemoveNamePrefix.ErrorsWhenRenameFails",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveNamePrefixActionErrorsWhenRenameFailsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveNamePrefixAction>();
    const auto DataTable = RuleRangerRemoveNamePrefixActionTests::CreateDataTableFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Name/PrefixFailure"),
        TEXT("M_"));
    if (TestNotNull(TEXT("RemoveNamePrefixAction should be created"), Action)
        && TestNotNull(TEXT("DataTable fixture should be created"), DataTable)
        && RuleRangerRemoveNamePrefixActionTests::SetPrefix(*this, Action, TEXT("M_")))
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
                         FString(TEXT("M_")));
    }
    else
    {
        return false;
    }
}

#endif
