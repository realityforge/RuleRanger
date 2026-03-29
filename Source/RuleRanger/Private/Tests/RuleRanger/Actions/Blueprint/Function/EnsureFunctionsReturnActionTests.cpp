#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/Blueprint/Function/EnsureFunctionsReturnAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureFunctionsReturnActionErrorsWhenReturnNodeMissingTest,
                                 "RuleRanger.Actions.Blueprint.Function.EnsureReturn.ErrorsWhenReturnNodeMissing",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureFunctionsReturnActionErrorsWhenReturnNodeMissingTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureFunctionsReturnAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/RuleRangerTests/Blueprint/Function/ReturnMissing"),
                                      TEXT("BP_FunctionReturnMissing"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("MissingReturnFunction"));
        if (TestNotNull(TEXT("Function graph should be created"), Graph))
        {
            TArray<UK2Node_FunctionResult*> ResultNodes;
            Graph->GetNodesOfClass(ResultNodes);
            for (const auto ResultNode : ResultNodes)
            {
                Graph->RemoveNode(ResultNode);
            }

            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestEqual(TEXT("Functions without result nodes should add one error"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureFunctionsReturnActionSkipsAnimGraphsTest,
                                 "RuleRanger.Actions.Blueprint.Function.EnsureReturn.SkipsAnimGraphs",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureFunctionsReturnActionSkipsAnimGraphsTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureFunctionsReturnAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/RuleRangerTests/Blueprint/Function/ReturnAnimGraph"),
                                      TEXT("BP_FunctionReturnAnimGraph"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, UEdGraphSchema_K2::GN_AnimGraph);
        if (TestNotNull(TEXT("AnimGraph should be created"), Graph))
        {
            TArray<UK2Node_FunctionResult*> ResultNodes;
            Graph->GetNodesOfClass(ResultNodes);
            for (const auto ResultNode : ResultNodes)
            {
                Graph->RemoveNode(ResultNode);
            }

            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestTrue(TEXT("AnimGraphs should be skipped"), Fixture.ActionContext->GetErrorMessages().IsEmpty());
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
