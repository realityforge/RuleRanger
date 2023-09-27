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
#include "Engine/DataTable.h"
#include "NameConvention.generated.h"

USTRUCT(BlueprintType)
struct FNameConvention : public FTableRowBase
{
    GENERATED_BODY()

    /** The object type that this name convention applied to. */
    UPROPERTY(EditAnywhere, meta = (AllowAbstract))
    TSoftClassPtr<UObject> ObjectType{ nullptr };

    /**
     * The variant/subtype to add to the name (if any).
     * This is expected to be derived before applying this rule.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Variant{ TEXT("*") };

    /** The prefix to add to the name (if any). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Prefix{ TEXT("") };

    /** The suffix to add to the name (if any). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Suffix{ TEXT("") };
};
