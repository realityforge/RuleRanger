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
#include "UObject/Interface.h"
#include "RuleRangerActionContext.generated.h"

UENUM(BlueprintType)
enum class ERuleRangerActionTrigger : uint8
{
    /** The action was invoked as a result of an import. */
    AT_Import UMETA(DisplayName = "Trigger"),
    /** The action was invoked as a result of a re-import. */
    AT_Reimport UMETA(DisplayName = "ReImport"),
    /** The action was invoked as a result of a validate. */
    AT_Validate UMETA(DisplayName = "Validate"),
    /** The action was invoked as an explicit action from a user. */
    AT_Explicit UMETA(DisplayName = "Explicit"),

    AT_Max UMETA(Hidden)
};

UINTERFACE(MinimalAPI, NotBlueprintable)
class URuleRangerActionContext : public UInterface
{
    GENERATED_BODY()
};

/**
 * Context object passed to an action so that the action can be provided context.
 */
class RULERANGER_API IRuleRangerActionContext
{
    GENERATED_BODY()

public:
    /**
     * Return true if the ActionContext is in the "error" state.
     * This means that one of the error methods have been invoked.
     *
     * @return true if the ActionContext is in the "error" state, otherwise false.
     */
    UFUNCTION(BlueprintCallable)
    virtual bool InErrorState() = 0;

    /**
     * Return the trigger for the current action. i.e. Why was this action invoked.
     *
     * @return the trigger for the current action
     */
    virtual ERuleRangerActionTrigger GetActionTrigger() = 0;

    // TODO: In the future this will provider the ability to pass back validation
    // failures as well as info, warning and error messages
    // TODO: Also some processes we want to provide "linting" and just generate warnings
    // while in some scenarios we want to auto-apply fixes.
    // TODO: Also provide access to flag indicating whether there are any fatal errors
    // (and thus can skip further actions until earlier errors addressed)
};
