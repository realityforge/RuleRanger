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
    #include "RuleRangerConfig.h"
    #include "RuleRangerExclusionSet.h"
    #include "RuleRangerRule.h"
    #include "RuleRangerRuleExclusion.h"
    #include "RuleRangerRuleSet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerConfigTests
{
    FDirectoryPath MakeDir(const TCHAR* const Path)
    {
        FDirectoryPath Dir;
        Dir.Path = Path;
        return Dir;
    }
} // namespace RuleRangerConfigTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerConfigConfigMatchesUsesNonEmptyCaseSensitivePrefixesTest,
                                 "RuleRanger.Config.ConfigMatches.UsesNonEmptyCaseSensitivePrefixes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerConfigConfigMatchesUsesNonEmptyCaseSensitivePrefixesTest::RunTest(const FString&)
{
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    if (TestNotNull(TEXT("Config should be created"), Config))
    {
        Config->Dirs = { RuleRangerConfigTests::MakeDir(TEXT("")),
                         RuleRangerConfigTests::MakeDir(TEXT("/Game/RuleRanger/")) };

        return TestTrue(TEXT("Config should match a path below a configured directory"),
                        Config->ConfigMatches(TEXT("/Game/RuleRanger/Asset")))
            && TestTrue(TEXT("Config should match the configured directory prefix exactly"),
                        Config->ConfigMatches(TEXT("/Game/RuleRanger/")))
            && TestFalse(TEXT("Config matching should be case sensitive"),
                         Config->ConfigMatches(TEXT("/game/RuleRanger/Asset")))
            && TestFalse(TEXT("Empty directories should not match every path"),
                         Config->ConfigMatches(TEXT("/Game/Other/Asset")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerConfigPreSaveCleansSortsAndNormalizesEntriesTest,
                                 "RuleRanger.Config.PreSave.CleansSortsAndNormalizesEntries",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerConfigPreSaveCleansSortsAndNormalizesEntriesTest::RunTest(const FString&)
{
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto ATable = RuleRangerTests::NewTransientObject<UDataTable>(Config, TEXT("ATable"));
    const auto BTable = RuleRangerTests::NewTransientObject<UDataTable>(Config, TEXT("BTable"));
    const auto AExclusionSet = RuleRangerTests::NewTransientObject<URuleRangerExclusionSet>(Config, TEXT("AExSet"));
    const auto BExclusionSet = RuleRangerTests::NewTransientObject<URuleRangerExclusionSet>(Config, TEXT("BExSet"));
    const auto ARuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("ARuleSet"));
    const auto BRuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("BRuleSet"));
    const auto ARule = RuleRangerTests::NewTransientObject<URuleRangerRule>(Config, TEXT("ARule"));
    const auto BRule = RuleRangerTests::NewTransientObject<URuleRangerRule>(Config, TEXT("BRule"));
    const auto AObject = RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("AObject"));
    const auto BObject = RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("BObject"));
    if (TestNotNull(TEXT("Config should be created"), Config) && TestNotNull(TEXT("ATable should be created"), ATable)
        && TestNotNull(TEXT("BTable should be created"), BTable)
        && TestNotNull(TEXT("A exclusion set should be created"), AExclusionSet)
        && TestNotNull(TEXT("B exclusion set should be created"), BExclusionSet)
        && TestNotNull(TEXT("A rule set should be created"), ARuleSet)
        && TestNotNull(TEXT("B rule set should be created"), BRuleSet)
        && TestNotNull(TEXT("A rule should be created"), ARule) && TestNotNull(TEXT("B rule should be created"), BRule)
        && TestNotNull(TEXT("A object should be created"), AObject)
        && TestNotNull(TEXT("B object should be created"), BObject))
    {
        FRuleRangerRuleExclusion Exclusion;
        Exclusion.RuleSets = { BRuleSet, nullptr, ARuleSet };
        Exclusion.Rules = { BRule, nullptr, ARule };
        Exclusion.Objects = { BObject, nullptr, AObject };
        Exclusion.Dirs = { RuleRangerConfigTests::MakeDir(TEXT("/Game/Zed")),
                           RuleRangerConfigTests::MakeDir(TEXT("")),
                           RuleRangerConfigTests::MakeDir(TEXT("/Game/Alpha")) };

        Config->Dirs = { RuleRangerConfigTests::MakeDir(TEXT("/Game/Zed")),
                         RuleRangerConfigTests::MakeDir(TEXT("")),
                         RuleRangerConfigTests::MakeDir(TEXT("/Game/Alpha")) };
        Config->DataTables = { BTable, nullptr, ATable };
        Config->ExclusionSets = { BExclusionSet, nullptr, AExclusionSet };
        Config->Exclusions = { Exclusion };

        const RuleRangerTests::FPreSaveContextHolder SaveContext;
        Config->PreSave(SaveContext.Context);

        const auto& SavedExclusion = Config->Exclusions[0];
        return TestEqual(TEXT("Config PreSave should remove empty dirs"), Config->Dirs.Num(), 2)
            && TestEqual(TEXT("Config PreSave should sort dirs and add trailing slash"),
                         Config->Dirs[0].Path,
                         FString(TEXT("/Game/Alpha/")))
            && TestEqual(TEXT("Config PreSave should normalize later dirs"),
                         Config->Dirs[1].Path,
                         FString(TEXT("/Game/Zed/")))
            && TestEqual(TEXT("Config PreSave should remove null data tables"), Config->DataTables.Num(), 2)
            && TestEqual(TEXT("Config PreSave should sort data tables by name"), Config->DataTables[0].Get(), ATable)
            && TestEqual(TEXT("Config PreSave should remove null exclusion sets"), Config->ExclusionSets.Num(), 2)
            && TestEqual(TEXT("Config PreSave should sort exclusion sets by name"),
                         Config->ExclusionSets[0].Get(),
                         AExclusionSet)
            && TestEqual(TEXT("Config PreSave should remove null exclusion rule sets"),
                         SavedExclusion.RuleSets.Num(),
                         2)
            && TestEqual(TEXT("Config PreSave should sort exclusion rule sets"),
                         SavedExclusion.RuleSets[0].Get(),
                         ARuleSet)
            && TestEqual(TEXT("Config PreSave should remove null exclusion rules"), SavedExclusion.Rules.Num(), 2)
            && TestEqual(TEXT("Config PreSave should sort exclusion rules"), SavedExclusion.Rules[0].Get(), ARule)
            && TestEqual(TEXT("Config PreSave should remove null exclusion objects"), SavedExclusion.Objects.Num(), 2)
            && TestEqual(TEXT("Config PreSave should sort exclusion objects"),
                         SavedExclusion.Objects[0].Get(),
                         static_cast<UObject*>(AObject))
            && TestEqual(TEXT("Config PreSave should remove empty exclusion dirs"), SavedExclusion.Dirs.Num(), 2)
            && TestEqual(TEXT("Config PreSave should sort and normalize exclusion dirs"),
                         SavedExclusion.Dirs[0].Path,
                         FString(TEXT("/Game/Alpha/")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerConfigCollectDataTablesIncludesDirectAndNestedMatchesTest,
                                 "RuleRanger.Config.CollectDataTables.IncludesDirectAndNestedMatches",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerConfigCollectDataTablesIncludesDirectAndNestedMatchesTest::RunTest(const FString&)
{
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto RuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("NestedRuleSet"));
    const auto DirectMatch = RuleRangerTests::NewTransientObject<UDataTable>(Config, TEXT("DirectMatch"));
    const auto DirectWrongRow = RuleRangerTests::NewTransientObject<UDataTable>(Config, TEXT("DirectWrongRow"));
    const auto NestedMatch = RuleRangerTests::NewTransientObject<UDataTable>(RuleSet, TEXT("NestedMatch"));
    if (TestNotNull(TEXT("Config should be created"), Config)
        && TestNotNull(TEXT("Nested rule set should be created"), RuleSet)
        && TestNotNull(TEXT("Direct matching table should be created"), DirectMatch)
        && TestNotNull(TEXT("Direct wrong-row table should be created"), DirectWrongRow)
        && TestNotNull(TEXT("Nested matching table should be created"), NestedMatch))
    {
        DirectMatch->RowStruct = FDataOnlyBlueprintEntry::StaticStruct();
        DirectWrongRow->RowStruct = FTableRowBase::StaticStruct();
        NestedMatch->RowStruct = FDataOnlyBlueprintEntry::StaticStruct();

        Config->DataTables = { DirectMatch, DirectWrongRow, nullptr };
        Config->RuleSets = { RuleSet, nullptr };
        RuleSet->DataTables = { NestedMatch };

        TArray<TObjectPtr<UDataTable>> DataTables;
        Config->CollectDataTables(FDataOnlyBlueprintEntry::StaticStruct(), DataTables);

        return TestEqual(TEXT("Config collection should include direct and nested matching tables"),
                         DataTables.Num(),
                         2)
            && TestEqual(TEXT("Direct matching table should be collected first"), DataTables[0].Get(), DirectMatch)
            && TestEqual(TEXT("Nested matching table should be collected after direct tables"),
                         DataTables[1].Get(),
                         NestedMatch);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerConfigValidationReportsInvalidExclusionsTest,
                                 "RuleRanger.Config.Validation.ReportsInvalidExclusions",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerConfigValidationReportsInvalidExclusionsTest::RunTest(const FString&)
{
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    if (TestNotNull(TEXT("Config should be created"), Config))
    {
        Config->Exclusions.AddDefaulted();

        return RuleRangerTests::TestValidation(*this,
                                               Config,
                                               EDataValidationResult::Invalid,
                                               TEXT("must specify at least one RuleSet or Rule"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerConfigValidationAcceptsCompleteExclusionsTest,
                                 "RuleRanger.Config.Validation.AcceptsCompleteExclusions",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerConfigValidationAcceptsCompleteExclusionsTest::RunTest(const FString&)
{
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto RuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("RuleSet"));
    if (TestNotNull(TEXT("Config should be created"), Config)
        && TestNotNull(TEXT("RuleSet should be created"), RuleSet))
    {
        FRuleRangerRuleExclusion Exclusion;
        Exclusion.RuleSets = { RuleSet };
        Exclusion.Dirs = { RuleRangerConfigTests::MakeDir(TEXT("/Game/RuleRanger")) };
        Config->Exclusions = { Exclusion };

        return RuleRangerTests::TestValidation(*this, Config, EDataValidationResult::Valid);
    }
    else
    {
        return false;
    }
}

#endif
