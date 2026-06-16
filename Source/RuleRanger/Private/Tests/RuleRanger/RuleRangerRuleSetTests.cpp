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
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/Blueprint/EnsureDataOnlyBlueprintAction.h"
    #include "RuleRangerRule.h"
    #include "RuleRangerRuleSet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleSetPreSaveCleansAndSortsOnlySortedCollectionsTest,
                                 "RuleRanger.RuleSet.PreSave.CleansAndSortsOnlySortedCollections",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleSetPreSaveCleansAndSortsOnlySortedCollectionsTest::RunTest(const FString&)
{
    const auto RuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>();
    const auto ATable = RuleRangerTests::NewTransientObject<UDataTable>(RuleSet, TEXT("ATable"));
    const auto BTable = RuleRangerTests::NewTransientObject<UDataTable>(RuleSet, TEXT("BTable"));
    const auto FirstNested = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(RuleSet, TEXT("FirstNested"));
    const auto SecondNested = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(RuleSet, TEXT("SecondNested"));
    const auto FirstRule = RuleRangerTests::NewTransientObject<URuleRangerRule>(RuleSet, TEXT("FirstRule"));
    const auto SecondRule = RuleRangerTests::NewTransientObject<URuleRangerRule>(RuleSet, TEXT("SecondRule"));
    if (TestNotNull(TEXT("Rule set should be created"), RuleSet)
        && TestNotNull(TEXT("A table should be created"), ATable)
        && TestNotNull(TEXT("B table should be created"), BTable)
        && TestNotNull(TEXT("First nested rule set should be created"), FirstNested)
        && TestNotNull(TEXT("Second nested rule set should be created"), SecondNested)
        && TestNotNull(TEXT("First rule should be created"), FirstRule)
        && TestNotNull(TEXT("Second rule should be created"), SecondRule))
    {
        RuleSet->DataTables = { BTable, nullptr, ATable };
        RuleSet->RuleSets = { SecondNested, nullptr, FirstNested };
        RuleSet->Rules = { SecondRule, nullptr, FirstRule };

        const RuleRangerTests::FPreSaveContextHolder SaveContext;
        RuleSet->PreSave(SaveContext.Context);

        return TestEqual(TEXT("RuleSet PreSave should remove null data tables"), RuleSet->DataTables.Num(), 2)
            && TestEqual(TEXT("RuleSet PreSave should sort data tables by name"), RuleSet->DataTables[0].Get(), ATable)
            && TestEqual(TEXT("RuleSet PreSave should remove null nested rule sets"), RuleSet->RuleSets.Num(), 2)
            && TestEqual(TEXT("RuleSet PreSave should preserve nested rule set execution order"),
                         RuleSet->RuleSets[0].Get(),
                         SecondNested)
            && TestEqual(TEXT("RuleSet PreSave should remove null rules"), RuleSet->Rules.Num(), 2)
            && TestEqual(TEXT("RuleSet PreSave should preserve rule execution order"),
                         RuleSet->Rules[0].Get(),
                         SecondRule);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleSetCollectDataTablesTraversesNestedSetsAndFiltersRowsTest,
                                 "RuleRanger.RuleSet.CollectDataTables.TraversesNestedSetsAndFiltersRows",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleSetCollectDataTablesTraversesNestedSetsAndFiltersRowsTest::RunTest(const FString&)
{
    const auto RootRuleSet =
        RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(GetTransientPackage(), TEXT("RootRuleSet"));
    const auto NestedRuleSet =
        RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(RootRuleSet, TEXT("NestedRuleSet"));
    const auto DirectMatch = RuleRangerTests::NewTransientObject<UDataTable>(RootRuleSet, TEXT("DirectMatch"));
    const auto DirectWrongRow = RuleRangerTests::NewTransientObject<UDataTable>(RootRuleSet, TEXT("DirectWrongRow"));
    const auto NestedMatch = RuleRangerTests::NewTransientObject<UDataTable>(NestedRuleSet, TEXT("NestedMatch"));
    if (TestNotNull(TEXT("Root rule set should be created"), RootRuleSet)
        && TestNotNull(TEXT("Nested rule set should be created"), NestedRuleSet)
        && TestNotNull(TEXT("Direct matching table should be created"), DirectMatch)
        && TestNotNull(TEXT("Direct wrong-row table should be created"), DirectWrongRow)
        && TestNotNull(TEXT("Nested matching table should be created"), NestedMatch))
    {
        DirectMatch->RowStruct = FDataOnlyBlueprintEntry::StaticStruct();
        DirectWrongRow->RowStruct = FTableRowBase::StaticStruct();
        NestedMatch->RowStruct = FDataOnlyBlueprintEntry::StaticStruct();

        RootRuleSet->DataTables = { DirectMatch, nullptr, DirectWrongRow };
        RootRuleSet->RuleSets = { NestedRuleSet, nullptr };
        NestedRuleSet->DataTables = { NestedMatch };

        TArray<TObjectPtr<UDataTable>> DataTables;
        RootRuleSet->CollectDataTables(FDataOnlyBlueprintEntry::StaticStruct(), DataTables);

        return TestEqual(TEXT("RuleSet collection should include direct and nested matching tables"),
                         DataTables.Num(),
                         2)
            && TestEqual(TEXT("Direct matching table should be collected first"), DataTables[0].Get(), DirectMatch)
            && TestEqual(TEXT("Nested matching table should be collected after nested traversal"),
                         DataTables[1].Get(),
                         NestedMatch);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleSetCollectDataTablesSkipsCyclesTest,
                                 "RuleRanger.RuleSet.CollectDataTables.SkipsCycles",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleSetCollectDataTablesSkipsCyclesTest::RunTest(const FString&)
{
    const auto RootRuleSet =
        RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(GetTransientPackage(), TEXT("CycleRoot"));
    const auto NestedRuleSet =
        RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(RootRuleSet, TEXT("CycleNested"));
    const auto RootTable = RuleRangerTests::NewTransientObject<UDataTable>(RootRuleSet, TEXT("RootTable"));
    const auto NestedTable = RuleRangerTests::NewTransientObject<UDataTable>(NestedRuleSet, TEXT("NestedTable"));
    if (TestNotNull(TEXT("Root rule set should be created"), RootRuleSet)
        && TestNotNull(TEXT("Nested rule set should be created"), NestedRuleSet)
        && TestNotNull(TEXT("Root table should be created"), RootTable)
        && TestNotNull(TEXT("Nested table should be created"), NestedTable))
    {
        RootTable->RowStruct = FDataOnlyBlueprintEntry::StaticStruct();
        NestedTable->RowStruct = FDataOnlyBlueprintEntry::StaticStruct();
        RootRuleSet->DataTables = { RootTable };
        RootRuleSet->RuleSets = { NestedRuleSet };
        NestedRuleSet->DataTables = { NestedTable };
        NestedRuleSet->RuleSets = { RootRuleSet };

        AddExpectedMessagePlain(TEXT("CollectDataTables: Detected cyclic reference involving Rule Set CycleRoot"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);

        TArray<TObjectPtr<UDataTable>> DataTables;
        RootRuleSet->CollectDataTables(FDataOnlyBlueprintEntry::StaticStruct(), DataTables);

        return TestEqual(TEXT("Cycle traversal should collect each reachable table once"), DataTables.Num(), 2)
            && TestEqual(TEXT("Root table should be collected before nested traversal"), DataTables[0].Get(), RootTable)
            && TestEqual(TEXT("Nested table should be collected before the cycle is skipped"),
                         DataTables[1].Get(),
                         NestedTable);
    }
    else
    {
        return false;
    }
}

#endif
