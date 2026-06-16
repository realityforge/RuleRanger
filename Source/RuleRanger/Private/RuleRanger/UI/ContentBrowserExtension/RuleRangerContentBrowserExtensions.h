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

#include "ContentBrowserDelegates.h"
#include "CoreMinimal.h"

class FExtender;
struct FAssetData;

/** The class responsible for managing MessageLog categories used in RuleRanger plugin. */
class FRuleRangerContentBrowserExtensions final
{
public:
    static void Initialize();

    static void Shutdown();

#if WITH_DEV_AUTOMATION_TESTS
    static TSharedRef<FExtender> CreateSelectedPathsMenuExtenderForTest(const TArray<FString>& Paths);

    static TSharedRef<FExtender> CreateSelectedAssetsMenuExtenderForTest(const TArray<FAssetData>& Assets);

    static bool AreExtendersRegisteredForTest();
#endif

private:
    // Delegate called when extending ContextBrowser SelectedPaths Context menu
    static FContentBrowserMenuExtender_SelectedPaths SelectedPathsDelegate;
    static FDelegateHandle SelectedPathsDelegateHandle;

    // Delegate called when extending ContextBrowser SelectedAssets Context menu
    static FContentBrowserMenuExtender_SelectedAssets SelectedAssetsDelegate;
    static FDelegateHandle SelectedAssetsDelegateHandle;
};
