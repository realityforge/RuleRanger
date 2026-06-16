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
    #include "RuleRangerConfig.h"
    #include "RuleRangerRule.h"
    #include "RuleRangerRuleExclusion.h"
    #include "RuleRangerRuleSet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerRuleExclusionTests
{
    FDirectoryPath MakeDir(const TCHAR* const Path)
    {
        FDirectoryPath Dir;
        Dir.Path = Path;
        return Dir;
    }

    FString GetEditorFriendlyTitle(const FRuleRangerRuleExclusion& Exclusion)
    {
        const auto Property =
            FindFProperty<FStrProperty>(FRuleRangerRuleExclusion::StaticStruct(), TEXT("EditorFriendlyTitle"));
        return Property ? Property->GetPropertyValue_InContainer(&Exclusion) : FString();
    }
} // namespace RuleRangerRuleExclusionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleExclusionExclusionMatchesObjectsAndPathPrefixesTest,
                                 "RuleRanger.RuleExclusion.ExclusionMatches.ObjectsAndPathPrefixes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleExclusionExclusionMatchesObjectsAndPathPrefixesTest::RunTest(const FString&)
{
    const auto MatchedObject =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("MatchedObject"));
    const auto OtherObject =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("OtherObject"));
    if (TestNotNull(TEXT("Matched object should be created"), MatchedObject)
        && TestNotNull(TEXT("Other object should be created"), OtherObject))
    {
        FRuleRangerRuleExclusion Exclusion;
        Exclusion.Objects = { nullptr, MatchedObject };
        Exclusion.Dirs = { RuleRangerRuleExclusionTests::MakeDir(TEXT("")),
                           RuleRangerRuleExclusionTests::MakeDir(TEXT("/Game/RuleRanger/")) };

        return TestTrue(TEXT("Exclusion should match an explicitly listed object"),
                        Exclusion.ExclusionMatches(*MatchedObject, TEXT("/Game/Other/Asset")))
            && TestTrue(TEXT("Exclusion should match a configured path prefix"),
                        Exclusion.ExclusionMatches(*OtherObject, TEXT("/Game/RuleRanger/Asset")))
            && TestFalse(TEXT("Exclusion path matching should be case sensitive"),
                         Exclusion.ExclusionMatches(*OtherObject, TEXT("/game/RuleRanger/Asset")))
            && TestFalse(TEXT("Exclusion should ignore empty paths and reject unrelated targets"),
                         Exclusion.ExclusionMatches(*OtherObject, TEXT("/Game/Other/Asset")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRuleExclusionPostLoadTitlesUseDescriptionSinglePairAndFallbackTest,
                                 "RuleRanger.RuleExclusion.Titles.UseDescriptionSinglePairAndFallback",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRuleExclusionPostLoadTitlesUseDescriptionSinglePairAndFallbackTest::RunTest(const FString&)
{
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto RuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("TitleRuleSet"));
    const auto Rule = RuleRangerTests::NewTransientObject<URuleRangerRule>(Config, TEXT("TitleRule"));
    const auto Object = RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("TitleObject"));
    if (TestNotNull(TEXT("Config should be created"), Config)
        && TestNotNull(TEXT("Rule set should be created"), RuleSet) && TestNotNull(TEXT("Rule should be created"), Rule)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        FRuleRangerRuleExclusion Described;
        Described.Description = FText::FromString(TEXT("  Intentional title  "));
        Described.RuleSets = { RuleSet };
        Described.Objects = { Object };

        FRuleRangerRuleExclusion SinglePair;
        SinglePair.Rules = { Rule };
        SinglePair.Dirs = { RuleRangerRuleExclusionTests::MakeDir(TEXT("/Game/RuleRanger/")) };

        FRuleRangerRuleExclusion Fallback;
        Fallback.RuleSets = { RuleSet };
        Fallback.Rules = { Rule };
        Fallback.Objects = { Object };
        Fallback.Dirs = { RuleRangerRuleExclusionTests::MakeDir(TEXT("/Game/RuleRanger/")) };

        FRuleRangerRuleExclusion Empty;

        Config->Exclusions = { Described, SinglePair, Fallback, Empty };
        Config->PostLoad();

        return TestEqual(TEXT("Description should win and be trimmed"),
                         RuleRangerRuleExclusionTests::GetEditorFriendlyTitle(Config->Exclusions[0]),
                         FString(TEXT("Intentional title")))
            && TestEqual(TEXT("A single rule/target pair should receive a derived title"),
                         RuleRangerRuleExclusionTests::GetEditorFriendlyTitle(Config->Exclusions[1]),
                         FString(TEXT("Exclude TitleRule from /Game/RuleRanger/")))
            && TestEqual(
                   TEXT("Fallback title should describe the populated counts"),
                   RuleRangerRuleExclusionTests::GetEditorFriendlyTitle(Config->Exclusions[2]),
                   FString(TEXT(
                       "Undocumented Exclude (Excluded RuleSets=1, Excluded Rules=1, Matched Objects=1, Matched Dirs=1)")))
            && TestEqual(TEXT("Empty exclusions should receive an invalid fallback title"),
                         RuleRangerRuleExclusionTests::GetEditorFriendlyTitle(Config->Exclusions[3]),
                         FString(TEXT("Undocumented Exclude (Invalid Exclusion)")));
    }
    else
    {
        return false;
    }
}

#endif
