#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Matchers/Blueprint/DataOnlyBlueprintMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerDataOnlyBlueprintMatcherTests
{
    UBlueprint* CreateBlueprintFixture(UClass* const ParentClass,
                                       const TCHAR* const PackageName,
                                       const TCHAR* const ObjectName,
                                       const bool bAddFunctionGraph)
    {
        const auto Blueprint = RuleRangerTests::NewBlueprint(ParentClass, PackageName, ObjectName);
        if (Blueprint && bAddFunctionGraph)
        {
            RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("ComputeValue"));
            RuleRangerTests::MarkBlueprintModified(Blueprint);
        }
        if (Blueprint)
        {
            RuleRangerTests::CompileBlueprint(Blueprint);
        }
        return Blueprint;
    }
} // namespace RuleRangerDataOnlyBlueprintMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerDataOnlyBlueprintMatcherMatchesDataOnlyBlueprintTest,
                                 "RuleRanger.Matchers.Blueprint.DataOnly.MatchesDataOnlyBlueprint",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerDataOnlyBlueprintMatcherMatchesDataOnlyBlueprintTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UDataOnlyBlueprintMatcher>();
    const auto Blueprint = RuleRangerDataOnlyBlueprintMatcherTests::CreateBlueprintFixture(
        URuleRangerAutomationBlueprintParentObject::StaticClass(),
        TEXT("/Game/Developers/RuleRangerTests/Blueprint/Matcher/DataOnly"),
        TEXT("BP_DataOnlyMatcher"),
        false);
    if (TestNotNull(TEXT("Matcher should be created"), Matcher)
        && TestNotNull(TEXT("Data-only Blueprint should be created"), Blueprint))
    {
        return TestTrue(TEXT("Blueprints without executable graphs should match as data-only"),
                        Matcher->Test(Blueprint));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerDataOnlyBlueprintMatcherRejectsNonDataOnlyBlueprintTest,
                                 "RuleRanger.Matchers.Blueprint.DataOnly.RejectsNonDataOnlyBlueprint",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerDataOnlyBlueprintMatcherRejectsNonDataOnlyBlueprintTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UDataOnlyBlueprintMatcher>();
    const auto Blueprint = RuleRangerDataOnlyBlueprintMatcherTests::CreateBlueprintFixture(
        URuleRangerAutomationBlueprintParentObject::StaticClass(),
        TEXT("/Game/Developers/RuleRangerTests/Blueprint/Matcher/NonDataOnly"),
        TEXT("BP_NonDataOnlyMatcher"),
        true);
    if (TestNotNull(TEXT("Matcher should be created"), Matcher)
        && TestNotNull(TEXT("Non-data-only Blueprint should be created"), Blueprint))
    {
        return TestFalse(TEXT("Blueprints with function graphs should not match as data-only"),
                         Matcher->Test(Blueprint));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerDataOnlyBlueprintMatcherRejectsNullAndWrongTypeTest,
                                 "RuleRanger.Matchers.Blueprint.DataOnly.RejectsNullAndWrongType",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerDataOnlyBlueprintMatcherRejectsNullAndWrongTypeTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UDataOnlyBlueprintMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestObject>();
    if (TestNotNull(TEXT("Matcher should be created"), Matcher)
        && TestNotNull(TEXT("Non-blueprint object should be created"), Object))
    {
        return TestFalse(TEXT("Null should not match"), Matcher->Test(nullptr))
            && TestFalse(TEXT("Non-blueprint objects should not match"), Matcher->Test(Object));
    }
    else
    {
        return false;
    }
}

#endif
