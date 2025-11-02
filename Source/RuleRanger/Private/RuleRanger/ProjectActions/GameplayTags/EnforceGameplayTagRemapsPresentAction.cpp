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
#include "EnforceGameplayTagRemapsPresentAction.h"
#include "GameplayTagsSettings.h"
#include "Logging/StructuredLog.h"
#include "RuleRangerConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnforceGameplayTagRemapsPresentAction)

void UEnforceGameplayTagRemapsPresentAction::Apply(URuleRangerProjectActionContext* ActionContext)
{
    UGameplayTagsSettings* const Settings = GetMutableDefault<UGameplayTagsSettings>();
    if (!IsValid(Settings))
    {
        ActionContext->Fatal(NSLOCTEXT("RuleRanger",
                                       "GameplayTagsSettingsMissing_Multiple",
                                       "Unable to access UGameplayTagsSettings to enforce CategoryRemapping."));
        return;
    }

    // Collect expected mappings from inline property and DataTables (including from Config)
    TMap<FString, TArray<FString>> Expected; // BaseCategory -> Targets

    for (const auto& Remap : Remaps)
    {
        if (!Remap.BaseCategory.TrimStartAndEnd().IsEmpty())
        {
            Expected.Add(Remap.BaseCategory, Remap.RemapCategories);
        }
    }

    TArray<TObjectPtr<UDataTable>> Tables;
    Tables.Append(RemapTables);
    if (const auto Config = ActionContext->GetConfig())
    {
        Config->CollectDataTables(FRuleRangerTagCategoryRow::StaticStruct(), Tables);
    }

    for (const auto& Table : Tables)
    {
        if (IsValid(Table))
        {
            for (const auto& RowPair : Table->GetRowMap())
            {
                const auto BaseCategory = RowPair.Key.ToString();
                if (const auto Row = reinterpret_cast<const FRuleRangerTagCategoryRow*>(RowPair.Value))
                {
                    if (!BaseCategory.TrimStartAndEnd().IsEmpty())
                    {
                        Expected.Add(BaseCategory, Row->DefaultTargets);
                    }
                }
            }
        }
    }

    if (0 == Expected.Num())
    {
        // Nothing to enforce
        return;
    }

    auto& RemapsRef = Settings->CategoryRemapping;

    // Build set of existing categories (case-sensitive)
    TSet<FString> Existing;
    Existing.Reserve(RemapsRef.Num());
    for (const auto& Remap : RemapsRef)
    {
        Existing.Add(Remap.BaseCategory);
    }

    for (const auto& Pair : Expected)
    {
        const auto& BaseCategory = Pair.Key;
        if (!Existing.Contains(BaseCategory))
        {
            if (ActionContext->IsDryRun())
            {
                ActionContext->Error(
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "MissingGameplayTagRemap_Multiple",
                                            "Gameplay Tag CategoryRemapping is missing an entry for '{0}'."),
                                  FText::FromString(BaseCategory)));
            }
            else
            {
                FGameplayTagCategoryRemap NewRemap;
                NewRemap.BaseCategory = BaseCategory;
                for (const auto& Target : Pair.Value)
                {
                    if (!Target.TrimStartAndEnd().IsEmpty())
                    {
                        NewRemap.RemapCategories.Add(Target);
                    }
                }
                RemapsRef.Add(MoveTemp(NewRemap));
                Settings->SaveConfig();
                ActionContext->Info(
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "AddedGameplayTagRemap_Multiple",
                                            "Added Gameplay Tag CategoryRemapping for '{0}' with targets: {1}"),
                                  FText::FromString(BaseCategory),
                                  FText::FromString(FString::Join(Pair.Value, TEXT(", ")))));
            }
        }
    }
}
