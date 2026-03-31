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
    #include "RuleRanger/Actions/Metadata/SetMetadataTagsAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerSetMetadataTagsActionTests
{
    bool SetMetadataTags(FAutomationTestBase& Test,
                         USetMetadataTagsAction* const Action,
                         const TMap<FName, FString>& MetadataTags)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("MetadataTags"), MetadataTags);
    }

    UMaterial* CreateMaterialFixture(FAutomationTestBase& Test,
                                     RuleRangerTests::FRuleFixture& Fixture,
                                     const ERuleRangerActionTrigger Trigger,
                                     const TCHAR* const PackageName,
                                     const TCHAR* const ObjectName)
    {
        const auto Material = RuleRangerTests::NewPackagedMaterial(PackageName, ObjectName);
        if (RuleRangerTests::CreateRuleFixture(Test, Fixture, ObjectName, Trigger)
            && Test.TestNotNull(TEXT("Material should be created"), Material))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Material, Trigger);
            RuleRangerTests::ClearPackageDirtyFlag(Material);
            return Material;
        }
        else
        {
            return nullptr;
        }
    }
} // namespace RuleRangerSetMetadataTagsActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerSetMetadataTagsActionErrorsWhenKeyEmptyTest,
                                 "RuleRanger.Actions.Metadata.SetMetadataTags.ErrorsWhenKeyEmpty",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerSetMetadataTagsActionErrorsWhenKeyEmptyTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<USetMetadataTagsAction>();
    const auto Material = RuleRangerSetMetadataTagsActionTests::CreateMaterialFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/Tests/RuleRanger/Mutation/Metadata/SetEmptyKey"),
        TEXT("SetEmptyKeyMaterial"));
    if (TestNotNull(TEXT("SetMetadataTagsAction should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerSetMetadataTagsActionTests::SetMetadataTags(*this, Action, { { NAME_None, TEXT("Value") } }))
    {
        AddExpectedMessagePlain(TEXT("Empty key specified when attempting to add MetadataTag"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        Action->Apply(Fixture.ActionContext, Material);

        return TestTrue(TEXT("Empty metadata keys should not add action-context errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerSetMetadataTagsActionErrorsWhenValueEmptyTest,
                                 "RuleRanger.Actions.Metadata.SetMetadataTags.ErrorsWhenValueEmpty",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerSetMetadataTagsActionErrorsWhenValueEmptyTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<USetMetadataTagsAction>();
    const auto Material = RuleRangerSetMetadataTagsActionTests::CreateMaterialFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/Tests/RuleRanger/Mutation/Metadata/SetEmptyValue"),
        TEXT("SetEmptyValueMaterial"));
    if (TestNotNull(TEXT("SetMetadataTagsAction should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerSetMetadataTagsActionTests::SetMetadataTags(*this,
                                                                 Action,
                                                                 { { TEXT("RuleRanger.Variant"), TEXT("") } }))
    {
        AddExpectedMessagePlain(TEXT("Empty Value specified when attempting to add MetadataTag"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        Action->Apply(Fixture.ActionContext, Material);

        return TestTrue(TEXT("Empty metadata values should not add action-context errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerSetMetadataTagsActionLogsInfoWhenValueAlreadyPresentTest,
                                 "RuleRanger.Actions.Metadata.SetMetadataTags.LogsInfoWhenValueAlreadyPresent",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerSetMetadataTagsActionLogsInfoWhenValueAlreadyPresentTest::RunTest(const FString&)
{
    static const FName VariantKey(TEXT("RuleRanger.Variant"));

    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<USetMetadataTagsAction>();
    const auto Material = RuleRangerSetMetadataTagsActionTests::CreateMaterialFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Mutation/Metadata/SetExisting"),
        TEXT("SetExistingMaterial"));
    if (TestNotNull(TEXT("SetMetadataTagsAction should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && TestTrue(TEXT("Existing metadata should be seeded"),
                    RuleRangerTests::SetAssetMetaData(Material, VariantKey, TEXT("BaseColor")))
        && RuleRangerTests::ClearPackageDirtyFlag(Material)
        && RuleRangerSetMetadataTagsActionTests::SetMetadataTags(*this, Action, { { VariantKey, TEXT("BaseColor") } }))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return TestEqual(TEXT("The existing metadata value should remain unchanged"),
                         RuleRangerTests::GetAssetMetaData(Material, VariantKey),
                         FString(TEXT("BaseColor")))
            && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                     Material,
                                                     false,
                                                     TEXT("Existing metadata should not dirty the package"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerSetMetadataTagsActionErrorsInDryRunModeTest,
                                 "RuleRanger.Actions.Metadata.SetMetadataTags.ErrorsInDryRunMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerSetMetadataTagsActionErrorsInDryRunModeTest::RunTest(const FString&)
{
    static const FName VariantKey(TEXT("RuleRanger.Variant"));

    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<USetMetadataTagsAction>();
    const auto Material = RuleRangerSetMetadataTagsActionTests::CreateMaterialFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/Tests/RuleRanger/Mutation/Metadata/SetDryRun"),
        TEXT("SetDryRunMaterial"));
    if (TestNotNull(TEXT("SetMetadataTagsAction should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerSetMetadataTagsActionTests::SetMetadataTags(*this, Action, { { VariantKey, TEXT("BaseColor") } }))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return TestEqual(TEXT("Dry-run metadata writes should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention DryRun mode"),
                                                      TEXT("DryRun mode"))
            && TestTrue(TEXT("Dry-run should not add metadata"),
                        RuleRangerTests::GetAssetMetaData(Material, VariantKey).IsEmpty())
            && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                     Material,
                                                     false,
                                                     TEXT("Dry-run metadata writes should not dirty the package"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerSetMetadataTagsActionWritesMetadataInSaveModeTest,
                                 "RuleRanger.Actions.Metadata.SetMetadataTags.WritesMetadataInSaveMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerSetMetadataTagsActionWritesMetadataInSaveModeTest::RunTest(const FString&)
{
    static const FName VariantKey(TEXT("RuleRanger.Variant"));

    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<USetMetadataTagsAction>();
    const auto Material = RuleRangerSetMetadataTagsActionTests::CreateMaterialFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Mutation/Metadata/SetSave"),
        TEXT("SetSaveMaterial"));
    if (TestNotNull(TEXT("SetMetadataTagsAction should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerSetMetadataTagsActionTests::SetMetadataTags(*this, Action, { { VariantKey, TEXT("BaseColor") } }))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return TestEqual(TEXT("Successful metadata writes should add one info"),
                         Fixture.ActionContext->GetInfoMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetInfoMessages(),
                                                      TEXT("The info should mention the added metadata"),
                                                      TEXT("Adding tag"))
            && TestEqual(TEXT("Save mode should write the metadata value"),
                         RuleRangerTests::GetAssetMetaData(Material, VariantKey),
                         FString(TEXT("BaseColor")))
            && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                     Material,
                                                     true,
                                                     TEXT("Successful metadata writes should dirty the package"));
    }
    else
    {
        return false;
    }
}

#endif
