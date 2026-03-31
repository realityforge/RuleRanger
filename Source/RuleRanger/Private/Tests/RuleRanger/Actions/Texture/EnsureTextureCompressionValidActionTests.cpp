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

    #include "Engine/Texture.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Actions/Texture/EnsureTextureCompressionValidAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureTextureCompressionValidActionTests
{
    bool SetSettings(FAutomationTestBase& Test,
                     UEnsureTextureCompressionValidAction* const Action,
                     const TArray<TEnumAsByte<TextureCompressionSettings>>& Settings)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Settings"), Settings);
    }

    bool
    SetApplyFix(FAutomationTestBase& Test, UEnsureTextureCompressionValidAction* const Action, const bool bApplyFix)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bApplyFix"), bApplyFix);
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
} // namespace RuleRangerEnsureTextureCompressionValidActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureTextureCompressionValidActionLogsInfoWhenAlreadyValidTest,
                                 "RuleRanger.Actions.Texture.EnsureTextureCompressionValid.LogsInfoWhenAlreadyValid",
                                 RuleRangerTests::AutomationTestFlags)

bool FRuleRangerEnsureTextureCompressionValidActionLogsInfoWhenAlreadyValidTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureCompressionValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureCompressionValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/CompressionValid"),
        TEXT("TextureCompressionValidTexture"));
    if (TestNotNull(TEXT("EnsureTextureCompressionValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->CompressionSettings = TC_Normalmap;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureTextureCompressionValidActionTests::SetSettings(*this,
                                                                            Action,
                                                                            { TEnumAsByte(TC_Normalmap) }))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestTrue(TEXT("A valid compression setting should not add info messages to the action context"),
                            Fixture.ActionContext->GetInfoMessages().IsEmpty())
                && RuleRangerTests::TestPackageDirtyFlag(
                       *this,
                       Texture,
                       false,
                       TEXT("A valid compression setting should not dirty the package"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureTextureCompressionValidActionErrorsWhenFixIsDisabledTest,
                                 "RuleRanger.Actions.Texture.EnsureTextureCompressionValid.ErrorsWhenFixIsDisabled",
                                 RuleRangerTests::AutomationTestFlags)

bool FRuleRangerEnsureTextureCompressionValidActionErrorsWhenFixIsDisabledTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureCompressionValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureCompressionValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/CompressionError"),
        TEXT("TextureCompressionErrorTexture"));
    if (TestNotNull(TEXT("EnsureTextureCompressionValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->CompressionSettings = TC_Normalmap;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureTextureCompressionValidActionTests::SetSettings(*this, Action, { TEnumAsByte(TC_Default) })
            && RuleRangerEnsureTextureCompressionValidActionTests::SetApplyFix(*this, Action, false))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Invalid compression settings without fix mode should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should include the actual setting"),
                                                          TEXT("Normalmap"))
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should include the valid setting"),
                                                          TEXT("Default"))
                && TestEqual(TEXT("The compression setting should remain unchanged"),
                             static_cast<TextureCompressionSettings>(Texture->CompressionSettings),
                             TC_Normalmap)
                && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                         Texture,
                                                         false,
                                                         TEXT("Errors without fix mode should not dirty the package"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureTextureCompressionValidActionWarnsInDryRunModeTest,
                                 "RuleRanger.Actions.Texture.EnsureTextureCompressionValid.WarnsInDryRunMode",
                                 RuleRangerTests::AutomationTestFlags)

bool FRuleRangerEnsureTextureCompressionValidActionWarnsInDryRunModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureCompressionValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureCompressionValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/CompressionDryRun"),
        TEXT("TextureCompressionDryRunTexture"));
    if (TestNotNull(TEXT("EnsureTextureCompressionValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->CompressionSettings = TC_Normalmap;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureTextureCompressionValidActionTests::SetSettings(
                *this,
                Action,
                { TEnumAsByte(TC_Default), TEnumAsByte(TC_Masks) })
            && RuleRangerEnsureTextureCompressionValidActionTests::SetApplyFix(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Dry-run fix mode should add one warning"),
                             Fixture.ActionContext->GetWarningMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetWarningMessages(),
                                                          TEXT("The warning should mention the current setting"),
                                                          TEXT("Normalmap"))
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetWarningMessages(),
                       TEXT("The warning should mention the first configured fix target"),
                       TEXT("Default"))
                && TestEqual(TEXT("Dry-run execution should not change the compression setting"),
                             static_cast<TextureCompressionSettings>(Texture->CompressionSettings),
                             TC_Normalmap)
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureTextureCompressionValidActionAppliesFirstConfiguredFixInSaveModeTest,
    "RuleRanger.Actions.Texture.EnsureTextureCompressionValid.AppliesFirstConfiguredFixInSaveMode",
    RuleRangerTests::AutomationTestFlags)

bool FRuleRangerEnsureTextureCompressionValidActionAppliesFirstConfiguredFixInSaveModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureCompressionValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureCompressionValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/CompressionFix"),
        TEXT("TextureCompressionFixTexture"));
    if (TestNotNull(TEXT("EnsureTextureCompressionValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->CompressionSettings = TC_Normalmap;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureTextureCompressionValidActionTests::SetSettings(
                *this,
                Action,
                { TEnumAsByte(TC_Default), TEnumAsByte(TC_Masks) })
            && RuleRangerEnsureTextureCompressionValidActionTests::SetApplyFix(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Save-mode fix execution should log one info"),
                             Fixture.ActionContext->GetInfoMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetInfoMessages(),
                                                          TEXT("The info message should mention the original setting"),
                                                          TEXT("Normalmap"))
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetInfoMessages(),
                       TEXT("The info message should mention the first configured fix target"),
                       TEXT("Default"))
                && TestEqual(TEXT("Save-mode fix execution should apply the first configured setting"),
                             static_cast<TextureCompressionSettings>(Texture->CompressionSettings),
                             TC_Default)
                && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                         Texture,
                                                         true,
                                                         TEXT("Save-mode fix execution should dirty the package"));
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
