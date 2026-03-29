#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/Blueprint/Variable/EnsureVariablesHaveDescriptionsAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureVariablesHaveDescriptionsActionTests
{
    bool SetCheckPrivateVariables(FAutomationTestBase& Test,
                                  UEnsureVariablesHaveDescriptionsAction* const Action,
                                  const bool bCheckPrivateVariables)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test,
                                                     Action,
                                                     TEXT("bCheckPrivateVariables"),
                                                     bCheckPrivateVariables);
    }

    bool SetCheckTransientVariables(FAutomationTestBase& Test,
                                    UEnsureVariablesHaveDescriptionsAction* const Action,
                                    const bool bCheckTransientVariables)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test,
                                                     Action,
                                                     TEXT("bCheckTransientVariables"),
                                                     bCheckTransientVariables);
    }

    bool SetCheckLocalVariables(FAutomationTestBase& Test,
                                UEnsureVariablesHaveDescriptionsAction* const Action,
                                const bool bCheckLocalVariables)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bCheckLocalVariables"), bCheckLocalVariables);
    }
} // namespace RuleRangerEnsureVariablesHaveDescriptionsActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureVariablesHaveDescriptionsActionErrorsForMissingDescriptionsTest,
    "RuleRanger.Actions.Blueprint.Variable.EnsureDescriptions.ErrorsForMissingDescriptions",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureVariablesHaveDescriptionsActionErrorsForMissingDescriptionsTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureVariablesHaveDescriptionsAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/RuleRangerTests/Blueprint/Variable/DescriptionsMissing"),
                                      TEXT("BP_VariableDescriptionsMissing"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && TestTrue(
            TEXT("Blueprint variable should be added"),
            RuleRangerTests::AddBlueprintVariable(Blueprint, TEXT("DocumentMe"), UEdGraphSchema_K2::PC_Boolean)))
    {
        RuleRangerTests::CompileBlueprint(Blueprint);
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestEqual(TEXT("Missing descriptions should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureVariablesHaveDescriptionsActionSkipsPrivateVariablesByDefaultTest,
    "RuleRanger.Actions.Blueprint.Variable.EnsureDescriptions.SkipsPrivateVariablesByDefault",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureVariablesHaveDescriptionsActionSkipsPrivateVariablesByDefaultTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureVariablesHaveDescriptionsAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/RuleRangerTests/Blueprint/Variable/DescriptionsPrivate"),
                                      TEXT("BP_VariableDescriptionsPrivate"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && TestTrue(
            TEXT("Private Blueprint variable should be added"),
            RuleRangerTests::AddBlueprintVariable(Blueprint, TEXT("PrivateVariable"), UEdGraphSchema_K2::PC_Boolean)))
    {
        RuleRangerTests::SetBlueprintVariableMetaData(Blueprint,
                                                      TEXT("PrivateVariable"),
                                                      FBlueprintMetadata::MD_Private,
                                                      TEXT("true"));
        RuleRangerTests::CompileBlueprint(Blueprint);
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestTrue(TEXT("Private variables should be skipped by default"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureVariablesHaveDescriptionsActionSkipsTransientVariablesByDefaultTest,
    "RuleRanger.Actions.Blueprint.Variable.EnsureDescriptions.SkipsTransientVariablesByDefault",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureVariablesHaveDescriptionsActionSkipsTransientVariablesByDefaultTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureVariablesHaveDescriptionsAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/RuleRangerTests/Blueprint/Variable/DescriptionsTransient"),
                                      TEXT("BP_VariableDescriptionsTransient"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && TestTrue(TEXT("Transient Blueprint variable should be added"),
                    RuleRangerTests::AddBlueprintVariable(Blueprint,
                                                          TEXT("TransientVariable"),
                                                          UEdGraphSchema_K2::PC_Boolean,
                                                          FText::FromString(TEXT("Default")),
                                                          nullptr,
                                                          CPF_Transient | CPF_DisableEditOnInstance)))
    {
        RuleRangerTests::CompileBlueprint(Blueprint);
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestTrue(TEXT("Transient variables should be skipped by default"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureVariablesHaveDescriptionsActionErrorsForLocalVariablesWhenEnabledTest,
    "RuleRanger.Actions.Blueprint.Variable.EnsureDescriptions.ErrorsForLocalVariablesWhenEnabled",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureVariablesHaveDescriptionsActionErrorsForLocalVariablesWhenEnabledTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureVariablesHaveDescriptionsAction>();
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/RuleRangerTests/Blueprint/Variable/DescriptionsLocal"),
                                      TEXT("BP_VariableDescriptionsLocal"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureVariablesHaveDescriptionsActionTests::SetCheckLocalVariables(*this, Action, true))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("LocalVariableFunction"));
        const auto Entry = Graph ? RuleRangerTests::GetFunctionEntry(Graph) : nullptr;
        if (TestNotNull(TEXT("Function entry should exist"), Entry)
            && TestTrue(TEXT("Local variable should be added"),
                        RuleRangerTests::AddLocalVariable(Entry, TEXT("LocalVariable"), UEdGraphSchema_K2::PC_Boolean)))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestEqual(TEXT("Local variables without descriptions should add one error when enabled"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should mention the function name"),
                                                          TEXT("LocalVariableFunction"));
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
