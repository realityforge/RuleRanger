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
#include "RuleRangerMatcher.h"
#include "UObject/Object.h"
#include "NotMatcher.generated.h"

/**
 * Matcher that performs a boolean NOT operation on contained matchers and only returns true if one of the child
 * matchers returns true and stops evaluating at that point.
 */
UCLASS()
class RULERANGER_API UNotMatcher final : public URuleRangerMatcher
{
    GENERATED_BODY()

    /** The matchers to perform logical NOT operation on. */
    UPROPERTY(Instanced, EditAnywhere, meta = (AllowAbstract = "false", ForceShowPluginContent = "true"))
    TObjectPtr<URuleRangerMatcher> Matcher;

public:
    virtual bool Test_Implementation(UObject* Object) const override;
};
