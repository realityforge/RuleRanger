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
#include "EnforceGameplayTagRemapPresentAction.h"
#include "GameplayTagsSettings.h"
#include "Logging/StructuredLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnforceGameplayTagRemapPresentAction)

namespace
{
    static FText MakeMissingMessage(const FString& Name)
    {
        return FText::Format(NSLOCTEXT("RuleRanger",
                                       "MissingGameplayTagRemap",
                                       "Gameplay Tag CategoryRemapping is missing an entry for '{0}'."),
                             FText::FromString(Name));
    }

    static FText MakeAddedMessage(const FString& Name, const TArray<FString>& Targets)
    {
        return FText::Format(NSLOCTEXT("RuleRanger",
                                       "AddedGameplayTagRemap",
                                       "Added Gameplay Tag CategoryRemapping for '{0}' with targets: {1}"),
                             FText::FromString(Name),
                             FText::FromString(FString::Join(Targets, TEXT(", "))));
    }
} // namespace

void UEnforceGameplayTagRemapPresentAction::Apply(URuleRangerProjectActionContext* ActionContext)
{
    if (Name.TrimStartAndEnd().IsEmpty())
    {
        ActionContext->Error(NSLOCTEXT("RuleRanger",
                                       "GameplayTagRemapMissingName",
                                       "EnforceGameplayTagRemapPresentAction requires Name to be set."));
    }
    else
    {
        auto const Settings = GetMutableDefault<UGameplayTagsSettings>();
        if (!IsValid(Settings))
        {
            ActionContext->Fatal(NSLOCTEXT("RuleRanger",
                                           "GameplayTagsSettingsMissing",
                                           "Unable to access UGameplayTagsSettings to enforce CategoryRemapping."));
        }
        else
        {
            auto& Remaps = Settings->CategoryRemapping;
            if (!Remaps.FindByPredicate([&](const auto& Remap) { return Remap.BaseCategory == Name; }))
            {
                if (ActionContext->IsDryRun())
                {
                    ActionContext->Error(MakeMissingMessage(Name));
                }
                else
                {
                    if (DefaultTargets.Num() <= 0)
                    {
                        // No fix strategy configured
                        ActionContext->Error(MakeMissingMessage(Name));
                    }
                    else
                    {
                        // Create the remap using defaults
                        FGameplayTagCategoryRemap NewRemap;
                        NewRemap.BaseCategory = Name;
                        for (const auto& Target : DefaultTargets)
                        {
                            if (!Target.TrimStartAndEnd().IsEmpty())
                            {
                                NewRemap.RemapCategories.Add(Target);
                            }
                        }

                        Remaps.Add(MoveTemp(NewRemap));
                        Settings->SaveConfig();

                        ActionContext->Info(MakeAddedMessage(Name, DefaultTargets));
                    }
                }
            }
        }
    }
}
