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
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerContextTests
{
    URuleRangerAutomationTestCommonContext* CreateCommonContext()
    {
        return RuleRangerTests::NewTransientObject<URuleRangerAutomationTestCommonContext>();
    }
} // namespace RuleRangerContextTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCommonContextInitialStateTest,
                                 "RuleRanger.Context.Common.InitialState",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCommonContextInitialStateTest::RunTest(const FString&)
{
    const auto Context = RuleRangerContextTests::CreateCommonContext();
    return TestNotNull(TEXT("Common context should be created"), Context)
        && TestNull(TEXT("Config should be unset before reset"), Context->GetConfig())
        && TestNull(TEXT("RuleSet should be unset before reset"), Context->GetRuleSet())
        && TestEqual(TEXT("Initial state should be AS_Max"), Context->GetState(), ERuleRangerActionState::AS_Max)
        && TestTrue(TEXT("Initial info messages should be empty"), Context->GetInfoMessages().IsEmpty())
        && TestTrue(TEXT("Initial warning messages should be empty"), Context->GetWarningMessages().IsEmpty())
        && TestTrue(TEXT("Initial error messages should be empty"), Context->GetErrorMessages().IsEmpty())
        && TestTrue(TEXT("Initial fatal messages should be empty"), Context->GetFatalMessages().IsEmpty());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCommonContextResetClearsMessagesTest,
                                 "RuleRanger.Context.Common.ResetClearsMessages",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCommonContextResetClearsMessagesTest::RunTest(const FString&)
{
    const auto Context = RuleRangerContextTests::CreateCommonContext();
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto RuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>();
    if (TestNotNull(TEXT("Common context should be created"), Context)
        && TestNotNull(TEXT("Config should be created"), Config)
        && TestNotNull(TEXT("RuleSet should be created"), RuleSet))
    {
        Context->Warning(FText::FromString(TEXT("Warning before reset")));
        Context->Error(FText::FromString(TEXT("Error before reset")));

        Context->ResetForTest(Config, RuleSet);

        return TestEqual(TEXT("Reset should set state back to success"),
                         Context->GetState(),
                         ERuleRangerActionState::AS_Success)
            && TestEqual(TEXT("Reset should store config"),
                         Context->GetConfig(),
                         static_cast<const URuleRangerConfig*>(Config))
            && TestEqual(TEXT("Reset should store rule set"),
                         Context->GetRuleSet(),
                         static_cast<const URuleRangerRuleSet*>(RuleSet))
            && TestTrue(TEXT("Reset should clear warnings"), Context->GetWarningMessages().IsEmpty())
            && TestTrue(TEXT("Reset should clear errors"), Context->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCommonContextClearResetsReferencesTest,
                                 "RuleRanger.Context.Common.ClearResetsReferences",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCommonContextClearResetsReferencesTest::RunTest(const FString&)
{
    const auto Context = RuleRangerContextTests::CreateCommonContext();
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto RuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>();
    if (TestNotNull(TEXT("Common context should be created"), Context)
        && TestNotNull(TEXT("Config should be created"), Config)
        && TestNotNull(TEXT("RuleSet should be created"), RuleSet))
    {
        Context->ResetForTest(Config, RuleSet);
        Context->Info(FText::FromString(TEXT("Info before clear")));
        Context->Fatal(FText::FromString(TEXT("Fatal before clear")));
        Context->ClearForTest();

        return TestNull(TEXT("Clear should reset config"), Context->GetConfig())
            && TestNull(TEXT("Clear should reset rule set"), Context->GetRuleSet())
            && TestEqual(TEXT("Clear should set state back to success"),
                         Context->GetState(),
                         ERuleRangerActionState::AS_Success)
            && TestTrue(TEXT("Clear should remove info messages"), Context->GetInfoMessages().IsEmpty())
            && TestTrue(TEXT("Clear should remove fatal messages"), Context->GetFatalMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCommonContextStatePromotionTest,
                                 "RuleRanger.Context.Common.StatePromotion",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCommonContextStatePromotionTest::RunTest(const FString&)
{
    const auto Context = RuleRangerContextTests::CreateCommonContext();
    const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto RuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>();
    if (TestNotNull(TEXT("Common context should be created"), Context)
        && TestNotNull(TEXT("Config should be created"), Config)
        && TestNotNull(TEXT("RuleSet should be created"), RuleSet))
    {
        Context->ResetForTest(Config, RuleSet);
        Context->Info(FText::FromString(TEXT("Info")));
        const auto bInfoPreservesSuccess =
            TestEqual(TEXT("Info should not change state"), Context->GetState(), ERuleRangerActionState::AS_Success);
        Context->Warning(FText::FromString(TEXT("Warning")));
        const auto bWarningPromotes =
            TestEqual(TEXT("Warning should promote state"), Context->GetState(), ERuleRangerActionState::AS_Warning);
        Context->Error(FText::FromString(TEXT("Error")));
        const auto bErrorPromotes =
            TestEqual(TEXT("Error should promote state"), Context->GetState(), ERuleRangerActionState::AS_Error);
        Context->Fatal(FText::FromString(TEXT("Fatal")));
        const auto bFatalPromotes =
            TestEqual(TEXT("Fatal should promote state"), Context->GetState(), ERuleRangerActionState::AS_Fatal);

        return bInfoPreservesSuccess && bWarningPromotes && bErrorPromotes && bFatalPromotes;
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerActionContextResetCapturesReferencesTest,
                                 "RuleRanger.Context.Action.ResetCapturesReferences",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerActionContextResetCapturesReferencesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this,
                                           Fixture,
                                           TEXT("ActionContextResetObject"),
                                           ERuleRangerActionTrigger::AT_Fix))
    {
        return TestEqual(TEXT("Reset should store the rule"),
                         Fixture.ActionContext->GetRule(),
                         static_cast<const URuleRangerRule*>(Fixture.Rule))
            && TestEqual(TEXT("Reset should store the object"),
                         Fixture.ActionContext->GetObject(),
                         static_cast<const UObject*>(Fixture.Object))
            && TestEqual(TEXT("Reset should store the trigger"),
                         Fixture.ActionContext->GetActionTrigger(),
                         ERuleRangerActionTrigger::AT_Fix)
            && TestEqual(TEXT("Reset should set common state to success"),
                         Fixture.ActionContext->GetState(),
                         ERuleRangerActionState::AS_Success);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerActionContextClearResetsStateTest,
                                 "RuleRanger.Context.Action.ClearResetsState",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerActionContextClearResetsStateTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this,
                                           Fixture,
                                           TEXT("ActionContextClearObject"),
                                           ERuleRangerActionTrigger::AT_Fix))
    {
        Fixture.ActionContext->Warning(FText::FromString(TEXT("Warning before clear")));
        FRuleRangerActionContextTestAccessor::ClearContext(Fixture.ActionContext);

        return TestNull(TEXT("Clear should reset the rule"), Fixture.ActionContext->GetRule())
            && TestNull(TEXT("Clear should reset the object"), Fixture.ActionContext->GetObject())
            && TestEqual(TEXT("Clear should default the trigger to report"),
                         Fixture.ActionContext->GetActionTrigger(),
                         ERuleRangerActionTrigger::AT_Report)
            && TestEqual(TEXT("Clear should reset state to success"),
                         Fixture.ActionContext->GetState(),
                         ERuleRangerActionState::AS_Success)
            && TestTrue(TEXT("Clear should remove warnings"), Fixture.ActionContext->GetWarningMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerActionContextDryRunSemanticsTest,
                                 "RuleRanger.Context.Action.DryRunSemantics",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerActionContextDryRunSemanticsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        struct FCase
        {
            ERuleRangerActionTrigger Trigger;
            bool bExpectedDryRun;
            const TCHAR* Label;
        };

        const TArray<FCase> Cases{
            { ERuleRangerActionTrigger::AT_Import, false, TEXT("Import should be fix mode") },
            { ERuleRangerActionTrigger::AT_Reimport, false, TEXT("Reimport should be fix mode") },
            { ERuleRangerActionTrigger::AT_Validate, true, TEXT("Validate should be dry run") },
            { ERuleRangerActionTrigger::AT_Save, false, TEXT("Save should be fix mode") },
            { ERuleRangerActionTrigger::AT_Report, true, TEXT("Report should be dry run") },
            { ERuleRangerActionTrigger::AT_Fix, false, TEXT("Fix should not be dry run") },
        };

        auto bAllPassed = true;
        for (const auto& Case : Cases)
        {
            FRuleRangerActionContextTestAccessor::ResetContext(Fixture.ActionContext,
                                                               Fixture.Config,
                                                               Fixture.RuleSet,
                                                               Fixture.Rule,
                                                               Fixture.Object,
                                                               Case.Trigger);
            bAllPassed = TestEqual(Case.Label, Fixture.ActionContext->IsDryRun(), Case.bExpectedDryRun) && bAllPassed;
        }

        return bAllPassed;
    }
    else
    {
        return false;
    }
}

#endif
