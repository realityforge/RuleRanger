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
#include "RuleRangerCommonContext.h"
#include "UObject/Interface.h"
#include "RuleRangerActionContext.generated.h"

class URuleRangerRuleSet;
class URuleRangerConfig;
class URuleRangerRule;

UENUM()
enum class ERuleRangerActionTrigger : uint8
{
    /** The action was invoked as a result of an import. */
    AT_Import UMETA(DisplayName = "Import"),
    /** The action was invoked as a result of a re-import. */
    AT_Reimport UMETA(DisplayName = "ReImport"),
    /** The action was invoked as a result of a validate. */
    AT_Validate UMETA(DisplayName = "Validate"),
    /** The action was invoked as a result of a save. */
    AT_Save UMETA(DisplayName = "Save"),
    /** The action was invoked as an explicit "report" action from a user. */
    AT_Report UMETA(DisplayName = "Report"),
    /** The action was invoked as an explicit "fix" action from a user. */
    AT_Fix UMETA(DisplayName = "Fix"),

    AT_Max UMETA(Hidden)
};

/**
 * Context object passed to an action so that the action can be provided context.
 */
UCLASS(MinimalAPI, Transient)
class URuleRangerActionContext final : public URuleRangerCommonContext
{
    GENERATED_BODY()
    friend class URuleRangerEditorSubsystem;
    friend class URuleRangerEditorValidator;

protected:
    void ResetContext(URuleRangerConfig* const InConfig,
                      URuleRangerRuleSet* const InRuleSet,
                      URuleRangerRule* InRule,
                      UObject* InObject,
                      ERuleRangerActionTrigger InActionTrigger);
    virtual void ClearContext() override;

private:
    /** The rule that contains the associated action that is using the context. */
    UPROPERTY(Transient)
    TObjectPtr<URuleRangerRule> Rule{ nullptr };

    /** The object that the associated action is acting upon. */
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UObject> Object{ nullptr };

    /** The reason that the associated action was triggered. */
    UPROPERTY(VisibleAnywhere)
    ERuleRangerActionTrigger ActionTrigger{ ERuleRangerActionTrigger::AT_Max };

public:
    FORCEINLINE const URuleRangerRule* GetRule() const { return Rule; }
    FORCEINLINE const UObject* GetObject() const { return Object; }

    /**
     * Return true if this action is a "Dry" run and should just issue warnings on non-compliance
     * and info on action that would take to fix compliance.
     *
     * @return true if the action should be a dry run.
     */
    FORCEINLINE bool IsDryRun() const
    {
        return !(ERuleRangerActionTrigger::AT_Save == ActionTrigger
                 || ERuleRangerActionTrigger::AT_Import == ActionTrigger
                 || ERuleRangerActionTrigger::AT_Reimport == ActionTrigger
                 || ERuleRangerActionTrigger::AT_Fix == ActionTrigger);
    }

    /**
     * Return the trigger for the current action. i.e. Why was this action invoked.
     *
     * @return the trigger for the current action
     */
    FORCEINLINE ERuleRangerActionTrigger GetActionTrigger() const { return ActionTrigger; }
};
