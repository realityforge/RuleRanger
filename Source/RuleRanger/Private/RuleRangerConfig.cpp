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
#include "RuleRangerConfig.h"
#include "RuleRanger/RuleRangerSortUtils.h"
#include "RuleRangerRuleExclusion.h"
#include "RuleRangerRuleSet.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RuleRangerConfig)

bool URuleRangerConfig::ConfigMatches(const FString& Path)
{
    for (auto Dir = Dirs.CreateIterator(); Dir; ++Dir)
    {
        const auto& DirPath = Dir->Path;
        if (!DirPath.IsEmpty())
        {
            auto Prefix = DirPath;
            if (!Prefix.EndsWith(TEXT("/")))
            {
                Prefix.Append(TEXT("/"));
            }
            if (Path.StartsWith(Prefix, ESearchCase::CaseSensitive) || Path.Equals(DirPath, ESearchCase::CaseSensitive))
            {
                return true;
            }
        }
    }
    return false;
}

void URuleRangerConfig::CollectDataTables(const UScriptStruct* RowStructure,
                                          TArray<TObjectPtr<UDataTable>>& OutDataTables) const
{
    for (const auto DataTable : DataTables)
    {
        if (IsValid(DataTable) && RowStructure == DataTable->RowStruct)
        {
            OutDataTables.Add(DataTable);
        }
    }
    for (const auto RuleSet : RuleSets)
    {
        if (IsValid(RuleSet))
        {
            RuleSet->CollectDataTables(RowStructure, OutDataTables);
        }
    }
}

void URuleRangerConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property)
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto PropertyName = PropertyChangedEvent.Property->GetFName();

        if ((GET_MEMBER_NAME_CHECKED(ThisClass, Exclusions)) == PropertyName)
        {
            UpdateExclusionsEditorFriendlyTitles();
        }
    }
}

void URuleRangerConfig::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto PropertyName = PropertyChangedEvent.PropertyChain.GetActiveMemberNode()->GetValue()->GetFName();

    if ((GET_MEMBER_NAME_CHECKED(ThisClass, Exclusions)) == PropertyName)
    {
        UpdateExclusionsEditorFriendlyTitles();
    }
}

void URuleRangerConfig::PostLoad()
{
    Super::PostLoad();
    UpdateExclusionsEditorFriendlyTitles();
}

void URuleRangerConfig::UpdateExclusionsEditorFriendlyTitles()
{
    for (auto& Exclusion : Exclusions)
    {
        Exclusion.InitEditorFriendlyTitleProperty();
    }
}

void URuleRangerConfig::PreSave(FObjectPreSaveContext SaveContext)
{
    using namespace RuleRanger::SortUtils;

    // Sort config-level directories
    SortDirsByPath(Dirs);

    // Clean and sort data tables
    RemoveNullsAndSortByName(DataTables);

    // Clean and sort exclusions' arrays
    for (auto& Exclusion : Exclusions)
    {
        RemoveNullsAndSortByName(Exclusion.RuleSets);
        RemoveNullsAndSortByName(Exclusion.Rules);
        RemoveNullsAndSortByName(Exclusion.Objects);
        SortDirsByPath(Exclusion.Dirs);
    }

    Super::PreSave(SaveContext);
}
