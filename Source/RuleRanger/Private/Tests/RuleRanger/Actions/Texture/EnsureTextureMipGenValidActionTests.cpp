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
    #include "RuleRanger/Actions/Texture/EnsureTextureMipGenValidAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureTextureMipGenValidActionTests
{
    bool SetSettings(FAutomationTestBase& Test,
                     UEnsureTextureMipGenValidAction* const Action,
                     const TArray<TEnumAsByte<TextureMipGenSettings>>& Settings)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Settings"), Settings);
    }

    bool SetApplyFix(FAutomationTestBase& Test, UEnsureTextureMipGenValidAction* const Action, const bool bApplyFix)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test, Action, TEXT("bApplyFix"), bApplyFix);
    }

    FString GetSettingDisplayName(const TextureMipGenSettings Setting)
    {
        return StaticEnum<TextureMipGenSettings>()->GetDisplayNameTextByValue(Setting).ToString();
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
} // namespace RuleRangerEnsureTextureMipGenValidActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureTextureMipGenValidActionLogsInfoWhenAlreadyValidTest,
                                 "RuleRanger.Actions.Texture.EnsureTextureMipGenValid.LogsInfoWhenAlreadyValid",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureMipGenValidActionLogsInfoWhenAlreadyValidTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureMipGenValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureMipGenValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/MipGenValid"),
        TEXT("TextureMipGenValidTexture"));
    if (TestNotNull(TEXT("EnsureTextureMipGenValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->MipGenSettings = TMGS_SimpleAverage;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureTextureMipGenValidActionTests::SetSettings(*this,
                                                                       Action,
                                                                       { TEnumAsByte(TMGS_SimpleAverage) }))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestTrue(TEXT("A valid MipGen setting should not add info messages to the action context"),
                            Fixture.ActionContext->GetInfoMessages().IsEmpty())
                && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                         Texture,
                                                         false,
                                                         TEXT("A valid MipGen setting should not dirty the package"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureTextureMipGenValidActionErrorsWhenFixIsDisabledTest,
                                 "RuleRanger.Actions.Texture.EnsureTextureMipGenValid.ErrorsWhenFixIsDisabled",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureMipGenValidActionErrorsWhenFixIsDisabledTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureMipGenValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureMipGenValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/MipGenError"),
        TEXT("TextureMipGenErrorTexture"));
    if (TestNotNull(TEXT("EnsureTextureMipGenValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->MipGenSettings = TMGS_SimpleAverage;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        const auto ActualSetting =
            RuleRangerEnsureTextureMipGenValidActionTests::GetSettingDisplayName(TMGS_SimpleAverage);
        const auto ValidSetting = RuleRangerEnsureTextureMipGenValidActionTests::GetSettingDisplayName(TMGS_NoMipmaps);
        if (RuleRangerEnsureTextureMipGenValidActionTests::SetSettings(*this, Action, { TEnumAsByte(TMGS_NoMipmaps) })
            && RuleRangerEnsureTextureMipGenValidActionTests::SetApplyFix(*this, Action, false))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Invalid MipGen settings without fix mode should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should include the actual setting"),
                                                          *ActualSetting)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should include the valid setting"),
                                                          *ValidSetting)
                && TestEqual(TEXT("The MipGen setting should remain unchanged"),
                             static_cast<TextureMipGenSettings>(Texture->MipGenSettings),
                             TMGS_SimpleAverage)
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureTextureMipGenValidActionWarnsInDryRunModeTest,
                                 "RuleRanger.Actions.Texture.EnsureTextureMipGenValid.WarnsInDryRunMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureMipGenValidActionWarnsInDryRunModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureMipGenValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureMipGenValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/MipGenDryRun"),
        TEXT("TextureMipGenDryRunTexture"));
    if (TestNotNull(TEXT("EnsureTextureMipGenValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->MipGenSettings = TMGS_SimpleAverage;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        const auto ActualSetting =
            RuleRangerEnsureTextureMipGenValidActionTests::GetSettingDisplayName(TMGS_SimpleAverage);
        const auto FixSetting = RuleRangerEnsureTextureMipGenValidActionTests::GetSettingDisplayName(TMGS_NoMipmaps);
        if (RuleRangerEnsureTextureMipGenValidActionTests::SetSettings(
                *this,
                Action,
                { TEnumAsByte(TMGS_NoMipmaps), TEnumAsByte(TMGS_FromTextureGroup) })
            && RuleRangerEnsureTextureMipGenValidActionTests::SetApplyFix(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Dry-run fix mode should add one warning"),
                             Fixture.ActionContext->GetWarningMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetWarningMessages(),
                                                          TEXT("The warning should mention the current setting"),
                                                          *ActualSetting)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetWarningMessages(),
                       TEXT("The warning should mention the first configured fix target"),
                       *FixSetting)
                && TestEqual(TEXT("Dry-run execution should not change the MipGen setting"),
                             static_cast<TextureMipGenSettings>(Texture->MipGenSettings),
                             TMGS_SimpleAverage)
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
    FRuleRangerEnsureTextureMipGenValidActionAppliesFirstConfiguredFixInSaveModeTest,
    "RuleRanger.Actions.Texture.EnsureTextureMipGenValid.AppliesFirstConfiguredFixInSaveMode",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureMipGenValidActionAppliesFirstConfiguredFixInSaveModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureMipGenValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureMipGenValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/Tests/RuleRanger/Texture/MipGenFix"),
        TEXT("TextureMipGenFixTexture"));
    if (TestNotNull(TEXT("EnsureTextureMipGenValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->MipGenSettings = TMGS_SimpleAverage;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        const auto ActualSetting =
            RuleRangerEnsureTextureMipGenValidActionTests::GetSettingDisplayName(TMGS_SimpleAverage);
        const auto FixSetting = RuleRangerEnsureTextureMipGenValidActionTests::GetSettingDisplayName(TMGS_NoMipmaps);
        if (RuleRangerEnsureTextureMipGenValidActionTests::SetSettings(
                *this,
                Action,
                { TEnumAsByte(TMGS_NoMipmaps), TEnumAsByte(TMGS_FromTextureGroup) })
            && RuleRangerEnsureTextureMipGenValidActionTests::SetApplyFix(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Save-mode fix execution should log one info"),
                             Fixture.ActionContext->GetInfoMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetInfoMessages(),
                                                          TEXT("The info message should mention the original setting"),
                                                          *ActualSetting)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetInfoMessages(),
                       TEXT("The info message should mention the first configured fix target"),
                       *FixSetting)
                && TestEqual(TEXT("Save-mode fix execution should apply the first configured setting"),
                             static_cast<TextureMipGenSettings>(Texture->MipGenSettings),
                             TMGS_NoMipmaps)
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
