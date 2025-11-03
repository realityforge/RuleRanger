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
#include "RuleRangerAction.h"
#include "RunDataValidationAction.generated.h"

/**
 * Action that runs UE Data Validation (IsDataValid) on the target object
 * and reports validation warnings/errors as RuleRanger warnings/errors.
 */
UCLASS(DisplayName = "Run Data Validation")
class URunDataValidationAction final : public URuleRangerAction
{
    GENERATED_BODY()

public:
    virtual void Apply(URuleRangerActionContext* ActionContext, UObject* Object) override;
};
