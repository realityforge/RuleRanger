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
#include "RuleRangerCommonContext.generated.h"

class URuleRangerRuleSet;
class URuleRangerConfig;

UENUM()
enum class ERuleRangerActionState : uint8
{
    /** The action was successful and produced no warnings or errors. */
    AS_Success UMETA(DisplayName = "Success"),
    /** The action produced one or more warnings but no errors. */
    AS_Warning UMETA(DisplayName = "Warning"),
    /** The action produced one or more errors but the errors are not fatal. */
    AS_Error UMETA(DisplayName = "Error"),
    /** The action produced one or more fatal errors and further rule processing should not occur. */
    AS_Fatal UMETA(DisplayName = "Fatal"),

    AS_Max UMETA(Hidden)
};

/**
 * Base class for context object passed to actions to provide the action contextual information.
 */
UCLASS(MinimalAPI, Abstract, Transient)
class URuleRangerCommonContext : public UObject
{
    GENERATED_BODY()
    friend class URuleRangerEditorSubsystem;
    friend class URuleRangerEditorValidator;

public:
    /**
     * Generate an informational message from action.
     *
     * @param InMessage the message.
     */
    RULERANGER_API void Info(const FText& InMessage);

    /**
     * Generate a warning message from the action.
     *
     * @param InMessage the message.
     */
    RULERANGER_API void Warning(const FText& InMessage);

    /**
     * Generate an error message from the action.
     *
     * @param InMessage the message.
     */
    RULERANGER_API void Error(const FText& InMessage);

    /**
     * Generate an error message from the action.
     *
     * @param InMessage the message.
     */
    RULERANGER_API void Fatal(const FText& InMessage);

protected:
    void ResetContext(URuleRangerConfig* const InConfig, URuleRangerRuleSet* const InRuleSet);
    virtual void ClearContext();

private:
    /** A reference to the RuleRangerConfig that transitively included this Action. */
    UPROPERTY(Transient)
    TObjectPtr<URuleRangerConfig> Config{ nullptr };

    /** A reference to the RuleRangerRuleSet that directly included this Action. */
    UPROPERTY(Transient)
    TObjectPtr<URuleRangerRuleSet> RuleSet{ nullptr };

    /** Current state of the action. */
    UPROPERTY(VisibleAnywhere)
    ERuleRangerActionState ActionState{ ERuleRangerActionState::AS_Max };

    /** The array of info messages. */
    UPROPERTY(VisibleAnywhere)
    TArray<FText> InfoMessages;

    /** The array of warning messages. */
    UPROPERTY(VisibleAnywhere)
    TArray<FText> WarningMessages;

    /** The array of error messages. */
    UPROPERTY(VisibleAnywhere)
    TArray<FText> ErrorMessages;

    /** The array of fatal messages. */
    UPROPERTY(VisibleAnywhere)
    TArray<FText> FatalMessages;

public:
    FORCEINLINE const URuleRangerConfig* GetConfig() const { return Config; }
    FORCEINLINE const URuleRangerRuleSet* GetRuleSet() const { return RuleSet; }

    /**
     * Return the current state of the action.
     *
     * @return the current state of the action.
     */
    FORCEINLINE ERuleRangerActionState GetState() const { return ActionState; }

    FORCEINLINE const TArray<FText>& GetInfoMessages() const { return InfoMessages; }
    FORCEINLINE const TArray<FText>& GetWarningMessages() const { return WarningMessages; }
    FORCEINLINE const TArray<FText>& GetErrorMessages() const { return ErrorMessages; }
    FORCEINLINE const TArray<FText>& GetFatalMessages() const { return FatalMessages; }
};
