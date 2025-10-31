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
#include "RuleRangerObjectBase.h"
#include "RuleRangerProjectActionContext.h"
#include "RuleRangerProjectAction.generated.h"

/**
 * Base class used to apply an action across a project.
 * This is typically used in the body of a rule.
 */
UCLASS(Abstract, CollapseCategories, DefaultToInstanced, EditInlineNew)
class RULERANGER_API URuleRangerProjectAction : public URuleRangerObjectBase
{
    GENERATED_BODY()

    friend class URuleRangerRule;

public:
    /**
     * Apply the action.
     *
     * @param ActionContext the context in which the action is invoked.
     */
    virtual void Apply(URuleRangerProjectActionContext* ActionContext);
};
