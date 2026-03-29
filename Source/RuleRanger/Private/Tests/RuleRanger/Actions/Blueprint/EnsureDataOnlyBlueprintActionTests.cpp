#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/Blueprint/EnsureDataOnlyBlueprintAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureDataOnlyBlueprintActionTests
{
    UBlueprint* CreateBlueprintFixture(UClass* const ParentClass,
                                       const TCHAR* const PackageName,
                                       const TCHAR* const ObjectName,
                                       const bool bAddFunctionGraph)
    {
        const auto Blueprint = RuleRangerTests::NewBlueprint(ParentClass, PackageName, ObjectName);
        if (Blueprint && bAddFunctionGraph)
        {
            RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("PerformWork"));
            RuleRangerTests::MarkBlueprintModified(Blueprint);
        }
        if (Blueprint)
        {
            RuleRangerTests::CompileBlueprint(Blueprint);
        }
        return Blueprint;
    }

    bool SetObjectTypes(FAutomationTestBase& Test,
                        UEnsureDataOnlyBlueprintAction* const Action,
                        const TArray<TSubclassOf<UObject>>& ObjectTypes)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("ObjectTypes"), ObjectTypes);
    }
} // namespace RuleRangerEnsureDataOnlyBlueprintActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureDataOnlyBlueprintActionAcceptsDataOnlyBlueprintsTest,
                                 "RuleRanger.Actions.Blueprint.EnsureDataOnly.AcceptsDataOnlyBlueprints",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureDataOnlyBlueprintActionAcceptsDataOnlyBlueprintsTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureDataOnlyBlueprintAction>();
    const auto Blueprint = RuleRangerEnsureDataOnlyBlueprintActionTests::CreateBlueprintFixture(
        URuleRangerAutomationBlueprintParentObject::StaticClass(),
        TEXT("/Game/Developers/RuleRangerTests/Blueprint/DataOnly/Accepts"),
        TEXT("BP_DataOnlyAccepts"),
        false);
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Data-only Blueprint should be created"), Blueprint)
        && RuleRangerEnsureDataOnlyBlueprintActionTests::SetObjectTypes(
            *this,
            Action,
            { URuleRangerAutomationBlueprintParentObject::StaticClass() }))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestTrue(TEXT("Data-only Blueprints should pass"), Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureDataOnlyBlueprintActionErrorsForConfiguredTypesTest,
                                 "RuleRanger.Actions.Blueprint.EnsureDataOnly.ErrorsForConfiguredTypes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureDataOnlyBlueprintActionErrorsForConfiguredTypesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureDataOnlyBlueprintAction>();
    const auto Blueprint = RuleRangerEnsureDataOnlyBlueprintActionTests::CreateBlueprintFixture(
        URuleRangerAutomationBlueprintParentObject::StaticClass(),
        TEXT("/Game/Developers/RuleRangerTests/Blueprint/DataOnly/ObjectTypes"),
        TEXT("BP_DataOnlyObjectTypes"),
        true);
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureDataOnlyBlueprintActionTests::SetObjectTypes(
            *this,
            Action,
            { URuleRangerAutomationBlueprintParentObject::StaticClass() }))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestEqual(TEXT("Configured object types should add one error for non-data-only Blueprints"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention the configured type list"),
                                                      TEXT("part of the list of DataOnlyBlueprints"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureDataOnlyBlueprintActionErrorsForConfigDataTablesTest,
                                 "RuleRanger.Actions.Blueprint.EnsureDataOnly.ErrorsForConfigDataTables",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureDataOnlyBlueprintActionErrorsForConfigDataTablesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureDataOnlyBlueprintAction>();
    const auto Blueprint = RuleRangerEnsureDataOnlyBlueprintActionTests::CreateBlueprintFixture(
        URuleRangerAutomationBlueprintParentObject::StaticClass(),
        TEXT("/Game/Developers/RuleRangerTests/Blueprint/DataOnly/ConfigTables"),
        TEXT("BP_DataOnlyConfigTables"),
        true);
    const auto DataTable =
        RuleRangerTests::NewDataTable(TEXT("/Game/Developers/RuleRangerTests/Blueprint/DataOnly/Table"),
                                      TEXT("DT_DataOnlyBlueprints"),
                                      FDataOnlyBlueprintEntry::StaticStruct());
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && TestNotNull(TEXT("DataTable should be created"), DataTable)
        && TestTrue(
            TEXT("DataTable row should be added"),
            RuleRangerTests::AddDataOnlyBlueprintRow(DataTable,
                                                     TEXT("BlueprintParent"),
                                                     URuleRangerAutomationBlueprintParentObject::StaticClass())))
    {
        Fixture.Config->DataTables = { DataTable };
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestEqual(TEXT("Config DataTables should add one error for non-data-only Blueprints"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention DataTable-driven expectations"),
                                                      TEXT("registered in a DataTable"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureDataOnlyBlueprintActionErrorsForMetadataTypesTest,
                                 "RuleRanger.Actions.Blueprint.EnsureDataOnly.ErrorsForMetadataTypes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureDataOnlyBlueprintActionErrorsForMetadataTypesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureDataOnlyBlueprintAction>();
    const auto Blueprint = RuleRangerEnsureDataOnlyBlueprintActionTests::CreateBlueprintFixture(
        URuleRangerAutomationMetaDataOnlyBlueprintParentObject::StaticClass(),
        TEXT("/Game/Developers/RuleRangerTests/Blueprint/DataOnly/Metadata"),
        TEXT("BP_DataOnlyMetadata"),
        true);
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
        Action->Apply(Fixture.ActionContext, Blueprint);
        return TestEqual(TEXT("Metadata-driven matches should add one error for non-data-only Blueprints"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention the RuleRangerDataOnly metadata"),
                                                      TEXT("RuleRangerDataOnly"));
    }
    else
    {
        return false;
    }
}

#endif
