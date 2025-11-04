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
#include "RuleRangerExclusionSet.h"
#include "Misc/DataValidation.h"
#include "RuleRanger/RuleRangerSortUtils.h"
#include "RuleRangerRule.h"
#include "RuleRangerRuleExclusion.h"
#include "RuleRangerRuleSet.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RuleRangerExclusionSet)

void URuleRangerExclusionSet::PostLoad()
{
    Super::PostLoad();
    UpdateExclusionsEditorFriendlyTitles();
}

void URuleRangerExclusionSet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property)
    {
        const auto PropertyName = PropertyChangedEvent.Property->GetFName();
        if ((GET_MEMBER_NAME_CHECKED(ThisClass, Exclusions)) == PropertyName)
        {
            UpdateExclusionsEditorFriendlyTitles();
        }
    }
}

void URuleRangerExclusionSet::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);

    const auto PropertyName = PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue()->GetFName();
    if ((GET_MEMBER_NAME_CHECKED(ThisClass, Exclusions)) == PropertyName)
    {
        UpdateExclusionsEditorFriendlyTitles();
    }
}

void URuleRangerExclusionSet::UpdateExclusionsEditorFriendlyTitles()
{
#if WITH_EDITORONLY_DATA
    for (auto& Exclusion : Exclusions)
    {
        Exclusion.InitEditorFriendlyTitleProperty();
    }
#endif
}

void URuleRangerExclusionSet::PreSave(const FObjectPreSaveContext SaveContext)
{
    using namespace RuleRanger::SortUtils;

    // Clean and sort exclusions' arrays
    for (auto& Exclusion : Exclusions)
    {
        RemoveNullsAndSortByName<URuleRangerRuleSet>(Exclusion.RuleSets);
        RemoveNullsAndSortByName<URuleRangerRule>(Exclusion.Rules);
        RemoveNullsAndSortByName<UObject>(Exclusion.Objects);
        SortDirsByPath(Exclusion.Dirs);

        // Ensure exclusion directory paths end with '/'
        for (auto& Dir : Exclusion.Dirs)
        {
            if (!Dir.Path.IsEmpty() && !Dir.Path.EndsWith(TEXT("/")))
            {
                Dir.Path.Append(TEXT("/"));
            }
        }
    }

    Super::PreSave(SaveContext);
}

#if WITH_EDITOR
EDataValidationResult URuleRangerExclusionSet::IsDataValid(FDataValidationContext& Context) const
{
    auto Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

    if (Description.IsEmptyOrWhitespace())
    {
        Context.AddError(NSLOCTEXT("RuleRanger",
                                   "ExclusionSetMissingDescription",
                                   "RuleRangerExclusionSet requires a Description."));
        Result = EDataValidationResult::Invalid;
    }

    if (Exclusions.IsEmpty())
    {
        Context.AddError(NSLOCTEXT("RuleRanger",
                                   "ExclusionSetEmptyExclusions",
                                   "RuleRangerExclusionSet must contain at least one exclusion."));
        Result = EDataValidationResult::Invalid;
    }

    // Validate each exclusion has both a rule specifier and a target specifier
    int32 Index{ 0 };
    for (const auto& Exclusion : Exclusions)
    {
        const auto Label = FString::Printf(TEXT("Exclusions[%d]"), Index);
        Result = CombineDataValidationResults(Exclusion.IsDataValid(Label, Context), Result);
        Index++;
    }

    return Result;
}
#endif
