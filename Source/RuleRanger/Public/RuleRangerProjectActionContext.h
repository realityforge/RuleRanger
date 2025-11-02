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
#include "RuleRangerProjectActionContext.generated.h"

class URuleRangerRuleSet;
class URuleRangerConfig;
class URuleRangerProjectRule;

UENUM()
enum class ERuleRangerProjectActionTrigger : uint8
{
    /** The action was invoked as an explicit "report" action from a user. */
    AT_Report UMETA(DisplayName = "Report"),
    /** The action was invoked as an explicit "fix" action from a user. */
    AT_Fix UMETA(DisplayName = "Fix"),

    AT_Max UMETA(Hidden)
};

/**
 * Context object passed to a project action so that the action can be provided context.
 */
UCLASS(MinimalAPI, Transient)
class URuleRangerProjectActionContext final : public URuleRangerCommonContext
{
    GENERATED_BODY()
    friend class URuleRangerEditorSubsystem;
    friend class URuleRangerEditorValidator;
    friend class URuleRangerCommandlet;

protected:
    virtual void ClearContext() override;

private:
    void ResetContext(URuleRangerConfig* const InConfig,
                      URuleRangerRuleSet* const InRuleSet,
                      URuleRangerProjectRule* InRule,
                      ERuleRangerProjectActionTrigger InActionTrigger);

    /** The rule that contains the associated action that is using the context. */
    UPROPERTY(Transient)
    TObjectPtr<URuleRangerProjectRule> Rule{ nullptr };

    /** The reason that the associated action was triggered. */
    UPROPERTY(VisibleAnywhere)
    ERuleRangerProjectActionTrigger ActionTrigger{ ERuleRangerProjectActionTrigger::AT_Max };

public:
    FORCEINLINE const URuleRangerProjectRule* GetRule() const { return Rule; }

    /**
     * Return true if this action is a "Dry" run and should just issue warnings on non-compliance
     * and info on action that would take to fix compliance.
     *
     * @return true if the action should be a dry run.
     */
    FORCEINLINE bool IsDryRun() const { return ERuleRangerProjectActionTrigger::AT_Fix != ActionTrigger; }

    /**
     * Return the trigger for the current action. i.e. Why was this action invoked.
     *
     * @return the trigger for the current action
     */
    FORCEINLINE ERuleRangerProjectActionTrigger GetActionTrigger() const { return ActionTrigger; }
};
