﻿/*
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
#include "EditorPropertyMatcherBase.h"
#include "UObject/Object.h"
#include "EditorPropertyMatcher.generated.h"

/**
 * Matcher that returns true if object has and editor property with the specified name and value.
 */
UCLASS()
class RULERANGER_API UEditorPropertyMatcher final : public UEditorPropertyMatcherBase
{
    GENERATED_BODY()

    /** The value of the editor property to match. */
    UPROPERTY(EditAnywhere)
    FString Value{ TEXT("") };

protected:
    virtual bool TestEditorProperty(UObject* Object, UObject* Instance, FProperty* Property) const override;
};
