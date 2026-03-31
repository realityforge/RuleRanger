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
    #include "RuleRanger/Actions/Texture/EnsureSRGBValidAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureSRGBValidActionTests
{
    bool SetExpectedSRGB(FAutomationTestBase& Test, UEnsureSRGBValidAction* const Action, const bool bSRGB)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bSRGB"), bSRGB);
    }

    UTexture2D* CreateTextureFixture(FAutomationTestBase& Test,
                                     RuleRangerTests::FRuleFixture& Fixture,
                                     const ERuleRangerActionTrigger Trigger,
                                     const TCHAR* const PackageName,
                                     const TCHAR* const ObjectName)
    {
        const auto Texture = RuleRangerTests::NewPackagedTexture2D(PackageName, ObjectName, 128, 128);
        if (RuleRangerTests::CreateRuleFixture(Test, Fixture, ObjectName, Trigger)
            && Test.TestNotNull(TEXT("Texture should be created"), Texture))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture, Trigger);
            return Texture;
        }
        else
        {
            return nullptr;
        }
    }
} // namespace RuleRangerEnsureSRGBValidActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureSRGBValidActionLogsInfoWhenAlreadyValidTest,
                                 "RuleRanger.Actions.Texture.EnsureSRGBValid.LogsInfoWhenAlreadyValid",
                                 RuleRangerTests::AutomationTestFlags)

bool FRuleRangerEnsureSRGBValidActionLogsInfoWhenAlreadyValidTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureSRGBValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureSRGBValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/SRGBValid"),
        TEXT("SRGBValidTexture"));
    if (TestNotNull(TEXT("EnsureSRGBValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->SRGB = true;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureSRGBValidActionTests::SetExpectedSRGB(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestTrue(TEXT("A valid texture should not add info messages to the action context"),
                            Fixture.ActionContext->GetInfoMessages().IsEmpty())
                && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                         Texture,
                                                         false,
                                                         TEXT("A valid texture should not dirty the package"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureSRGBValidActionWarnsInDryRunModeTest,
                                 "RuleRanger.Actions.Texture.EnsureSRGBValid.WarnsInDryRunMode",
                                 RuleRangerTests::AutomationTestFlags)

bool FRuleRangerEnsureSRGBValidActionWarnsInDryRunModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureSRGBValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureSRGBValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/SRGBDryRun"),
        TEXT("SRGBDryRunTexture"));
    if (TestNotNull(TEXT("EnsureSRGBValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->SRGB = false;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureSRGBValidActionTests::SetExpectedSRGB(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Dry-run execution should add one warning"),
                             Fixture.ActionContext->GetWarningMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetWarningMessages(),
                                                          TEXT("Dry-run warnings should mention DryRun mode"),
                                                          TEXT("DryRun mode"))
                && TestFalse(TEXT("Dry-run execution should not mutate sRGB"), Texture->SRGB)
                && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                         Texture,
                                                         false,
                                                         TEXT("Dry-run execution should not dirty the package"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureSRGBValidActionAppliesFixInSaveModeTest,
                                 "RuleRanger.Actions.Texture.EnsureSRGBValid.AppliesFixInSaveMode",
                                 RuleRangerTests::AutomationTestFlags)

bool FRuleRangerEnsureSRGBValidActionAppliesFixInSaveModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureSRGBValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureSRGBValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/SRGBFix"),
        TEXT("SRGBFixTexture"));
    if (TestNotNull(TEXT("EnsureSRGBValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->SRGB = false;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureSRGBValidActionTests::SetExpectedSRGB(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Fix mode should log one info"), Fixture.ActionContext->GetInfoMessages().Num(), 1)
                && TestTrue(TEXT("Fix mode should update sRGB"), Texture->SRGB)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetInfoMessages(),
                                                          TEXT("The info message should mention the sRGB change"),
                                                          TEXT("changed sRGB"))
                && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                         Texture,
                                                         true,
                                                         TEXT("Fix mode should dirty the texture package"));
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
