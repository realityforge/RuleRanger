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

class IContentBrowserSingleton;
struct FAssetData;
class SRuleRangerToolPanel;
class UToolMenu;

/**
 * Provides various utility methods when implementing tools.
 */
class FRuleRangerTools
{
public:
    static bool HasAnyRunnableRules();
    static bool HasAnyConfiguredDirs();
    static bool HasAnyProjectRules();

    static void OnScanContent();
    static void OnFixContent();

    static void OnScanProject();
    static void OnFixProject();

    static void OnScanAll();
    static void OnFixAll();

    static void OnOpenProjectSettings();
    static void OnOpenTool();

    static void OnScanSelected();
    static void OnFixSelected();

    static void OnScanSelectedAssets(const TArray<FAssetData>& Assets);
    static void OnFixSelectedAssets(const TArray<FAssetData>& Assets);

    static bool AssetSelectionIntersectsConfiguredDirs(const TArray<FAssetData>& Assets);

    static void OnScanSelectedPaths(const TArray<FString>& AssetPaths);
    static void OnFixSelectedPaths(const TArray<FString>& AssetPaths);

    static bool PathSelectionIntersectsConfiguredDirs(const TArray<FString>& Paths);

    static bool SelectionIntersectsConfiguredDirs();

    static void CollectAllSelectedAssets(TArray<FAssetData>& OutAssets);
    static void CollectAllConfiguredAssets(TArray<FAssetData>& OutAssets);

private:
    static void OnScanSelectedAssets();
    static void OnFixSelectedAssets();

    static void OnScanSelectedPaths();
    static void OnFixSelectedPaths();

    static bool AssetSelectionIntersectsConfiguredDirs();
    static bool PathSelectionIntersectsConfiguredDirs();

    static void GetSelectedAssets(TArray<FAssetData>& OutAssets);
    static void GetSelectedPaths(TArray<FString>& OutPaths);

    static IContentBrowserSingleton* GetContentBrowser();
};
