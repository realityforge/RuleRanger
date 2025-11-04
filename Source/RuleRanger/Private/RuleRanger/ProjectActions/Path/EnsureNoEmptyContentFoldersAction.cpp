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

#include "EnsureNoEmptyContentFoldersAction.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFile.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureNoEmptyContentFoldersAction)

namespace
{
    static FString NormalizeMountPath(const FString& Path)
    {
        auto OutPath{ Path };
        // Normalize slashes, ensure leading '/'
        FPaths::NormalizeFilename(OutPath);
        if (!OutPath.StartsWith(TEXT("/")))
        {
            OutPath = TEXT("/") + OutPath;
        }
        // Remove trailing slash
        while (OutPath.Len() > 1 && (OutPath.EndsWith(TEXT("/")) || OutPath.EndsWith(TEXT("\\"))))
        {
            OutPath.LeftChopInline(1, EAllowShrinking::No);
        }
        return OutPath;
    }

    static FString WithTrailingSlash(const FString& Path)
    {
        return Path.IsEmpty() || Path.EndsWith(TEXT("/")) ? Path : Path + TEXT("/");
    }

    static bool IsExcludedPath(const FString& MountPath, const TArray<FString>& RawExcludes)
    {
        const auto Candidate = WithTrailingSlash(NormalizeMountPath(MountPath));
        for (const auto& Raw : RawExcludes)
        {
            if (Candidate.StartsWith(WithTrailingSlash(NormalizeMountPath(Raw))))
            {
                return true;
            }
        }
        return false;
    }

    static void CollectContentRoots(const bool bScanPlugins, TArray<FContentRoot>& Out)
    {
        FContentRoot Project;
        Project.FileSystemRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
        FPaths::NormalizeDirectoryName(Project.FileSystemRoot);
        Project.MountRoot = TEXT("/Game");
        Out.Add(MoveTemp(Project));

        if (bScanPlugins)
        {
            for (const auto& Plugin : IPluginManager::Get().GetDiscoveredPlugins())
            {
                if (Plugin->CanContainContent() && Plugin->IsEnabled()
                    && EPluginLoadedFrom::Engine != Plugin->GetLoadedFrom())
                {
                    const FString ContentDir = Plugin->GetContentDir();
                    if (!ContentDir.IsEmpty())
                    {
                        auto RootPath = FPaths::ConvertRelativePathToFull(ContentDir);
                        FPaths::NormalizeDirectoryName(RootPath);
                        if (IFileManager::Get().DirectoryExists(*RootPath))
                        {
                            auto Mount = Plugin->GetMountedAssetPath(); // Typically "/<PluginName>"
                            if (Mount.IsEmpty())
                            {
                                Mount = TEXT("/") + Plugin->GetName();
                            }

                            FContentRoot Root;
                            Root.FileSystemRoot = MoveTemp(RootPath);
                            Root.MountRoot = NormalizeMountPath(Mount);
                            Out.Add(MoveTemp(Root));
                        }
                    }
                }
            }
        }
    }

    static void CollectAllSubdirectories(const FString& RootDir, TArray<FString>& OutDirs)
    {
        // Non-recursive stack-based traversal to collect every subdirectory under RootDir
        TArray<FString> Pending;
        Pending.Add(RootDir);

        while (Pending.Num() > 0)
        {
            const auto Dir{ Pending.Pop(EAllowShrinking::No) };

            // List immediate children; collect subdirectories
            class FVisitor final : public IPlatformFile::FDirectoryVisitor
            {
            public:
                TArray<FString>& PendingRef;
                explicit FVisitor(TArray<FString>& InPending) : PendingRef(InPending) {}
                virtual bool Visit(const TCHAR* FilenameOrDirectory, const bool bIsDirectory) override
                {
                    if (bIsDirectory)
                    {
                        PendingRef.Add(FilenameOrDirectory);
                    }
                    return true;
                }
            } Visitor(Pending);

            IFileManager::Get().IterateDirectory(*Dir, Visitor);

            if (Dir != RootDir)
            {
                OutDirs.Add(Dir);
            }
        }
    }

    static bool HasSubdirectories(const FString& Dir)
    {
        bool bHasSubdir = false;
        class FVisitor final : public IPlatformFile::FDirectoryVisitor
        {
        public:
            bool& bFoundRef;
            explicit FVisitor(bool& InFound) : bFoundRef(InFound) {}
            virtual bool Visit(const TCHAR* FilenameOrDirectory, const bool bIsDirectory) override
            {
                if (bIsDirectory)
                {
                    bFoundRef = true;
                    return false; // early out
                }
                return true;
            }
        } Visitor(bHasSubdir);

        IFileManager::Get().IterateDirectory(*Dir, Visitor);
        return bHasSubdir;
    }
} // namespace

UEnsureNoEmptyContentFoldersAction::UEnsureNoEmptyContentFoldersAction()
{
    // Sensible defaults commonly present in UE projects
    ExcludedPaths.Add(TEXT("/Game/Collections"));
    ExcludedPaths.Add(TEXT("/Game/Developers"));
}

void UEnsureNoEmptyContentFoldersAction::ProcessDirEntry(URuleRangerProjectActionContext* ActionContext,
                                                         const IAssetRegistry& AssetRegistry,
                                                         const FDirEntry& Dir)
{
    // A folder may have been removed due to a prior deletion
    // If there are any subdirectories, this folder is compliant regardless of assets
    if (IFileManager::Get().DirectoryExists(*Dir.FileSystemPath) && !HasSubdirectories(Dir.FileSystemPath))
    {
        // Check for at least one asset directly under this folder (non-recursive)
        TArray<FAssetData> Assets;
        AssetRegistry.GetAssetsByPath(*Dir.MountPath, Assets, /*bRecursive=*/false);

        if (0 == Assets.Num())
        {
            if (ActionContext->IsDryRun())
            {
                ActionContext->Error(
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "EmptyContentFolderDetected",
                                            "Empty content folder detected: {0}. "
                                            "Folder must contain at least one asset or have subfolders."),
                                  FText::FromString(Dir.MountPath)));
            }
            // Fix: only remove directory if there are no files present on the filesystem
            else if (HasAnyFilesNonRecursive(Dir.FileSystemPath))
            {
                ActionContext->Error(
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "CannotDeleteNonEmptyFolderOnDisk",
                                            "Folder {0} has files on disk but no assets; cannot remove."),
                                  FText::FromString(Dir.MountPath)));
            }
            // Attempt removal (should succeed for an empty directory)
            else if (IFileManager::Get().DeleteDirectory(*Dir.FileSystemPath,
                                                         /*RequireExists=*/false,
                                                         /*Tree=*/false))
            {
                ActionContext->Info(
                    FText::Format(NSLOCTEXT("RuleRanger", "DeletedEmptyFolder", "Deleted empty content folder {0}"),
                                  FText::FromString(Dir.MountPath)));
            }
            else
            {
                ActionContext->Error(FText::Format(
                    NSLOCTEXT("RuleRanger", "FailedToDeleteEmptyFolder", "Failed to delete empty content folder {0}"),
                    FText::FromString(Dir.MountPath)));
            }
        }
    }
}

bool UEnsureNoEmptyContentFoldersAction::HasAnyFilesNonRecursive(const FString& Dir)
{
    bool bHasFile = false;
    class FVisitor final : public IPlatformFile::FDirectoryVisitor
    {
    public:
        bool& bFoundRef;
        explicit FVisitor(bool& InFound) : bFoundRef(InFound) {}
        virtual bool Visit(const TCHAR* FilenameOrDirectory, const bool bIsDirectory) override
        {
            if (!bIsDirectory)
            {
                bFoundRef = true;
                return false;
            }
            return true;
        }
    } Visitor(bHasFile);

    IFileManager::Get().IterateDirectory(*Dir, Visitor);
    return bHasFile;
}

FString UEnsureNoEmptyContentFoldersAction::DeriveMountPath(FString Rel, const FString& MountRoot)
{
    auto MountPath{ MountRoot };
    if (!Rel.IsEmpty())
    {
        // Normalize directory name and join
        FPaths::NormalizeDirectoryName(Rel);
        Rel.ReplaceInline(TEXT("\\"), TEXT("/"));
        Rel.RightChopInline(8, EAllowShrinking::No);
        MountPath = MountRoot + TEXT("/") + Rel;
    }
    MountPath = NormalizeMountPath(MountPath);
    return MountPath;
}

void UEnsureNoEmptyContentFoldersAction::GenerateDirectoriesForRoot(const FContentRoot& Root,
                                                                    TArray<FDirEntry>& Candidates) const
{
    Candidates.Reserve(128);

    TArray<FString> AbsSubDirPaths;
    CollectAllSubdirectories(Root.FileSystemRoot, AbsSubDirPaths);

    for (const auto& AbsPath : AbsSubDirPaths)
    {
        auto Rel{ AbsPath };
        // Determine mount path under this root
        if (FPaths::MakePathRelativeTo(Rel, *Root.FileSystemRoot))
        {
            const auto MountPath{ DeriveMountPath(Rel, Root.MountRoot) };

            if (!IsExcludedPath(MountPath, ExcludedPaths))
            {
                auto SlashCount{ 0 };
                for (auto i = 0; i < MountPath.Len(); ++i)
                {
                    if (MountPath[i] == TEXT('/'))
                    {
                        ++SlashCount;
                    }
                }

                FDirEntry Entry;
                Entry.FileSystemPath = AbsPath;
                Entry.MountPath = MountPath;
                Entry.Depth = SlashCount;
                Candidates.Add(MoveTemp(Entry));
            }
        }
    }
}

void UEnsureNoEmptyContentFoldersAction::ProcessContentRoot(URuleRangerProjectActionContext* ActionContext,
                                                            const FContentRoot& Root) const
{
    TArray<FDirEntry> Candidates;
    GenerateDirectoriesForRoot(Root, Candidates);

    // Leaf-first: process deepest directories first
    Candidates.Sort([](const auto& A, const auto& B) { return A.Depth > B.Depth; });

    const auto& AssetRegistry =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName).Get();

    for (const auto& Dir : Candidates)
    {
        ProcessDirEntry(ActionContext, AssetRegistry, Dir);
    }
}

void UEnsureNoEmptyContentFoldersAction::Apply(URuleRangerProjectActionContext* ActionContext)
{
    // Gather content roots to scan (project + optional plugin content)
    TArray<FContentRoot> Roots;
    CollectContentRoots(bScanPluginContent, Roots);

    for (const auto& Root : Roots)
    {
        if (IFileManager::Get().DirectoryExists(*Root.FileSystemRoot))
        {
            ProcessContentRoot(ActionContext, Root);
        }
    }
}
