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
    #include "RuleRanger/Actions/Metadata/RemoveMetadataTagsAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerRemoveMetadataTagsActionTests
{
    bool SetKeys(FAutomationTestBase& Test, URemoveMetadataTagsAction* const Action, const TArray<FName>& Keys)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Keys"), Keys);
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
} // namespace RuleRangerRemoveMetadataTagsActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveMetadataTagsActionErrorsWhenKeyEmptyTest,
                                 "RuleRanger.Actions.Metadata.RemoveMetadataTags.ErrorsWhenKeyEmpty",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveMetadataTagsActionErrorsWhenKeyEmptyTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveMetadataTagsAction>();
    const auto Material = RuleRangerRemoveMetadataTagsActionTests::CreateMaterialFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Metadata/RemoveEmptyKey"),
        TEXT("RemoveEmptyKeyMaterial"));
    if (TestNotNull(TEXT("RemoveMetadataTagsAction should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerRemoveMetadataTagsActionTests::SetKeys(*this, Action, { NAME_None }))
    {
        AddExpectedMessagePlain(TEXT("Empty key specified when attempting to remove MetadataTag."),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveMetadataTagsActionLogsInfoWhenKeyMissingTest,
                                 "RuleRanger.Actions.Metadata.RemoveMetadataTags.LogsInfoWhenKeyMissing",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveMetadataTagsActionLogsInfoWhenKeyMissingTest::RunTest(const FString&)
{
    static const FName VariantKey(TEXT("RuleRanger.Variant"));

    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveMetadataTagsAction>();
    const auto Material = RuleRangerRemoveMetadataTagsActionTests::CreateMaterialFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Metadata/RemoveMissing"),
        TEXT("RemoveMissingMaterial"));
    if (TestNotNull(TEXT("RemoveMetadataTagsAction should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && RuleRangerRemoveMetadataTagsActionTests::SetKeys(*this, Action, { VariantKey }))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return RuleRangerTests::TestPackageDirtyFlag(*this,
                                                     Material,
                                                     false,
                                                     TEXT("Missing metadata keys should not dirty the package"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveMetadataTagsActionErrorsInDryRunModeTest,
                                 "RuleRanger.Actions.Metadata.RemoveMetadataTags.ErrorsInDryRunMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveMetadataTagsActionErrorsInDryRunModeTest::RunTest(const FString&)
{
    static const FName VariantKey(TEXT("RuleRanger.Variant"));

    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveMetadataTagsAction>();
    const auto Material = RuleRangerRemoveMetadataTagsActionTests::CreateMaterialFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Metadata/RemoveDryRun"),
        TEXT("RemoveDryRunMaterial"));
    if (TestNotNull(TEXT("RemoveMetadataTagsAction should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && TestTrue(TEXT("Metadata should be seeded"),
                    RuleRangerTests::SetAssetMetaData(Material, VariantKey, TEXT("BaseColor")))
        && RuleRangerTests::ClearPackageDirtyFlag(Material)
        && RuleRangerRemoveMetadataTagsActionTests::SetKeys(*this, Action, { VariantKey }))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return TestEqual(TEXT("Dry-run removals should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention DryRun mode"),
                                                      TEXT("DryRun mode"))
            && TestEqual(TEXT("Dry-run removals should leave the metadata intact"),
                         RuleRangerTests::GetAssetMetaData(Material, VariantKey),
                         FString(TEXT("BaseColor")))
            && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                     Material,
                                                     false,
                                                     TEXT("Dry-run removals should not dirty the package"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRemoveMetadataTagsActionRemovesMetadataInSaveModeTest,
                                 "RuleRanger.Actions.Metadata.RemoveMetadataTags.RemovesMetadataInSaveMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRemoveMetadataTagsActionRemovesMetadataInSaveModeTest::RunTest(const FString&)
{
    static const FName VariantKey(TEXT("RuleRanger.Variant"));

    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URemoveMetadataTagsAction>();
    const auto Material = RuleRangerRemoveMetadataTagsActionTests::CreateMaterialFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/RuleRangerTests/Mutation/Metadata/RemoveSave"),
        TEXT("RemoveSaveMaterial"));
    if (TestNotNull(TEXT("RemoveMetadataTagsAction should be created"), Action)
        && TestNotNull(TEXT("Material fixture should be created"), Material)
        && TestTrue(TEXT("Metadata should be seeded"),
                    RuleRangerTests::SetAssetMetaData(Material, VariantKey, TEXT("BaseColor")))
        && RuleRangerTests::ClearPackageDirtyFlag(Material)
        && RuleRangerRemoveMetadataTagsActionTests::SetKeys(*this, Action, { VariantKey }))
    {
        Action->Apply(Fixture.ActionContext, Material);

        return TestEqual(TEXT("Successful removals should add one info"),
                         Fixture.ActionContext->GetInfoMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetInfoMessages(),
                                                      TEXT("The info should mention the metadata removal"),
                                                      TEXT("Removing tag"))
            && TestTrue(TEXT("Save mode should remove the metadata value"),
                        RuleRangerTests::GetAssetMetaData(Material, VariantKey).IsEmpty())
            && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                     Material,
                                                     true,
                                                     TEXT("Successful removals should dirty the package"));
    }
    else
    {
        return false;
    }
}

#endif
