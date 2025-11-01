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
#include "EnforceGameplayTagRemapsSortedAction.h"
#include "GameplayTagsSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnforceGameplayTagRemapsSortedAction)

static bool IsSortedAscending(const TArray<FGameplayTagCategoryRemap>& Remaps)
{
    if (Remaps.Num() < 2)
    {
        return true;
    }
    else
    {
        auto Prev = Remaps[0].BaseCategory;
        for (auto i = 1; i < Remaps.Num(); ++i)
        {
            const auto& Curr = Remaps[i].BaseCategory;
            if (Curr < Prev)
            {
                return false;
            }
            Prev = Curr;
        }
        return true;
    }
}

void UEnforceGameplayTagRemapsSortedAction::Apply(URuleRangerProjectActionContext* ActionContext)
{
    auto const Settings = GetMutableDefault<UGameplayTagsSettings>();
    if (IsValid(Settings))
    {
        auto& Remaps = Settings->CategoryRemapping;

        // Compute desired state: keep only last occurrence for each BaseCategory, then sort ascending by BaseCategory
        TMap<FString, int32> RemapMap;
        RemapMap.Reserve(Remaps.Num());
        for (int32 i = 0; i < Remaps.Num(); ++i)
        {
            RemapMap.Add(Remaps[i].BaseCategory, i); // overwrite to keep last
        }

        const int32 DuplicateCount = Remaps.Num() - RemapMap.Num();

        TArray<FGameplayTagCategoryRemap> Unique;
        Unique.Reserve(RemapMap.Num());
        for (const auto& Pair : RemapMap)
        {
            Unique.Add(Remaps[Pair.Value]);
        }
        Unique.Sort([](const auto& A, const auto& B) { return A.BaseCategory < B.BaseCategory; });

        if (0 != DuplicateCount || !IsSortedAscending(Remaps))
        {
            if (ActionContext->IsDryRun())
            {
                if (DuplicateCount > 0)
                {
                    ActionContext->Error(FText::Format(NSLOCTEXT("RuleRanger",
                                                                 "GameplayTagsRemapsHaveDuplicates",
                                                                 "Gameplay Tag CategoryRemapping has {0} duplicate "
                                                                 "entr{0}|plural(one=y,other=ies)."),
                                                       FText::AsNumber(DuplicateCount)));
                }
                if (!IsSortedAscending(Remaps))
                {
                    ActionContext->Error(NSLOCTEXT("RuleRanger",
                                                   "GameplayTagsRemapsUnsorted",
                                                   "Gameplay Tag CategoryRemapping is not sorted "
                                                   "by BaseCategory (ascending)."));
                }
            }
            else
            {
                Remaps = MoveTemp(Unique);
                Settings->SaveConfig();
                ActionContext->Info(FText::Format(NSLOCTEXT("RuleRanger",
                                                            "GameplayTagsRemapsSorted",
                                                            "Sorted Gameplay Tag CategoryRemapping and removed "
                                                            "{0} duplicate entr{0}|plural(one=y,other=ies)."),
                                                  FText::AsNumber(DuplicateCount)));
            }
        }
    }
    else
    {
        ActionContext->Fatal(NSLOCTEXT("RuleRanger",
                                       "GameplayTagsSettingsMissing_Sort",
                                       "Unable to access UGameplayTagsSettings to sort CategoryRemapping."));
    }
}
