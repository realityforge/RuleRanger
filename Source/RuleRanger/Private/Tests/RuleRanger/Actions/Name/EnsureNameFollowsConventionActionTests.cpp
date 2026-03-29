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
    #include "RuleRanger/Actions/Name/EnsureNameFollowsConventionAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureNameFollowsConventionActionTests
{
    bool SetNameConventionsTables(FAutomationTestBase& Test,
                                  UEnsureNameFollowsConventionAction* const Action,
                                  const TArray<TObjectPtr<UDataTable>>& Tables)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("NameConventionsTables"), Tables);
    }

    bool SetNotifyIfNameConventionMissing(FAutomationTestBase& Test,
                                          UEnsureNameFollowsConventionAction* const Action,
                                          const bool bNotify)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bNotifyIfNameConventionMissing"), bNotify);
    }

    UDataTable* CreateNameConventionTable(const TCHAR* const PackageName,
                                          const TCHAR* const ObjectName,
                                          const UClass* const ObjectType,
                                          const TCHAR* const Prefix)
    {
        const auto Table = RuleRangerTests::NewDataTable(PackageName, ObjectName, FNameConvention::StaticStruct());
        if (Table)
        {
            FNameConvention Convention;
            Convention.ObjectType = ObjectType;
            Convention.Prefix = Prefix;
            Convention.Suffix = TEXT("");
            RuleRangerTests::AddDataTableRow(Table, TEXT("Convention"), Convention);
        }
        return Table;
    }
} // namespace RuleRangerEnsureNameFollowsConventionActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNameFollowsConventionActionWarnsForPropertyTablesInDryRunTest,
                                 "RuleRanger.Actions.Name.EnsureNameFollowsConvention.WarnsForPropertyTablesInDryRun",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNameFollowsConventionActionWarnsForPropertyTablesInDryRunTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNameFollowsConventionAction>();
    const auto Table = RuleRangerEnsureNameFollowsConventionActionTests::CreateNameConventionTable(
        TEXT("/Game/Developers/RuleRangerTests/Convention/Name/PropertyTable"),
        TEXT("NameConventionPropertyTable"),
        UMaterial::StaticClass(),
        TEXT("M_"));
    const auto FirstMaterial =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/RuleRangerTests/Convention/Name/PropertyFirst"),
                                             TEXT("BodyMaterial"));
    const auto SecondMaterial =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/RuleRangerTests/Convention/Name/PropertySecond"),
                                             TEXT("SurfaceMaterial"));
    const TArray<TObjectPtr<UDataTable>> Tables{ Table };
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureNameFollowsConventionAction should be created"), Action)
        && TestNotNull(TEXT("Convention table should be created"), Table)
        && TestNotNull(TEXT("First material should be created"), FirstMaterial)
        && TestNotNull(TEXT("Second material should be created"), SecondMaterial)
        && RuleRangerEnsureNameFollowsConventionActionTests::SetNameConventionsTables(*this, Action, Tables))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, FirstMaterial, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, FirstMaterial);
        const bool bFirstPasses = TestEqual(TEXT("Dry-run rename warnings should be emitted for the first material"),
                                            Fixture.ActionContext->GetWarningMessages().Num(),
                                            1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetWarningMessages(),
                                                      TEXT("The first warning should mention the prefixed name"),
                                                      TEXT("M_BodyMaterial"));

        RuleRangerTests::ResetRuleFixtureObject(Fixture, SecondMaterial, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, SecondMaterial);
        const bool bSecondPasses = TestEqual(TEXT("Dry-run rename warnings should be emitted for the second material"),
                                             Fixture.ActionContext->GetWarningMessages().Num(),
                                             1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetWarningMessages(),
                                                      TEXT("The second warning should mention the prefixed name"),
                                                      TEXT("M_SurfaceMaterial"));

        return bFirstPasses && bSecondPasses;
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNameFollowsConventionActionUsesConfigTablesTest,
                                 "RuleRanger.Actions.Name.EnsureNameFollowsConvention.UsesConfigTables",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNameFollowsConventionActionUsesConfigTablesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNameFollowsConventionAction>();
    const auto Table = RuleRangerEnsureNameFollowsConventionActionTests::CreateNameConventionTable(
        TEXT("/Game/Developers/RuleRangerTests/Convention/Name/ConfigTable"),
        TEXT("NameConventionConfigTable"),
        UMaterial::StaticClass(),
        TEXT("M_"));
    const auto Material =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/RuleRangerTests/Convention/Name/ConfigAsset"),
                                             TEXT("ConfigDrivenMaterial"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureNameFollowsConventionAction should be created"), Action)
        && TestNotNull(TEXT("Convention table should be created"), Table)
        && TestNotNull(TEXT("Material should be created"), Material))
    {
        Fixture.Config->DataTables = { Table };
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Material, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, Material);
        return TestEqual(TEXT("Config-backed naming conventions should emit one dry-run warning"),
                         Fixture.ActionContext->GetWarningMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetWarningMessages(),
                                                      TEXT("The warning should mention the config-driven rename"),
                                                      TEXT("M_ConfigDrivenMaterial"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureNameFollowsConventionActionWarnsForPackagedObjectsWithoutConventionsTest,
    "RuleRanger.Actions.Name.EnsureNameFollowsConvention.WarnsForPackagedObjectsWithoutConventions",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNameFollowsConventionActionWarnsForPackagedObjectsWithoutConventionsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNameFollowsConventionAction>();
    const auto Material =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/RuleRangerTests/Convention/Name/Missing"),
                                             TEXT("MissingConventionMaterial"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureNameFollowsConventionAction should be created"), Action)
        && TestNotNull(TEXT("Material should be created"), Material)
        && RuleRangerEnsureNameFollowsConventionActionTests::SetNotifyIfNameConventionMissing(*this, Action, true))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Material, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, Material);
        return TestEqual(TEXT("Packaged objects without matching conventions should emit one warning when enabled"),
                         Fixture.ActionContext->GetWarningMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetWarningMessages(),
                                                      TEXT("The warning should mention the missing naming convention"),
                                                      TEXT("Unable to locate naming convention rule"))
            && TestTrue(TEXT("Missing packaged conventions should not emit errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

#endif
