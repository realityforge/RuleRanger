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

    #include "Engine/DataTable.h"
    #include "GameplayTagsSettings.h"
    #include "HAL/FileManager.h"
    #include "Misc/AutomationTest.h"
    #include "Misc/FileHelper.h"
    #include "Misc/Paths.h"
    #include "RuleRanger/ProjectActions/GameplayTags/EnforceGameplayTagRemapPresentAction.h"
    #include "RuleRanger/ProjectActions/GameplayTags/EnforceGameplayTagRemapsPresentAction.h"
    #include "RuleRanger/ProjectActions/GameplayTags/EnforceGameplayTagRemapsSortedAction.h"
    #include "RuleRangerCommonContext.h"
    #include "RuleRangerConfig.h"
    #include "RuleRangerProjectActionContext.h"
    #include "UObject/UnrealType.h"

namespace RuleRangerGameplayTagActionTests
{
    constexpr auto AutomationTestFlags =
        EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

    template <typename TObject>
    TObject* NewTransientObject(UObject* const Outer = GetTransientPackage())
    {
        return NewObject<TObject>(Outer, NAME_None, RF_Transient);
    }

    FGameplayTagCategoryRemap MakeRemap(const TCHAR* const BaseCategory, TArray<FString> RemapCategories)
    {
        FGameplayTagCategoryRemap Remap;
        Remap.BaseCategory = BaseCategory;
        Remap.RemapCategories = MoveTemp(RemapCategories);
        return Remap;
    }

    bool TextArrayContainsFragment(const TArray<FText>& Messages, const FString& ExpectedFragment)
    {
        for (const auto& Message : Messages)
        {
            if (Message.ToString().Contains(ExpectedFragment))
            {
                return true;
            }
        }

        return false;
    }

    bool TestTextArrayContains(FAutomationTestBase& Test,
                               const TArray<FText>& Messages,
                               const TCHAR* const Description,
                               const TCHAR* const ExpectedFragment)
    {
        return Test.TestTrue(Description, TextArrayContainsFragment(Messages, ExpectedFragment));
    }

    bool SetEnumPropertyValue(UObject* const Object, const TCHAR* const PropertyName, const int64 Value)
    {
        const auto Property = Object ? FindFProperty<FProperty>(Object->GetClass(), PropertyName) : nullptr;
        if (!Property)
        {
            return false;
        }

        void* const ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);
        if (const auto EnumProperty = CastField<FEnumProperty>(Property))
        {
            EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, Value);
            return true;
        }
        else if (const auto ByteProperty = CastField<FByteProperty>(Property))
        {
            ByteProperty->SetIntPropertyValue(ValuePtr, Value);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool SetObjectPropertyValue(UObject* const Object, const TCHAR* const PropertyName, UObject* const Value)
    {
        const auto Property = Object ? FindFProperty<FObjectPropertyBase>(Object->GetClass(), PropertyName) : nullptr;
        if (!Property)
        {
            return false;
        }

        Property->SetObjectPropertyValue_InContainer(Object, Value);
        return true;
    }

    struct FProjectActionFixture
    {
        TObjectPtr<URuleRangerProjectActionContext> ActionContext;
        TObjectPtr<URuleRangerConfig> Config;

        explicit FProjectActionFixture(
            const ERuleRangerProjectActionTrigger Trigger = ERuleRangerProjectActionTrigger::AT_Report)
            : ActionContext(NewTransientObject<URuleRangerProjectActionContext>())
            , Config(NewTransientObject<URuleRangerConfig>())
        {
            SetEnumPropertyValue(ActionContext,
                                 TEXT("ActionState"),
                                 static_cast<int64>(ERuleRangerActionState::AS_Success));
            SetEnumPropertyValue(ActionContext, TEXT("ActionTrigger"), static_cast<int64>(Trigger));
            SetObjectPropertyValue(ActionContext, TEXT("Config"), Config);
        }
    };

    struct FScopedGameplayTagRemaps
    {
        UGameplayTagsSettings* Settings{ nullptr };
        TArray<FGameplayTagCategoryRemap> OriginalRemaps;
        FString ConfigFilename;
        TArray<uint8> OriginalConfigBytes;
        bool bHadConfigFile{ false };

        explicit FScopedGameplayTagRemaps(TArray<FGameplayTagCategoryRemap> InitialRemaps)
            : Settings(GetMutableDefault<UGameplayTagsSettings>())
        {
            if (Settings)
            {
                OriginalRemaps = Settings->CategoryRemapping;
                ConfigFilename = Settings->GetDefaultConfigFilename();
                if (!ConfigFilename.IsEmpty())
                {
                    bHadConfigFile = FPaths::FileExists(ConfigFilename);
                    if (bHadConfigFile)
                    {
                        FFileHelper::LoadFileToArray(OriginalConfigBytes, *ConfigFilename);
                    }
                }

                Settings->CategoryRemapping = MoveTemp(InitialRemaps);
            }
        }

        ~FScopedGameplayTagRemaps()
        {
            if (Settings)
            {
                Settings->CategoryRemapping = OriginalRemaps;
            }

            if (!ConfigFilename.IsEmpty())
            {
                if (bHadConfigFile)
                {
                    FFileHelper::SaveArrayToFile(OriginalConfigBytes, *ConfigFilename);
                }
                else
                {
                    IFileManager::Get().Delete(*ConfigFilename, false, true);
                }
            }
        }
    };

    UDataTable* NewTagCategoryTable(const TCHAR* const RowName, TArray<FString> DefaultTargets)
    {
        const auto DataTable = NewTransientObject<UDataTable>();
        if (DataTable)
        {
            DataTable->RowStruct = FRuleRangerTagCategoryRow::StaticStruct();

            FRuleRangerTagCategoryRow Row;
            Row.DefaultTargets = MoveTemp(DefaultTargets);
            DataTable->AddRow(FName(RowName), Row);
        }
        return DataTable;
    }
} // namespace RuleRangerGameplayTagActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapPresentMissingNameReportsErrorTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapPresent.MissingNameReportsError",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapPresentMissingNameReportsErrorTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({});
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Action context should be created"), Fixture.ActionContext.Get()))
    {
        Action->Name = TEXT("  ");
        Action->Apply(Fixture.ActionContext);

        return TestEqual(TEXT("Missing name should add one error"), Fixture.ActionContext->GetErrorMessages().Num(), 1)
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should describe the missing name"),
                                     TEXT("requires Name to be set"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapPresentMissingDryRunReportsErrorTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapPresent.MissingDryRunReportsError",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapPresentMissingDryRunReportsErrorTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({});
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Name = TEXT("RuleRanger.Missing");
        Action->DefaultTargets = { TEXT("RuleRanger.Target") };
        Action->Apply(Fixture.ActionContext);

        return TestEqual(TEXT("Missing remap should add one error"), Fixture.ActionContext->GetErrorMessages().Num(), 1)
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should mention the missing base category"),
                                     TEXT("RuleRanger.Missing"))
            && TestTrue(TEXT("Dry-run should not mutate remaps"), ScopedRemaps.Settings->CategoryRemapping.IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapPresentExistingPassesTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapPresent.ExistingPasses",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapPresentExistingPassesTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({
        MakeRemap(TEXT("RuleRanger.Existing"), { TEXT("RuleRanger.Target") }),
    });
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Name = TEXT("RuleRanger.Existing");
        Action->Apply(Fixture.ActionContext);

        return TestTrue(TEXT("Existing populated remap should not add errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestTrue(TEXT("Existing populated remap should not add fatal messages"),
                        Fixture.ActionContext->GetFatalMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapPresentExistingEmptyDryRunReportsErrorTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapPresent.EmptyExistingDryRunReportsError",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapPresentExistingEmptyDryRunReportsErrorTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({
        MakeRemap(TEXT("RuleRanger.Empty"), {}),
    });
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Name = TEXT("RuleRanger.Empty");
        Action->DefaultTargets = { TEXT("RuleRanger.Target") };
        Action->Apply(Fixture.ActionContext);

        return TestEqual(TEXT("Empty existing remap should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should describe empty remap categories"),
                                     TEXT("has no RemapCategories"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapPresentFixAddsMissingRemapTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapPresent.FixAddsMissingRemap",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapPresentFixAddsMissingRemapTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({});
    FProjectActionFixture Fixture(ERuleRangerProjectActionTrigger::AT_Fix);
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Name = TEXT("RuleRanger.Add");
        Action->DefaultTargets = { TEXT("RuleRanger.TargetA"), TEXT("  "), TEXT(""), TEXT("RuleRanger.TargetB") };
        Action->Apply(Fixture.ActionContext);

        const auto Existing = ScopedRemaps.Settings->CategoryRemapping.FindByPredicate(
            [](const FGameplayTagCategoryRemap& Remap) { return Remap.BaseCategory == TEXT("RuleRanger.Add"); });

        return TestTrue(TEXT("Fix should not add errors"), Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("Fix should add one remap"), ScopedRemaps.Settings->CategoryRemapping.Num(), 1)
            && TestNotNull(TEXT("The added remap should exist"), Existing)
            && TestEqual(TEXT("Whitespace default targets should be skipped"),
                         Existing ? Existing->RemapCategories.Num() : 0,
                         2)
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetInfoMessages(),
                                     TEXT("Info should mention the added base category"),
                                     TEXT("RuleRanger.Add"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapPresentFixUpdatesEmptyRemapTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapPresent.FixUpdatesEmptyRemap",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapPresentFixUpdatesEmptyRemapTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({
        MakeRemap(TEXT("RuleRanger.Empty"), {}),
    });
    FProjectActionFixture Fixture(ERuleRangerProjectActionTrigger::AT_Fix);
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Name = TEXT("RuleRanger.Empty");
        Action->DefaultTargets = { TEXT("RuleRanger.Target"), TEXT(" ") };
        Action->Apply(Fixture.ActionContext);

        return TestTrue(TEXT("Fix should not add errors"), Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("Fix should keep the existing remap entry"),
                         ScopedRemaps.Settings->CategoryRemapping.Num(),
                         1)
            && TestEqual(TEXT("Fix should add non-empty default targets"),
                         ScopedRemaps.Settings->CategoryRemapping[0].RemapCategories.Num(),
                         1)
            && TestEqual(TEXT("The target should be copied"),
                         ScopedRemaps.Settings->CategoryRemapping[0].RemapCategories[0],
                         FString(TEXT("RuleRanger.Target")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapPresentCaseVariantExistingPassesTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapPresent.CaseVariantExistingPasses",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapPresentCaseVariantExistingPassesTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({
        MakeRemap(TEXT("ruleranger.case"), { TEXT("RuleRanger.Target") }),
    });
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Name = TEXT("RuleRanger.Case");
        Action->Apply(Fixture.ActionContext);

        return TestTrue(TEXT("Production comparison treats case variants as an existing remap"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("Dry-run should not add a second remap for a case variant"),
                         ScopedRemaps.Settings->CategoryRemapping.Num(),
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapsPresentEmptyExpectationsPassTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapsPresent.EmptyExpectationsPass",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapsPresentEmptyExpectationsPassTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({});
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapsPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Apply(Fixture.ActionContext);

        return TestTrue(TEXT("Empty expectations should not add errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestTrue(TEXT("Empty expectations should not mutate remaps"),
                        ScopedRemaps.Settings->CategoryRemapping.IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapsPresentExistingPassesTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapsPresent.ExistingPasses",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapsPresentExistingPassesTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({
        MakeRemap(TEXT("RuleRanger.First"), { TEXT("RuleRanger.TargetA") }),
        MakeRemap(TEXT("RuleRanger.Second"), { TEXT("RuleRanger.TargetB") }),
    });
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapsPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Remaps = {
            MakeRemap(TEXT("RuleRanger.First"), {}),
            MakeRemap(TEXT("RuleRanger.Second"), { TEXT("RuleRanger.OtherTarget") }),
        };
        Action->Apply(Fixture.ActionContext);

        return TestTrue(TEXT("Existing expected remaps should not add errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("Existing remaps should not be duplicated"),
                         ScopedRemaps.Settings->CategoryRemapping.Num(),
                         2);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapsPresentMultipleMissingDryRunReportsAllTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapsPresent.MultipleMissingDryRunReportsAll",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapsPresentMultipleMissingDryRunReportsAllTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({
        MakeRemap(TEXT("RuleRanger.Existing"), { TEXT("RuleRanger.Target") }),
    });
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapsPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Remaps = {
            MakeRemap(TEXT("RuleRanger.Existing"), {}),
            MakeRemap(TEXT("RuleRanger.MissingA"), {}),
            MakeRemap(TEXT("  "), { TEXT("RuleRanger.Ignored") }),
            MakeRemap(TEXT("RuleRanger.MissingB"), { TEXT("RuleRanger.Default") }),
        };
        Action->Apply(Fixture.ActionContext);

        return TestEqual(TEXT("Both missing remaps should be reported"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         2)
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should mention the missing remap without defaults"),
                                     TEXT("RuleRanger.MissingA"))
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should mention the missing remap with defaults"),
                                     TEXT("RuleRanger.MissingB"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerGameplayTagRemapsPresentFixAddsAndReportsUnfixableMissingTest,
    "RuleRanger.ProjectActions.GameplayTags.RemapsPresent.FixAddsAndReportsUnfixableMissing",
    RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapsPresentFixAddsAndReportsUnfixableMissingTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({});
    FProjectActionFixture Fixture(ERuleRangerProjectActionTrigger::AT_Fix);
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapsPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Remaps = {
            MakeRemap(TEXT("RuleRanger.Added"), { TEXT("RuleRanger.Target"), TEXT(" ") }),
            MakeRemap(TEXT("RuleRanger.NoDefaults"), {}),
        };
        Action->Apply(Fixture.ActionContext);

        const auto Added = ScopedRemaps.Settings->CategoryRemapping.FindByPredicate(
            [](const FGameplayTagCategoryRemap& Remap) { return Remap.BaseCategory == TEXT("RuleRanger.Added"); });

        return TestEqual(TEXT("Missing remap with no defaults should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should mention unfixable remap"),
                                     TEXT("RuleRanger.NoDefaults"))
            && TestNotNull(TEXT("Fix should add the remap that has defaults"), Added)
            && TestEqual(TEXT("Whitespace targets should be skipped when adding"),
                         Added ? Added->RemapCategories.Num() : 0,
                         1)
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetInfoMessages(),
                                     TEXT("Info should mention the fixed remap"),
                                     TEXT("RuleRanger.Added"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerGameplayTagRemapsPresentConfigDataTableDryRunReportsMissingTest,
    "RuleRanger.ProjectActions.GameplayTags.RemapsPresent.ConfigDataTableDryRunReportsMissing",
    RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapsPresentConfigDataTableDryRunReportsMissingTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({});
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapsPresentAction>();
    const auto Table = NewTagCategoryTable(TEXT("RuleRanger.Table"), { TEXT("RuleRanger.TableTarget") });
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Data table should be created"), Table))
    {
        Fixture.Config->DataTables = { Table };
        Action->Apply(Fixture.ActionContext);

        return TestEqual(TEXT("Missing table remap should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should mention the table row name"),
                                     TEXT("RuleRanger.Table"))
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should describe that defaults exist"),
                                     TEXT("defined defaults"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapsPresentDuplicateInlineUsesLastEntryTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapsPresent.DuplicateInlineUsesLastEntry",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapsPresentDuplicateInlineUsesLastEntryTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({});
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapsPresentAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Remaps = {
            MakeRemap(TEXT("RuleRanger.Duplicate"), { TEXT("RuleRanger.Default") }),
            MakeRemap(TEXT("RuleRanger.Duplicate"), {}),
        };
        Action->Apply(Fixture.ActionContext);

        return TestEqual(TEXT("Duplicate expected remap should collapse to one missing report"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Last duplicate entry should define the reported behavior"),
                                     TEXT("is missing an entry for 'RuleRanger.Duplicate'."));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapsSortedSortedInputPassesTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapsSorted.SortedInputPasses",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapsSortedSortedInputPassesTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({
        MakeRemap(TEXT("RuleRanger.Alpha"), { TEXT("RuleRanger.TargetA") }),
        MakeRemap(TEXT("RuleRanger.Zeta"), { TEXT("RuleRanger.TargetZ") }),
    });
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapsSortedAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Apply(Fixture.ActionContext);

        return TestTrue(TEXT("Sorted remaps should not add errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("Sorted input should remain unchanged"),
                         ScopedRemaps.Settings->CategoryRemapping[0].BaseCategory,
                         FString(TEXT("RuleRanger.Alpha")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapsSortedUnsortedDryRunReportsErrorTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapsSorted.UnsortedDryRunReportsError",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapsSortedUnsortedDryRunReportsErrorTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({
        MakeRemap(TEXT("RuleRanger.Zeta"), { TEXT("RuleRanger.TargetZ") }),
        MakeRemap(TEXT("RuleRanger.Alpha"), { TEXT("RuleRanger.TargetA") }),
    });
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapsSortedAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Apply(Fixture.ActionContext);

        return TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should mention base-category ordering"),
                                     TEXT("not sorted by BaseCategory"))
            && TestEqual(TEXT("Dry-run should leave original order intact"),
                         ScopedRemaps.Settings->CategoryRemapping[0].BaseCategory,
                         FString(TEXT("RuleRanger.Zeta")));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapsSortedDryRunReportsCleanupIssuesTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapsSorted.DryRunReportsCleanupIssues",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapsSortedDryRunReportsCleanupIssuesTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({
        MakeRemap(TEXT("RuleRanger.Zeta"),
                  { TEXT("RuleRanger.TargetZ"), TEXT(" "), TEXT("RuleRanger.TargetA"), TEXT("RuleRanger.TargetA") }),
        MakeRemap(TEXT("RuleRanger.Empty"), {}),
        MakeRemap(TEXT("RuleRanger.Alpha"), { TEXT("RuleRanger.TargetA") }),
        MakeRemap(TEXT("RuleRanger.Zeta"), { TEXT("RuleRanger.TargetReplacement") }),
    });
    FProjectActionFixture Fixture;
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapsSortedAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Apply(Fixture.ActionContext);

        return TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should mention whitespace category entries"),
                                     TEXT("whitespace-only"))
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should mention empty remaps"),
                                     TEXT("empty RemapCategories"))
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should mention duplicate remap categories"),
                                     TEXT("duplicate"))
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should mention unsorted categories"),
                                     TEXT("not sorted alphabetically"))
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetErrorMessages(),
                                     TEXT("Error should mention base-category ordering"),
                                     TEXT("not sorted by BaseCategory"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerGameplayTagRemapsSortedFixNormalizesRemapsTest,
                                 "RuleRanger.ProjectActions.GameplayTags.RemapsSorted.FixNormalizesRemaps",
                                 RuleRangerGameplayTagActionTests::AutomationTestFlags)
bool FRuleRangerGameplayTagRemapsSortedFixNormalizesRemapsTest::RunTest(const FString&)
{
    using namespace RuleRangerGameplayTagActionTests;

    FScopedGameplayTagRemaps ScopedRemaps({
        MakeRemap(TEXT("RuleRanger.Zeta"),
                  { TEXT("RuleRanger.TargetZ"), TEXT(" "), TEXT("RuleRanger.TargetA"), TEXT("RuleRanger.TargetA") }),
        MakeRemap(TEXT("RuleRanger.Empty"), {}),
        MakeRemap(TEXT("RuleRanger.Alpha"), { TEXT("RuleRanger.TargetB") }),
        MakeRemap(TEXT("RuleRanger.Zeta"), { TEXT("RuleRanger.TargetReplacement") }),
    });
    FProjectActionFixture Fixture(ERuleRangerProjectActionTrigger::AT_Fix);
    const auto Action = NewTransientObject<UEnforceGameplayTagRemapsSortedAction>();
    if (TestNotNull(TEXT("Action should be created"), Action))
    {
        Action->Apply(Fixture.ActionContext);

        const auto& Remaps = ScopedRemaps.Settings->CategoryRemapping;
        return TestTrue(TEXT("Fix should not add errors"), Fixture.ActionContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("Fix should remove empty and duplicate remaps"), Remaps.Num(), 2)
            && TestEqual(TEXT("Fix should sort remaps by base category"),
                         Remaps[0].BaseCategory,
                         FString(TEXT("RuleRanger.Alpha")))
            && TestEqual(TEXT("Duplicate base category should keep the last occurrence"),
                         Remaps[1].RemapCategories[0],
                         FString(TEXT("RuleRanger.TargetReplacement")))
            && TestTextArrayContains(*this,
                                     Fixture.ActionContext->GetInfoMessages(),
                                     TEXT("Info should mention sorted or duplicate remaps"),
                                     TEXT("Sorted Gameplay Tag CategoryRemapping"));
    }
    else
    {
        return false;
    }
}

#endif
