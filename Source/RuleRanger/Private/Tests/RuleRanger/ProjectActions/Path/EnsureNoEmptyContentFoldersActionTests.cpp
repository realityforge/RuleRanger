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

    #include "HAL/FileManager.h"
    #include "Misc/AutomationTest.h"
    #include "Misc/FileHelper.h"
    #include "Misc/Paths.h"
    #include "RuleRanger/ProjectActions/Path/EnsureNoEmptyContentFoldersAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureNoEmptyContentFoldersActionTests
{
    struct FScopedContentFolderSandbox
    {
        static constexpr TCHAR SandboxScopeName[] = TEXT("EnsureNoEmptyContentFolders");

        FString FolderName;
        FString RootFileSystemPath;
        FString RootMountPath;

        explicit FScopedContentFolderSandbox(const TCHAR* const NameSuffix)
        {
            FolderName = FString::Printf(TEXT("RuleRangerEmptyFolderTests_%s"), NameSuffix);
            RootFileSystemPath =
                FPaths::Combine(RuleRangerTests::GetRuleRangerTestContentRoot(), SandboxScopeName, FolderName);
            RootMountPath = FString::Printf(TEXT("%s/%s/%s"),
                                            RuleRangerTests::GetRuleRangerTestMountRoot(),
                                            SandboxScopeName,
                                            *FolderName);

            IFileManager::Get().DeleteDirectory(*RootFileSystemPath, false, true);
            IFileManager::Get().MakeDirectory(*RootFileSystemPath, true);

            // Keep the sandbox root compliant so only the targeted leaf folders participate in assertions.
            RuleRangerTests::NewPackagedMaterial(*FString::Printf(TEXT("%s/KeepAliveAsset"), *RootMountPath),
                                                 TEXT("KeepAliveAsset"));
        }

        ~FScopedContentFolderSandbox() { IFileManager::Get().DeleteDirectory(*RootFileSystemPath, false, true); }

        FString MakeDirectory(const TCHAR* const RelativePath) const
        {
            const auto DirectoryPath = FPaths::Combine(RootFileSystemPath, RelativePath);
            IFileManager::Get().MakeDirectory(*DirectoryPath, true);
            return DirectoryPath;
        }

        FString MakeMountPath(const TCHAR* const RelativePath) const
        {
            if (nullptr != RelativePath && TEXT('\0') != RelativePath[0])
            {
                FString Relative(RelativePath);
                Relative.ReplaceInline(TEXT("\\"), TEXT("/"));
                return FString::Printf(TEXT("%s/%s"), *RootMountPath, *Relative);
            }
            else
            {
                return RootMountPath;
            }
        }

        FString WriteFile(const TCHAR* const RelativeDirectory, const TCHAR* const Filename) const
        {
            const auto DirectoryPath = MakeDirectory(RelativeDirectory);
            const auto FilePath = FPaths::Combine(DirectoryPath, Filename);
            FFileHelper::SaveStringToFile(TEXT("orphan"), *FilePath);
            return FilePath;
        }

        UObject* RegisterAsset(const TCHAR* const RelativeDirectory, const TCHAR* const AssetName) const
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            MakeDirectory(RelativeDirectory);
            const auto PackagePath = FString::Printf(TEXT("%s/%s/%s"), *RootMountPath, RelativeDirectory, AssetName);
            return RuleRangerTests::NewPackagedMaterial(*PackagePath, AssetName);
        }
    };

    void AppendExcludedSiblingMounts(const FString& RootFileSystemPath,
                                     const FString& RootMountPath,
                                     const FString& IncludedDirectoryName,
                                     TArray<FString>& ExcludedPaths)
    {
        TArray<FString> Directories;
        IFileManager::Get().FindFiles(Directories, *FPaths::Combine(RootFileSystemPath, TEXT("*")), false, true);

        for (const auto& Directory : Directories)
        {
            if (!Directory.Equals(IncludedDirectoryName, ESearchCase::CaseSensitive))
            {
                ExcludedPaths.Add(FString::Printf(TEXT("%s/%s"), *RootMountPath, *Directory));
            }
        }
    }

    void ConfigureAction(UEnsureNoEmptyContentFoldersAction* const Action, const FScopedContentFolderSandbox& Sandbox)
    {
        Action->bScanPluginContent = false;

        TArray<FString> ExcludedPaths;
        AppendExcludedSiblingMounts(FPaths::ProjectContentDir(), TEXT("/Game"), TEXT("Developers"), ExcludedPaths);
        AppendExcludedSiblingMounts(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Developers")),
                                    TEXT("/Game/Developers"),
                                    TEXT("Tests"),
                                    ExcludedPaths);
        AppendExcludedSiblingMounts(FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Developers/Tests")),
                                    TEXT("/Game/Developers/Tests"),
                                    TEXT("RuleRanger"),
                                    ExcludedPaths);
        AppendExcludedSiblingMounts(RuleRangerTests::GetRuleRangerTestContentRoot(),
                                    RuleRangerTests::GetRuleRangerTestMountRoot(),
                                    FScopedContentFolderSandbox::SandboxScopeName,
                                    ExcludedPaths);
        AppendExcludedSiblingMounts(FPaths::Combine(RuleRangerTests::GetRuleRangerTestContentRoot(),
                                                    FScopedContentFolderSandbox::SandboxScopeName),
                                    FString::Printf(TEXT("%s/%s"),
                                                    RuleRangerTests::GetRuleRangerTestMountRoot(),
                                                    FScopedContentFolderSandbox::SandboxScopeName),
                                    Sandbox.FolderName,
                                    ExcludedPaths);
        Action->ExcludedPaths = MoveTemp(ExcludedPaths);
    }
} // namespace RuleRangerEnsureNoEmptyContentFoldersActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNoEmptyContentFoldersActionDefaultsTest,
                                 "RuleRanger.ProjectActions.Path.EnsureNoEmptyContentFolders.Defaults",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNoEmptyContentFoldersActionDefaultsTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNoEmptyContentFoldersAction>();
    if (TestNotNull(TEXT("EnsureNoEmptyContentFoldersAction should be created"), Action))
    {
        return TestTrue(TEXT("Plugin content scanning should default to enabled"), Action->bScanPluginContent)
            && TestTrue(TEXT("Default exclusions should contain /Game/Collections"),
                        Action->ExcludedPaths.Contains(TEXT("/Game/Collections")))
            && TestTrue(TEXT("Default exclusions should contain /Game/Developers"),
                        Action->ExcludedPaths.Contains(TEXT("/Game/Developers")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNoEmptyContentFoldersActionDryRunReportsEmptyLeafTest,
                                 "RuleRanger.ProjectActions.Path.EnsureNoEmptyContentFolders.DryRunReportsEmptyLeaf",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNoEmptyContentFoldersActionDryRunReportsEmptyLeafTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    RuleRangerEnsureNoEmptyContentFoldersActionTests::FScopedContentFolderSandbox Sandbox(TEXT("DryRunEmpty"));
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNoEmptyContentFoldersAction>();
    const auto LeafDirectory = Sandbox.MakeDirectory(TEXT("LeafEmpty"));
    const auto LeafMountPath = Sandbox.MakeMountPath(TEXT("LeafEmpty"));

    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureNoEmptyContentFoldersAction should be created"), Action))
    {
        RuleRangerEnsureNoEmptyContentFoldersActionTests::ConfigureAction(Action, Sandbox);
        Action->Apply(Fixture.ActionContext);

        return TestEqual(TEXT("Dry run should report one empty leaf folder"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention the empty mount path"),
                                                      *LeafMountPath)
            && TestTrue(TEXT("Dry run should leave the leaf directory on disk"),
                        IFileManager::Get().DirectoryExists(*LeafDirectory));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureNoEmptyContentFoldersActionDryRunReportsFilesWithoutAssetsTest,
    "RuleRanger.ProjectActions.Path.EnsureNoEmptyContentFolders.DryRunReportsFilesWithoutAssets",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNoEmptyContentFoldersActionDryRunReportsFilesWithoutAssetsTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    RuleRangerEnsureNoEmptyContentFoldersActionTests::FScopedContentFolderSandbox Sandbox(TEXT("DryRunFiles"));
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNoEmptyContentFoldersAction>();
    const auto LeafDirectory = Sandbox.MakeDirectory(TEXT("LeafFiles"));
    const auto LeafMountPath = Sandbox.MakeMountPath(TEXT("LeafFiles"));
    // ReSharper disable once CppExpressionWithoutSideEffects
    Sandbox.WriteFile(TEXT("LeafFiles"), TEXT("orphan.txt"));

    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureNoEmptyContentFoldersAction should be created"), Action))
    {
        RuleRangerEnsureNoEmptyContentFoldersActionTests::ConfigureAction(Action, Sandbox);
        Action->Apply(Fixture.ActionContext);

        return TestEqual(TEXT("Dry run should report folders that have files but no assets"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention files on disk"),
                                                      TEXT("files on disk"))
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention the affected mount path"),
                                                      *LeafMountPath)
            && TestTrue(TEXT("Dry run should leave the directory with files on disk"),
                        IFileManager::Get().DirectoryExists(*LeafDirectory));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNoEmptyContentFoldersActionFixDeletesEmptyLeafTest,
                                 "RuleRanger.ProjectActions.Path.EnsureNoEmptyContentFolders.FixDeletesEmptyLeaf",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNoEmptyContentFoldersActionFixDeletesEmptyLeafTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    RuleRangerEnsureNoEmptyContentFoldersActionTests::FScopedContentFolderSandbox Sandbox(TEXT("FixDeletes"));
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNoEmptyContentFoldersAction>();
    const auto LeafDirectory = Sandbox.MakeDirectory(TEXT("LeafDelete"));
    const auto LeafMountPath = Sandbox.MakeMountPath(TEXT("LeafDelete"));

    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture, ERuleRangerProjectActionTrigger::AT_Fix)
        && TestNotNull(TEXT("EnsureNoEmptyContentFoldersAction should be created"), Action))
    {
        RuleRangerEnsureNoEmptyContentFoldersActionTests::ConfigureAction(Action, Sandbox);
        Action->Apply(Fixture.ActionContext);

        return TestTrue(TEXT("Fix mode should not add errors for a deletable empty folder"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("Fix mode should emit one informational deletion message"),
                         Fixture.ActionContext->GetInfoMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetInfoMessages(),
                                                      TEXT("The info should mention the deleted mount path"),
                                                      *LeafMountPath)
            && TestFalse(TEXT("Fix mode should delete the empty leaf directory"),
                         IFileManager::Get().DirectoryExists(*LeafDirectory));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureNoEmptyContentFoldersActionFixRejectsFoldersWithFilesTest,
    "RuleRanger.ProjectActions.Path.EnsureNoEmptyContentFolders.FixRejectsFoldersWithFiles",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNoEmptyContentFoldersActionFixRejectsFoldersWithFilesTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    RuleRangerEnsureNoEmptyContentFoldersActionTests::FScopedContentFolderSandbox Sandbox(TEXT("FixFiles"));
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNoEmptyContentFoldersAction>();
    const auto LeafDirectory = Sandbox.MakeDirectory(TEXT("LeafFiles"));
    const auto LeafMountPath = Sandbox.MakeMountPath(TEXT("LeafFiles"));
    // ReSharper disable once CppExpressionWithoutSideEffects
    Sandbox.WriteFile(TEXT("LeafFiles"), TEXT("orphan.txt"));

    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture, ERuleRangerProjectActionTrigger::AT_Fix)
        && TestNotNull(TEXT("EnsureNoEmptyContentFoldersAction should be created"), Action))
    {
        RuleRangerEnsureNoEmptyContentFoldersActionTests::ConfigureAction(Action, Sandbox);
        Action->Apply(Fixture.ActionContext);

        return TestEqual(TEXT("Fix mode should add one error when files block deletion"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should explain that deletion is blocked"),
                                                      TEXT("cannot remove"))
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention the blocked mount path"),
                                                      *LeafMountPath)
            && TestTrue(TEXT("Fix mode should leave folders with files on disk"),
                        IFileManager::Get().DirectoryExists(*LeafDirectory));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureNoEmptyContentFoldersActionAcceptsFoldersWithRegisteredAssetsTest,
    "RuleRanger.ProjectActions.Path.EnsureNoEmptyContentFolders.AcceptsFoldersWithRegisteredAssets",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNoEmptyContentFoldersActionAcceptsFoldersWithRegisteredAssetsTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    RuleRangerEnsureNoEmptyContentFoldersActionTests::FScopedContentFolderSandbox Sandbox(TEXT("HasAssets"));
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNoEmptyContentFoldersAction>();
    const auto LeafDirectory = Sandbox.MakeDirectory(TEXT("LeafWithAsset"));
    const auto RegisteredAsset = Sandbox.RegisterAsset(TEXT("LeafWithAsset"), TEXT("LeafWithAssetMaterial"));

    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureNoEmptyContentFoldersAction should be created"), Action)
        && TestNotNull(TEXT("A registered asset should be created for the leaf folder"), RegisteredAsset))
    {
        RuleRangerEnsureNoEmptyContentFoldersActionTests::ConfigureAction(Action, Sandbox);
        Action->Apply(Fixture.ActionContext);

        return TestTrue(TEXT("Folders with registered assets should not add errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestTrue(TEXT("Folders with registered assets should remain on disk"),
                        IFileManager::Get().DirectoryExists(*LeafDirectory));
    }
    else
    {
        return false;
    }
}

#endif
