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
#include "EnsureMetaSoundAuthorBlankAction.generated.h"

/**
 * Action that ensures that the Author field of a MetaSoundSource is blank.
 */
UCLASS(DisplayName = "Ensure MetaSound Author Blank")
class RULERANGER_API UEnsureMetaSoundAuthorBlankAction final : public URuleRangerAction
{
    GENERATED_BODY()

protected:
    virtual UClass* GetExpectedType() override;

    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;
};
