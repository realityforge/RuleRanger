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
#include "RuleRangerResultHandler.generated.h"

class URuleRangerActionContext;

UINTERFACE(MinimalAPI)
class URuleRangerResultHandler : public UInterface
{
    GENERATED_BODY()
};

/**
 * The interface to receive results from RuleRanger analysis.
 */
class IRuleRangerResultHandler
{
    GENERATED_BODY()

public:
    /**
     * A callback that is invoked after a Rule has been applied to an Asset.
     * Th supplied ActionContext contains the accumulated state gathered during the action but will be reset after the
     * callback returns. No reference should be held to the ActionContext after the callback returns.
     *
     * @param InActionContext The ActionContext after the rule has been applied.  The ActionState and Messages have been
     * set or added by this time.
     */
    virtual void OnRuleApplied(URuleRangerActionContext* InActionContext) = 0;
};
