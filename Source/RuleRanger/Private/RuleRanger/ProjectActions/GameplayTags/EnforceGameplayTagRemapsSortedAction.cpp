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

namespace
{
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
} // namespace

void UEnforceGameplayTagRemapsSortedAction::Apply(URuleRangerProjectActionContext* ActionContext)
{
    auto const Settings = GetMutableDefault<UGameplayTagsSettings>();
    if (IsValid(Settings))
    {
        auto& Remaps = Settings->CategoryRemapping;

        // Clean up whitespace-only entries and remove remaps that end up empty after cleaning.
        int32 EmptyRemovedCount = 0;
        int32 WhitespaceRemovedCount = 0;
        int32 CategoryDupesRemovedCount = 0;
        int32 CategoriesUnsortedCount = 0;
        int32 NonEmptyCount = 0;
        TMap<FString, FGameplayTagCategoryRemap> RemapMap; // BaseCategory -> last cleaned remap
        RemapMap.Reserve(Remaps.Num());
        for (int32 i = 0; i < Remaps.Num(); ++i)
        {
            const auto& R = Remaps[i];
            FGameplayTagCategoryRemap CleanRemap;
            CleanRemap.BaseCategory = R.BaseCategory;
            CleanRemap.RemapCategories.Reserve(R.RemapCategories.Num());
            TSet<FString> SeenCats;
            for (const auto& Cat : R.RemapCategories)
            {
                const auto Trimmed = Cat.TrimStartAndEnd();
                if (!Trimmed.IsEmpty())
                {
                    if (!SeenCats.Contains(Trimmed))
                    {
                        CleanRemap.RemapCategories.Add(Trimmed);
                        SeenCats.Add(Trimmed);
                    }
                    else
                    {
                        ++CategoryDupesRemovedCount;
                    }
                }
                else
                {
                    ++WhitespaceRemovedCount;
                }
            }
            if (CleanRemap.RemapCategories.Num() <= 0)
            {
                ++EmptyRemovedCount;
                continue;
            }
            else
            {
                // Would sorting categories alphabetically change order?
                auto Sorted = CleanRemap.RemapCategories;
                Sorted.Sort([](const auto& A, const auto& B) { return A < B; });
                if (Sorted != CleanRemap.RemapCategories)
                {
                    ++CategoriesUnsortedCount;
                }
            }
            ++NonEmptyCount;
            RemapMap.Add(CleanRemap.BaseCategory, MoveTemp(CleanRemap)); // keep last
        }

        const int32 DuplicateCount = FMath::Max(0, NonEmptyCount - RemapMap.Num());

        // Build the new list from the last non-empty cleaned occurrences, then sort ascending by BaseCategory
        TArray<FGameplayTagCategoryRemap> Unique;
        Unique.Reserve(RemapMap.Num());
        for (auto& Pair : RemapMap)
        {
            Unique.Add(MoveTemp(Pair.Value));
        }
        Unique.Sort([](const auto& A, const auto& B) { return A.BaseCategory < B.BaseCategory; });

        const bool bUnsorted = !IsSortedAscending(Remaps);
        const bool bHasIssues = (DuplicateCount > 0) || bUnsorted || (EmptyRemovedCount > 0)
            || (WhitespaceRemovedCount > 0) || (CategoryDupesRemovedCount > 0) || (CategoriesUnsortedCount > 0);

        if (bHasIssues)
        {
            if (ActionContext->IsDryRun())
            {
                if (WhitespaceRemovedCount > 0)
                {
                    ActionContext->Error(
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "GameplayTagsRemapsHaveWhitespaceEntries",
                                                "Gameplay Tag CategoryRemapping has {0} whitespace-only "
                                                "ent{0}|plural(one=ry,other=ries) within RemapCategories; "
                                                "these would be removed if in Fix mode."),
                                      FText::AsNumber(WhitespaceRemovedCount)));
                }
                if (EmptyRemovedCount > 0)
                {
                    ActionContext->Error(
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "GameplayTagsRemapsHaveEmptyEntries",
                                                "Gameplay Tag CategoryRemapping has {0} "
                                                "entr{0}|plural(one=y,other=ies) with an empty RemapCategories list; "
                                                "these would be removed if in Fix mode."),
                                      FText::AsNumber(EmptyRemovedCount)));
                }
                if (CategoryDupesRemovedCount > 0)
                {
                    ActionContext->Error(
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "GameplayTagsRemapsHaveCategoryDuplicates",
                                                "Gameplay Tag CategoryRemapping has {0} duplicate "
                                                "ent{0}|plural(one=ry,other=ries) within RemapCategories lists; "
                                                "these would be removed if in Fix mode."),
                                      FText::AsNumber(CategoryDupesRemovedCount)));
                }
                if (DuplicateCount > 0)
                {
                    ActionContext->Error(FText::Format(
                        NSLOCTEXT("RuleRanger",
                                  "GameplayTagsRemapsHaveDuplicates",
                                  "Gameplay Tag CategoryRemapping has {0} duplicate entr{0}|plural(one=y,other=ies)."),
                        FText::AsNumber(DuplicateCount)));
                }
                if (DuplicateCount > 0)
                {
                    ActionContext->Error(FText::Format(
                        NSLOCTEXT("RuleRanger",
                                  "GameplayTagsRemapsHaveDuplicates",
                                  "Gameplay Tag CategoryRemapping has {0} duplicate entr{0}|plural(one=y,other=ies)."),
                        FText::AsNumber(DuplicateCount)));
                }
                if (CategoriesUnsortedCount > 0)
                {
                    ActionContext->Error(FText::Format(
                        NSLOCTEXT("RuleRanger",
                                  "GameplayTagsRemapCategoriesUnsorted",
                                  "{0} RemapCategories entr{0}|plural(one=y,other=ies) "
                                  "are not sorted alphabetically and would be re-ordered if in Fix mode."),
                        FText::AsNumber(CategoriesUnsortedCount)));
                }
                if (bUnsorted)
                {
                    ActionContext->Error(
                        NSLOCTEXT("RuleRanger",
                                  "GameplayTagsRemapsUnsorted",
                                  "Gameplay Tag CategoryRemapping is not sorted by BaseCategory (ascending)."));
                }
            }
            else
            {
                // Sort RemapCategories alphabetically within each remap when applying fix
                for (auto& R : Unique)
                {
                    R.RemapCategories.Sort([](const FString& A, const FString& B) { return A < B; });
                }

                Remaps = MoveTemp(Unique);
                Settings->SaveConfig();

                // Report changes
                if (DuplicateCount > 0 || bUnsorted)
                {
                    ActionContext->Info(FText::Format(NSLOCTEXT("RuleRanger",
                                                                "GameplayTagsRemapsSorted",
                                                                "Sorted Gameplay Tag CategoryRemapping and removed {0} "
                                                                "duplicate entr{0}|plural(one=y,other=ies)."),
                                                      FText::AsNumber(DuplicateCount)));
                }
                if (EmptyRemovedCount > 0)
                {
                    ActionContext->Info(FText::Format(NSLOCTEXT("RuleRanger",
                                                                "GameplayTagsRemapsRemovedEmpty",
                                                                "Removed {0} empty Gameplay Tag CategoryRemapping "
                                                                "entr{0}|plural(one=y,other=ies)."),
                                                      FText::AsNumber(EmptyRemovedCount)));
                }
                if (WhitespaceRemovedCount > 0)
                {
                    ActionContext->Info(
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "GameplayTagsRemapsRemovedWhitespaceEntries",
                                                "Removed {0} whitespace-only ent{0}|plural(one=ry,other=ries) "
                                                "from RemapCategories lists."),
                                      FText::AsNumber(WhitespaceRemovedCount)));
                }
                if (CategoryDupesRemovedCount > 0)
                {
                    ActionContext->Info(
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "GameplayTagsRemapsRemovedCategoryDuplicates",
                                                "Removed {0} duplicate ent{0}|plural(one=ry,other=ries) "
                                                "within RemapCategories lists."),
                                      FText::AsNumber(CategoryDupesRemovedCount)));
                }
                if (CategoriesUnsortedCount > 0)
                {
                    ActionContext->Info(
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "GameplayTagsRemapCategoriesSorted",
                                                "Sorted {0} RemapCategories entr{0}|plural(one=y,other=ies) "
                                                "alphabetically."),
                                      FText::AsNumber(CategoriesUnsortedCount)));
                }
                if (CategoryDupesRemovedCount > 0)
                {
                    ActionContext->Info(
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "GameplayTagsRemapsRemovedCategoryDuplicates",
                                                "Removed {0} duplicate ent{0}|plural(one=ry,other=ries) "
                                                "within RemapCategories lists."),
                                      FText::AsNumber(CategoryDupesRemovedCount)));
                }
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
