#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/Blueprint/Function/EnsureFunctionsHaveDescriptionsAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureFunctionsHaveDescriptionsActionTests
{
    bool SetCheckPrivateFunctions(FAutomationTestBase& Test,
                                  UEnsureFunctionsHaveDescriptionsAction* const Action,
                                  const bool bCheckPrivateFunctions)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test,
                                                     Action,
                                                     TEXT("bCheckPrivateFunctions"),
                                                     bCheckPrivateFunctions);
    }
} // namespace RuleRangerEnsureFunctionsHaveDescriptionsActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureFunctionsHaveDescriptionsActionErrorsForProtectedFunctionsTest,
                                 "RuleRanger.Actions.Blueprint.Function.EnsureDescriptions.ErrorsForProtectedFunctions",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureFunctionsHaveDescriptionsActionErrorsForProtectedFunctionsTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureFunctionsHaveDescriptionsAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/RuleRangerTests/Blueprint/Function/DescriptionsProtected"),
                                      TEXT("BP_FunctionDescriptionsProtected"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("ProtectedFunction"));
        const auto Entry = Graph ? RuleRangerTests::GetFunctionEntry(Graph) : nullptr;
        if (TestNotNull(TEXT("Function entry should exist"), Entry)
            && TestTrue(TEXT("Function flags should be set"), RuleRangerTests::SetFunctionFlags(Entry, FUNC_Protected)))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestEqual(TEXT("Protected functions without descriptions should add one error"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureFunctionsHaveDescriptionsActionSkipsPrivateFunctionsByDefaultTest,
    "RuleRanger.Actions.Blueprint.Function.EnsureDescriptions.SkipsPrivateFunctionsByDefault",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureFunctionsHaveDescriptionsActionSkipsPrivateFunctionsByDefaultTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureFunctionsHaveDescriptionsAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/RuleRangerTests/Blueprint/Function/DescriptionsPrivate"),
                                      TEXT("BP_FunctionDescriptionsPrivate"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("PrivateFunction"));
        const auto Entry = Graph ? RuleRangerTests::GetFunctionEntry(Graph) : nullptr;
        if (TestNotNull(TEXT("Function entry should exist"), Entry)
            && TestTrue(TEXT("Function flags should be set"), RuleRangerTests::SetFunctionFlags(Entry, FUNC_Private)))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestTrue(TEXT("Private functions should be skipped by default"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureFunctionsHaveDescriptionsActionHonorsPrivateFunctionOptInTest,
                                 "RuleRanger.Actions.Blueprint.Function.EnsureDescriptions.HonorsPrivateFunctionOptIn",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureFunctionsHaveDescriptionsActionHonorsPrivateFunctionOptInTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureFunctionsHaveDescriptionsAction>();
    const auto Blueprint = RuleRangerTests::NewBlueprint(
        URuleRangerAutomationBlueprintParentObject::StaticClass(),
        TEXT("/Game/Developers/RuleRangerTests/Blueprint/Function/DescriptionsPrivateOptIn"),
        TEXT("BP_FunctionDescriptionsPrivateOptIn"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureFunctionsHaveDescriptionsActionTests::SetCheckPrivateFunctions(*this, Action, true))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("PrivateFunction"));
        const auto Entry = Graph ? RuleRangerTests::GetFunctionEntry(Graph) : nullptr;
        if (TestNotNull(TEXT("Function entry should exist"), Entry)
            && TestTrue(TEXT("Function flags should be set"), RuleRangerTests::SetFunctionFlags(Entry, FUNC_Private)))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestEqual(TEXT("Opted-in private functions without descriptions should add one error"),
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

#endif
