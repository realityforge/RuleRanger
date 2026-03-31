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
    #include "RuleRanger/Actions/Texture/EnsureNeverStreamValidAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureNeverStreamValidActionTests
{
    bool SetExpectedNeverStream(FAutomationTestBase& Test,
                                UEnsureNeverStreamValidAction* const Action,
                                const bool bNeverStream)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bNeverStream"), bNeverStream);
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
} // namespace RuleRangerEnsureNeverStreamValidActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNeverStreamValidActionLogsInfoWhenAlreadyValidTest,
                                 "RuleRanger.Actions.Texture.EnsureNeverStreamValid.LogsInfoWhenAlreadyValid",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNeverStreamValidActionLogsInfoWhenAlreadyValidTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNeverStreamValidAction>();
    const auto Texture = RuleRangerEnsureNeverStreamValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/NeverStreamValid"),
        TEXT("NeverStreamValidTexture"));
    if (TestNotNull(TEXT("EnsureNeverStreamValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->NeverStream = true;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureNeverStreamValidActionTests::SetExpectedNeverStream(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestTrue(TEXT("A valid texture should not add info messages to the action context"),
                            Fixture.ActionContext->GetInfoMessages().IsEmpty())
                && TestTrue(TEXT("A valid texture should not add warnings"),
                            Fixture.ActionContext->GetWarningMessages().IsEmpty())
                && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                         Texture,
                                                         false,
                                                         TEXT("A valid texture should not dirty the package"));
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNeverStreamValidActionWarnsInDryRunModeTest,
                                 "RuleRanger.Actions.Texture.EnsureNeverStreamValid.WarnsInDryRunMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNeverStreamValidActionWarnsInDryRunModeTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNeverStreamValidAction>();
    const auto Texture = RuleRangerEnsureNeverStreamValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/NeverStreamDryRun"),
        TEXT("NeverStreamDryRunTexture"));
    if (TestNotNull(TEXT("EnsureNeverStreamValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->NeverStream = false;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureNeverStreamValidActionTests::SetExpectedNeverStream(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Dry-run execution should add one warning"),
                             Fixture.ActionContext->GetWarningMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetWarningMessages(),
                                                          TEXT("Dry-run warnings should mention DryRun mode"),
                                                          TEXT("DryRun mode"))
                && TestTrue(TEXT("Dry-run execution should not mutate NeverStream"), Texture->NeverStream == false)
                && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                         Texture,
                                                         false,
                                                         TEXT("Dry-run execution should not dirty the package"));
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNeverStreamValidActionAppliesFixInSaveModeTest,
                                 "RuleRanger.Actions.Texture.EnsureNeverStreamValid.AppliesFixInSaveMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNeverStreamValidActionAppliesFixInSaveModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNeverStreamValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureNeverStreamValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/NeverStreamFix"),
        TEXT("NeverStreamFixTexture"));
    if (TestNotNull(TEXT("EnsureNeverStreamValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->NeverStream = false;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureNeverStreamValidActionTests::SetExpectedNeverStream(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Fix mode should log one info"), Fixture.ActionContext->GetInfoMessages().Num(), 1)
                && TestTrue(TEXT("Fix mode should update NeverStream"), Texture->NeverStream)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetInfoMessages(),
                       TEXT("The info message should mention the new NeverStream value"),
                       TEXT("changed NeverStream"))
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
