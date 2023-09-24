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
#include "ActionContext.generated.h"

/**
 * Context object passed to an action so that the action can be provided context.
 */
UCLASS()
class RULERANGER_API UActionContext : public UObject
{
    GENERATED_BODY()

    // TODO: In the future this will provider the ability to pass back validation
    // failures as well as info, warning and error messages
    // TODO: This will also provide context. i.e. is the action applied due to explicit
    // call, import, reimport or save action?
};
