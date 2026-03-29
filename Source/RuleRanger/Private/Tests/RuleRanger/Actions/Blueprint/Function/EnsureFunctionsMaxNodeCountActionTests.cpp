#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/Blueprint/Function/EnsureFunctionsMaxNodeCountAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureFunctionsMaxNodeCountActionTests
{
    bool SetMaxNonTrivialNodeCount(FAutomationTestBase& Test,
                                   UEnsureFunctionsMaxNodeCountAction* const Action,
                                   const int32 Count)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("MaxNonTrivialNodeCount"), Count);
    }
} // namespace RuleRangerEnsureFunctionsMaxNodeCountActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureFunctionsMaxNodeCountActionErrorsWhenLimitExceededTest,
                                 "RuleRanger.Actions.Blueprint.Function.EnsureMaxNodeCount.ErrorsWhenLimitExceeded",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureFunctionsMaxNodeCountActionErrorsWhenLimitExceededTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureFunctionsMaxNodeCountAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/RuleRangerTests/Blueprint/Function/NodeCountError"),
                                      TEXT("BP_FunctionNodeCountError"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureFunctionsMaxNodeCountActionTests::SetMaxNonTrivialNodeCount(*this, Action, 1))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("BusyFunction"));
        if (TestNotNull(TEXT("Function graph should be created"), Graph)
            && TestNotNull(TEXT("First non-trivial node should be created"),
                           RuleRangerTests::AddGraphNode<URuleRangerAutomationExecGraphNode>(Graph))
            && TestNotNull(TEXT("Second non-trivial node should be created"),
                           RuleRangerTests::AddGraphNode<URuleRangerAutomationExecGraphNode>(Graph)))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestEqual(TEXT("Functions over the non-trivial node limit should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureFunctionsMaxNodeCountActionAllowsGraphsWithinLimitTest,
                                 "RuleRanger.Actions.Blueprint.Function.EnsureMaxNodeCount.AllowsGraphsWithinLimit",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureFunctionsMaxNodeCountActionAllowsGraphsWithinLimitTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureFunctionsMaxNodeCountAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/RuleRangerTests/Blueprint/Function/NodeCountOkay"),
                                      TEXT("BP_FunctionNodeCountOkay"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureFunctionsMaxNodeCountActionTests::SetMaxNonTrivialNodeCount(*this, Action, 1))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("SmallFunction"));
        if (TestNotNull(TEXT("Function graph should be created"), Graph)
            && TestNotNull(TEXT("Non-trivial node should be created"),
                           RuleRangerTests::AddGraphNode<URuleRangerAutomationExecGraphNode>(Graph)))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestTrue(TEXT("Functions within the non-trivial node limit should not add errors"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty());
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

#endif
