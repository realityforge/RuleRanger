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
#include "RuleRangerActionContext.h"
#include "UObject/Object.h"
#include "ImportActionContext.generated.h"

/**
 * ActionContext implementation used during the import process.
 */
UCLASS(Blueprintable, BlueprintType)
class RULERANGER_API UImportActionContext : public UObject, public IRuleRangerActionContext
{
    GENERATED_BODY()

public:
    virtual bool InErrorState() override;
    virtual ERuleRangerActionTrigger GetActionTrigger() override;

    void ResetContext(UObject* InObject, ERuleRangerActionTrigger InActionTrigger);

private:
    /** The object that the associated action is acting upon. */
    UPROPERTY(VisibleAnywhere)
    UObject* Object;

    /** The reason that the associated action was triggered. */
    UPROPERTY(VisibleAnywhere)
    ERuleRangerActionTrigger ActionTrigger{ ERuleRangerActionTrigger::AT_Max };

    /** Flag indicating the action triggered an error state. */
    UPROPERTY(VisibleAnywhere)
    bool bError;
};
