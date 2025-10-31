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

/**
 * Registers the Rule Ranger submenu under the Tools menu with actions to scan/fix configured content.
 */
class FRuleRangerToolsMenu
{
public:
    static void Initialize();
    static void Shutdown();

private:
    static void RegisterMenus();
    static void FillRuleRangerSubMenu(UToolMenu* Menu);

    static void OnScanConfiguredContent();
    static void OnFixConfiguredContent();

    static FDelegateHandle RegisterHandle;
    static int32 OwnerToken;
};
