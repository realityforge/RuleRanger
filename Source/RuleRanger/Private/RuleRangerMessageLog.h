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
#include "Styling/SlateStyle.h"

/** The class responsible for managing MessageLog categories used in RuleRanger plugin. */
class FRuleRangerMessageLog final
{
public:
    static void Initialize();

    static void Shutdown();

private:
    const static FName MessageLogName;

public:
    FORCEINLINE static const FName& GetMessageLogName() { return MessageLogName; }
};
