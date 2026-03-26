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

#endif
