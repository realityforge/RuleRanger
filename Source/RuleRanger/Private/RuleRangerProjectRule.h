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
#include "Engine/DataAsset.h"
#include "RuleRangerProjectRule.generated.h"

class URuleRangerProjectActionContext;
class URuleRangerProjectAction;
class FObjectPreSaveContext;

/**
 * The object that binds one or more matchers with one or more actions.
 */
UCLASS(AutoExpandCategories = ("Default", "Rule Ranger"), EditInlineNew)
class URuleRangerProjectRule final : public UDataAsset
{
    GENERATED_BODY()

public:
    /**
     * An explanation of the rule.
     */
    UPROPERTY(EditAnywhere, Category = "Default", meta = (MultiLine))
    FString Description{ TEXT("") };

    /**
     * True to apply this rule when explicitly requested from the editor UI.
     */
    UPROPERTY(EditAnywhere, Category = "Default")
    bool bApplyOnDemand{ true };

    /**
     * A flag that controls whether the presence of an error will result in subsequent actions being skipped if an error
     * is detected.
     */
    UPROPERTY(EditAnywhere, Category = "Default")
    bool bContinueOnError{ false };

    /** The actions that will be applied by the rule. */
    UPROPERTY(Instanced,
              EditAnywhere,
              Category = "Rule Ranger",
              meta = (AllowAbstract = "false", ForceShowPluginContent = "true"))
    TArray<TObjectPtr<URuleRangerProjectAction>> Actions;

    /**
     * Apply the actions associated with the rule.
     *
     * @param ActionContext the context in which the actions are invoked.
     */
    void Apply(URuleRangerProjectActionContext* ActionContext);

    // Clean up invalid entries before saving
    virtual void PreSave(FObjectPreSaveContext SaveContext) override;

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
