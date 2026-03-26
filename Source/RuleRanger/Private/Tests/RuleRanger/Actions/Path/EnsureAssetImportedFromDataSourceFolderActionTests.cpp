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
    #include "RuleRanger/Actions/Path/EnsureAssetImportedFromDataSourceFolderAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureAssetImportedFromDataSourceFolderErrorsWhenFolderMissingTest,
    "RuleRanger.Actions.Path.EnsureAssetImportedFromDataSourceFolder.ErrorsWhenFolderMissing",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureAssetImportedFromDataSourceFolderErrorsWhenFolderMissingTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureAssetImportedFromDataSourceFolderAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationImportDataObject>(TEXT("/Game/Characters/SM_Hero"),
                                                                                  TEXT("SM_Hero"));
    RuleRangerTests::FScopedDataSourceFolderOverride DataSourceFolder(TEXT(""));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureAssetImportedFromDataSourceFolderAction should be created"), Action)
        && TestNotNull(TEXT("Import-data object should be created"), Object))
    {
        Action->Apply(Fixture.ActionContext, Object);
        return TestEqual(TEXT("Missing Data Source Folder should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(
                   *this,
                   Fixture.ActionContext->GetErrorMessages(),
                   TEXT("Missing Data Source Folder errors should explain the issue"),
                   TEXT("Data Source Folder not set"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureAssetImportedFromDataSourceFolderSkipsMissingImportDataByDefaultTest,
    "RuleRanger.Actions.Path.EnsureAssetImportedFromDataSourceFolder.SkipsMissingImportDataByDefault",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureAssetImportedFromDataSourceFolderSkipsMissingImportDataByDefaultTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureAssetImportedFromDataSourceFolderAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationImportDataObject>(TEXT("/Game/Characters/SM_Hero"),
                                                                                  TEXT("SM_Hero"));
    RuleRangerTests::FScopedDataSourceFolderOverride DataSourceFolder(TEXT("C:/SourceArt"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureAssetImportedFromDataSourceFolderAction should be created"), Action)
        && TestNotNull(TEXT("Import-data object should be created"), Object))
    {
        Action->Apply(Fixture.ActionContext, Object);
        return TestTrue(TEXT("Missing AssetImportData should be skipped by default"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureAssetImportedFromDataSourceFolderErrorsForMissingImportDataWhenConfiguredTest,
    "RuleRanger.Actions.Path.EnsureAssetImportedFromDataSourceFolder.ErrorsForMissingImportDataWhenConfigured",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureAssetImportedFromDataSourceFolderErrorsForMissingImportDataWhenConfiguredTest::RunTest(
    const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureAssetImportedFromDataSourceFolderAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationImportDataObject>(TEXT("/Game/Characters/SM_Hero"),
                                                                                  TEXT("SM_Hero"));
    RuleRangerTests::FScopedDataSourceFolderOverride DataSourceFolder(TEXT("C:/SourceArt"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureAssetImportedFromDataSourceFolderAction should be created"), Action)
        && TestNotNull(TEXT("Import-data object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Action, TEXT("bSkipAssetsMissingAssetImportData"), false))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestEqual(TEXT("Configured missing AssetImportData failures should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Missing AssetImportData failures should mention the setting"),
                       TEXT("SkipAssetsMissingAssetImportData=false"));
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
    FRuleRangerEnsureAssetImportedFromDataSourceFolderSkipsObjectsOutsideGameTest,
    "RuleRanger.Actions.Path.EnsureAssetImportedFromDataSourceFolder.SkipsObjectsOutsideGame",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureAssetImportedFromDataSourceFolderSkipsObjectsOutsideGameTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureAssetImportedFromDataSourceFolderAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationImportDataObject>(TEXT("/Engine/Characters/SM_Hero"),
                                                                                  TEXT("SM_Hero"));
    RuleRangerTests::FScopedDataSourceFolderOverride DataSourceFolder(TEXT("C:/SourceArt"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureAssetImportedFromDataSourceFolderAction should be created"), Action)
        && TestNotNull(TEXT("Import-data object should be created"), Object))
    {
        if (RuleRangerTests::SetImportFilename(*this, Object, TEXT("C:/SourceArt/Characters/SM_Hero.fbx")))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestTrue(TEXT("Objects outside /Game should be skipped"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty());
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
    FRuleRangerEnsureAssetImportedFromDataSourceFolderErrorsForImportsOutsideFolderTest,
    "RuleRanger.Actions.Path.EnsureAssetImportedFromDataSourceFolder.ErrorsForImportsOutsideFolder",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureAssetImportedFromDataSourceFolderErrorsForImportsOutsideFolderTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureAssetImportedFromDataSourceFolderAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationImportDataObject>(TEXT("/Game/Characters/SM_Hero"),
                                                                                  TEXT("SM_Hero"));
    RuleRangerTests::FScopedDataSourceFolderOverride DataSourceFolder(TEXT("C:/SourceArt"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureAssetImportedFromDataSourceFolderAction should be created"), Action)
        && TestNotNull(TEXT("Import-data object should be created"), Object))
    {
        if (RuleRangerTests::SetImportFilename(*this, Object, TEXT("C:/Other/Characters/SM_Hero.fbx")))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestEqual(TEXT("Imports outside the Data Source Folder should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Outside-folder import failures should mention the Data Source Folder"),
                       TEXT("outside of the Data Source Folder"));
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
    FRuleRangerEnsureAssetImportedFromDataSourceFolderErrorsForNameMismatchesTest,
    "RuleRanger.Actions.Path.EnsureAssetImportedFromDataSourceFolder.ErrorsForNameMismatches",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureAssetImportedFromDataSourceFolderErrorsForNameMismatchesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureAssetImportedFromDataSourceFolderAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationImportDataObject>(TEXT("/Game/Characters/SM_Wrong"),
                                                                                  TEXT("SM_Wrong"));
    RuleRangerTests::FScopedDataSourceFolderOverride DataSourceFolder(TEXT("C:/SourceArt"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureAssetImportedFromDataSourceFolderAction should be created"), Action)
        && TestNotNull(TEXT("Import-data object should be created"), Object))
    {
        if (RuleRangerTests::SetImportFilename(*this, Object, TEXT("C:/SourceArt/Characters/SM_Hero.fbx")))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestEqual(TEXT("Name mismatches should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("Name mismatches should mention the asset name"),
                                                          TEXT("does not match the asset name"));
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
    FRuleRangerEnsureAssetImportedFromDataSourceFolderErrorsForPathMismatchesTest,
    "RuleRanger.Actions.Path.EnsureAssetImportedFromDataSourceFolder.ErrorsForPathMismatches",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureAssetImportedFromDataSourceFolderErrorsForPathMismatchesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureAssetImportedFromDataSourceFolderAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationImportDataObject>(TEXT("/Game/Props/SM_Hero"),
                                                                                  TEXT("SM_Hero"));
    RuleRangerTests::FScopedDataSourceFolderOverride DataSourceFolder(TEXT("C:/SourceArt"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureAssetImportedFromDataSourceFolderAction should be created"), Action)
        && TestNotNull(TEXT("Import-data object should be created"), Object))
    {
        if (RuleRangerTests::SetImportFilename(*this, Object, TEXT("C:/SourceArt/Characters/SM_Hero.fbx")))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestEqual(TEXT("Path mismatches should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("Path mismatches should mention the asset path"),
                                                          TEXT("does not match the asset path"));
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
    FRuleRangerEnsureAssetImportedFromDataSourceFolderCanSkipPathAndNameChecksWhenDisabledTest,
    "RuleRanger.Actions.Path.EnsureAssetImportedFromDataSourceFolder.CanSkipPathAndNameChecksWhenDisabled",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureAssetImportedFromDataSourceFolderCanSkipPathAndNameChecksWhenDisabledTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureAssetImportedFromDataSourceFolderAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationImportDataObject>(TEXT("/Game/Props/SM_Wrong"),
                                                                                  TEXT("SM_Wrong"));
    RuleRangerTests::FScopedDataSourceFolderOverride DataSourceFolder(TEXT("C:/SourceArt"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureAssetImportedFromDataSourceFolderAction should be created"), Action)
        && TestNotNull(TEXT("Import-data object should be created"), Object))
    {
        if (RuleRangerTests::SetImportFilename(*this, Object, TEXT("C:/SourceArt/Characters/SM_Hero.fbx"))
            && RuleRangerTests::SetPropertyValue(*this, Action, TEXT("bRequireMatchingName"), false)
            && RuleRangerTests::SetPropertyValue(*this, Action, TEXT("bRequireMatchingPath"), false))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestTrue(TEXT("Disabling path and name checks should allow otherwise mismatched imports"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty());
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

#endif
