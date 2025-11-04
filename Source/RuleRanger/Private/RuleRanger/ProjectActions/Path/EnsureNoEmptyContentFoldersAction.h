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

#pragma once

#include "CoreMinimal.h"
#include "RuleRangerProjectAction.h"
#include "EnsureNoEmptyContentFoldersAction.generated.h"

class IAssetRegistry;

// Map: list all directories and process deepest-first
struct FDirEntry
{
    FString FileSystemPath; // absolute path
    FString MountPath;      // content path e.g. /Game/Foo/Bar
    int32 Depth{ 0 };       // depth by number of '/'
};

struct FContentRoot
{
    FString FileSystemRoot; // Absolute path to .../Content
    FString MountRoot;      // "/Game" or "/<PluginName>"
};

/**
 * Project action that ensures content folders are non-empty.
 *
 * - Scans Project Content and, optionally, enabled Plugin Content folders.
 * - A folder is compliant if it contains at least one asset or it contains subfolders.
 * - In Fix mode, deletes leaf folders that have no assets only if truly empty on disk (no files present).
 * - Supports excluding specific content paths (and their subtrees) from analysis.
 */
UCLASS(DisplayName = "Ensure No Empty Content Folders", CollapseCategories, DefaultToInstanced, EditInlineNew)
class UEnsureNoEmptyContentFoldersAction final : public URuleRangerProjectAction
{
    GENERATED_BODY()

    static void ProcessDirEntry(URuleRangerProjectActionContext* ActionContext,
                                const IAssetRegistry& AssetRegistry,
                                const FDirEntry& Dir);
    static FString DeriveMountPath(FString Rel, const FString& MountRoot);

    static bool HasAnyFilesNonRecursive(const FString& Dir);

    void GenerateDirectoriesForRoot(const FContentRoot& Root, TArray<FDirEntry>& Candidates) const;

    void ProcessContentRoot(URuleRangerProjectActionContext* ActionContext, const FContentRoot& Root) const;

public:
    UEnsureNoEmptyContentFoldersAction();

    /** Exclude these content paths (and sub-directories) from analysis. Example: /Game/Developer, /MyPlugin/Skip */
    UPROPERTY(EditAnywhere, Category = "Default")
    TArray<FString> ExcludedPaths;

    /** Also scan enabled plugins' Content folders. */
    UPROPERTY(EditAnywhere, Category = "Default")
    bool bScanPluginContent{ true };

    virtual void Apply(URuleRangerProjectActionContext* ActionContext) override;
};
