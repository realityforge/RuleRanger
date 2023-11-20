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
#include "RuleRangerAction.h"
#include "RuleRangerLogging.h"
#include "CheckTextureSubTypePartOfName.generated.h"

/**
 * The various texture subtypes in use.
 */
UENUM(BlueprintType)
enum class ETextureSubType : uint8
{
    AT_Roughness UMETA(DisplayName = "Roughness"),
    AT_Opacity UMETA(DisplayName = "Alpha/Opacity"),
    AT_AmbientOcclusion UMETA(DisplayName = "Ambient Occlusion"),
    AT_BaseColor UMETA(DisplayName = "Base Color"),
    AT_Specular UMETA(DisplayName = "Specular"),
    AT_Metallic UMETA(DisplayName = "Metallic"),
    AT_Normal UMETA(DisplayName = "Normal"),
    AT_Emissive UMETA(DisplayName = "Emissive"),
    AT_Height UMETA(DisplayName = "Height"),
    AT_Mask UMETA(DisplayName = "Mask"),
    AT_FlowMap UMETA(DisplayName = "Flow Map"),
    AT_Displacement UMETA(DisplayName = "Displacement"),
    AT_LightMap UMETA(DisplayName = "Light Map"),
    AT_Unspecified UMETA(DisplayName = "Unspecified"),

    AT_Max UMETA(Hidden)
};

/**
 * The structure defining naming conventions for Texture extensions.
 */
USTRUCT(BlueprintType)
struct FTextureSubTypeNameConvention final : public FTableRowBase
{
    GENERATED_BODY()

    /**
     * The variant/subtype that this rule applies to.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETextureSubType SubType{ ETextureSubType::AT_Unspecified };

    /** The prefix to add to the name (if any). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Text{ TEXT("") };

    bool DoesTextMatchAtIndex(const FString& InText, const int32 Index) const;

    FORCEINLINE bool operator<(const FTextureSubTypeNameConvention& Other) const
    {
        const int LengthDifference = Text.Len() - Other.Text.Len();
        return 0 == LengthDifference ? Text < Other.Text : LengthDifference > 0;
    }
};

/**
 * Action to check that a Texture confirms to desired naming conventions.
 */
UCLASS(AutoExpandCategories = ("Rule Ranger"),
       Blueprintable,
       BlueprintType,
       CollapseCategories,
       DefaultToInstanced,
       EditInlineNew)
class RULERANGER_API UCheckTextureSubTypePartOfName final : public URuleRangerAction
{
    GENERATED_BODY()

public:
    virtual void Apply_Implementation(TScriptInterface<IRuleRangerActionContext>& ActionContext,
                                      UObject* Object) override;

    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:
    /** The table that contains the object naming rules */
    UPROPERTY(EditAnywhere,
              BlueprintReadWrite,
              Category = "Rule Ranger",
              meta = (ExposeOnSpawn,
                      AllowPrivateAccess,
                      RequiredAssetDataTags = "RowStructure=/Script/RuleRanger.TextureSubTypeNameConvention"))
    UDataTable* NameConventionsTable{ nullptr };

    /** Should the action issue a message log when it attempts to process a Texture that has no naming convention? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule Ranger", meta = (ExposeOnSpawn, AllowPrivateAccess))
    bool bNotifyIfNameConventionMissing;

    /** Cache for looking up rules. */
    TArray<FTextureSubTypeNameConvention> NameConventionsCache;

    /** Handle for delegate called when any object modified in editor. */
    FDelegateHandle OnObjectModifiedDelegateHandle;

    /** Callback when any object is modified in the editor. */
    void ResetCacheIfTableModified(UObject* Object);

    /** Method to clear cache. */
    void ResetNameConventionsCache();

    /** Method to build cache if necessary. */
    void RebuildNameConventionsCacheIfNecessary();

    TArray<ETextureSubType> ExtractSubTypes(const FString& Name);
};
