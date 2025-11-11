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
 * Registers a dockable Nomad tab for the RuleRanger Tool.
 */
class FRuleRangerToolTab
{
public:
    /** Register the tab spawner. */
    static void Initialize();

    /** Unregister the tab spawner. */
    static void Shutdown();

    /** The unique tab name for the RuleRanger Tool. */
    static FName GetTabName();
};
