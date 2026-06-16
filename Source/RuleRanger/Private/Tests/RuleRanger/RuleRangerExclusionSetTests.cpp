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
    #include "RuleRangerExclusionSet.h"
    #include "RuleRangerRule.h"
    #include "RuleRangerRuleExclusion.h"
    #include "RuleRangerRuleSet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerExclusionSetTests
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
} // namespace RuleRangerExclusionSetTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerExclusionSetPreSaveCleansSortsAndNormalizesExclusionsTest,
                                 "RuleRanger.ExclusionSet.PreSave.CleansSortsAndNormalizesExclusions",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerExclusionSetPreSaveCleansSortsAndNormalizesExclusionsTest::RunTest(const FString&)
{
    const auto ExclusionSet = RuleRangerTests::NewTransientObject<URuleRangerExclusionSet>();
    const auto ARuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(ExclusionSet, TEXT("ARuleSet"));
    const auto BRuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(ExclusionSet, TEXT("BRuleSet"));
    const auto ARule = RuleRangerTests::NewTransientObject<URuleRangerRule>(ExclusionSet, TEXT("ARule"));
    const auto BRule = RuleRangerTests::NewTransientObject<URuleRangerRule>(ExclusionSet, TEXT("BRule"));
    const auto AObject = RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("AObject"));
    const auto BObject = RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("BObject"));
    if (TestNotNull(TEXT("Exclusion set should be created"), ExclusionSet)
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
        Exclusion.Dirs = { RuleRangerExclusionSetTests::MakeDir(TEXT("/Game/Zed")),
                           RuleRangerExclusionSetTests::MakeDir(TEXT("")),
                           RuleRangerExclusionSetTests::MakeDir(TEXT("/Game/Alpha")) };
        ExclusionSet->Exclusions = { Exclusion };

        const RuleRangerTests::FPreSaveContextHolder SaveContext;
        ExclusionSet->PreSave(SaveContext.Context);

        const auto& SavedExclusion = ExclusionSet->Exclusions[0];
        return TestEqual(TEXT("ExclusionSet PreSave should remove null rule sets"), SavedExclusion.RuleSets.Num(), 2)
            && TestEqual(TEXT("ExclusionSet PreSave should sort rule sets by name"),
                         SavedExclusion.RuleSets[0].Get(),
                         ARuleSet)
            && TestEqual(TEXT("ExclusionSet PreSave should remove null rules"), SavedExclusion.Rules.Num(), 2)
            && TestEqual(TEXT("ExclusionSet PreSave should sort rules by name"), SavedExclusion.Rules[0].Get(), ARule)
            && TestEqual(TEXT("ExclusionSet PreSave should remove null objects"), SavedExclusion.Objects.Num(), 2)
            && TestEqual(TEXT("ExclusionSet PreSave should sort objects by name"),
                         SavedExclusion.Objects[0].Get(),
                         static_cast<UObject*>(AObject))
            && TestEqual(TEXT("ExclusionSet PreSave should remove empty dirs"), SavedExclusion.Dirs.Num(), 2)
            && TestEqual(TEXT("ExclusionSet PreSave should sort and normalize dirs"),
                         SavedExclusion.Dirs[0].Path,
                         FString(TEXT("/Game/Alpha/")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerExclusionSetPostLoadUpdatesTitlesForMultipleExclusionsTest,
                                 "RuleRanger.ExclusionSet.PostLoad.UpdatesTitlesForMultipleExclusions",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerExclusionSetPostLoadUpdatesTitlesForMultipleExclusionsTest::RunTest(const FString&)
{
    const auto ExclusionSet = RuleRangerTests::NewTransientObject<URuleRangerExclusionSet>();
    const auto RuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(ExclusionSet, TEXT("TitleRuleSet"));
    const auto Object = RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("TitleObject"));
    if (TestNotNull(TEXT("Exclusion set should be created"), ExclusionSet)
        && TestNotNull(TEXT("Rule set should be created"), RuleSet)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        FRuleRangerRuleExclusion First;
        First.Description = FText::FromString(TEXT("First exclusion"));
        First.RuleSets = { RuleSet };
        First.Objects = { Object };

        FRuleRangerRuleExclusion Second;
        Second.RuleSets = { RuleSet };
        Second.Dirs = { RuleRangerExclusionSetTests::MakeDir(TEXT("/Game/Target/")) };

        ExclusionSet->Exclusions = { First, Second };
        ExclusionSet->PostLoad();

        return TestEqual(TEXT("PostLoad should update the first exclusion title"),
                         RuleRangerExclusionSetTests::GetEditorFriendlyTitle(ExclusionSet->Exclusions[0]),
                         FString(TEXT("First exclusion")))
            && TestEqual(TEXT("PostLoad should update the second exclusion title"),
                         RuleRangerExclusionSetTests::GetEditorFriendlyTitle(ExclusionSet->Exclusions[1]),
                         FString(TEXT("Exclude TitleRuleSet from /Game/Target/")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerExclusionSetValidationReportsDescriptionEmptyAndNestedIssuesTest,
                                 "RuleRanger.ExclusionSet.Validation.ReportsDescriptionEmptyAndNestedIssues",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerExclusionSetValidationReportsDescriptionEmptyAndNestedIssuesTest::RunTest(const FString&)
{
    const auto ExclusionSet = RuleRangerTests::NewTransientObject<URuleRangerExclusionSet>();
    if (TestNotNull(TEXT("Exclusion set should be created"), ExclusionSet))
    {
        return RuleRangerTests::TestValidation(*this,
                                               ExclusionSet,
                                               EDataValidationResult::Invalid,
                                               TEXT("requires a Description"))
            && RuleRangerTests::TestValidation(*this,
                                               ExclusionSet,
                                               EDataValidationResult::Invalid,
                                               TEXT("must contain at least one exclusion"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerExclusionSetValidationAcceptsCompleteExclusionsTest,
                                 "RuleRanger.ExclusionSet.Validation.AcceptsCompleteExclusions",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerExclusionSetValidationAcceptsCompleteExclusionsTest::RunTest(const FString&)
{
    const auto ExclusionSet = RuleRangerTests::NewTransientObject<URuleRangerExclusionSet>();
    const auto Rule = RuleRangerTests::NewTransientObject<URuleRangerRule>(ExclusionSet, TEXT("Rule"));
    const auto Object = RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("Target"));
    if (TestNotNull(TEXT("Exclusion set should be created"), ExclusionSet)
        && TestNotNull(TEXT("Rule should be created"), Rule) && TestNotNull(TEXT("Object should be created"), Object))
    {
        FRuleRangerRuleExclusion Exclusion;
        Exclusion.Rules = { Rule };
        Exclusion.Objects = { Object };

        ExclusionSet->Description = FText::FromString(TEXT("Reusable exclusions"));
        ExclusionSet->Exclusions = { Exclusion };

        return RuleRangerTests::TestValidation(*this, ExclusionSet, EDataValidationResult::Valid);
    }
    else
    {
        return false;
    }
}

#endif
