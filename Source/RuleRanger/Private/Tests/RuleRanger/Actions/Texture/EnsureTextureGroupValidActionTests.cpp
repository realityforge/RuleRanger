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
    #include "RuleRanger/Actions/Texture/EnsureTextureGroupValidAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureTextureGroupValidActionTests
{
    bool SetTextureGroups(FAutomationTestBase& Test,
                          UEnsureTextureGroupValidAction* const Action,
                          const TArray<TEnumAsByte<TextureGroup>>& TextureGroups)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("TextureGroups"), TextureGroups);
    }

    bool SetApplyFix(FAutomationTestBase& Test, UEnsureTextureGroupValidAction* const Action, const bool bApplyFix)
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
} // namespace RuleRangerEnsureTextureGroupValidActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureTextureGroupValidActionLogsInfoWhenAlreadyValidTest,
                                 "RuleRanger.Actions.Texture.EnsureTextureGroupValid.LogsInfoWhenAlreadyValid",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureGroupValidActionLogsInfoWhenAlreadyValidTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureGroupValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureGroupValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/RuleRangerTests/Texture/GroupValid"),
        TEXT("TextureGroupValidTexture"));
    if (TestNotNull(TEXT("EnsureTextureGroupValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->LODGroup = TEXTUREGROUP_World;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureTextureGroupValidActionTests::SetTextureGroups(*this,
                                                                           Action,
                                                                           { TEnumAsByte(TEXTUREGROUP_World) }))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestTrue(TEXT("A valid texture group should not add info messages to the action context"),
                            Fixture.ActionContext->GetInfoMessages().IsEmpty())
                && RuleRangerTests::TestPackageDirtyFlag(*this,
                                                         Texture,
                                                         false,
                                                         TEXT("A valid texture group should not dirty the package"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureTextureGroupValidActionErrorsWhenFixIsDisabledTest,
                                 "RuleRanger.Actions.Texture.EnsureTextureGroupValid.ErrorsWhenFixIsDisabled",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureGroupValidActionErrorsWhenFixIsDisabledTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureGroupValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureGroupValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/RuleRangerTests/Texture/GroupError"),
        TEXT("TextureGroupErrorTexture"));
    if (TestNotNull(TEXT("EnsureTextureGroupValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->LODGroup = TEXTUREGROUP_UI;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureTextureGroupValidActionTests::SetTextureGroups(*this,
                                                                           Action,
                                                                           { TEnumAsByte(TEXTUREGROUP_World) })
            && RuleRangerEnsureTextureGroupValidActionTests::SetApplyFix(*this, Action, false))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Invalid texture groups without fix mode should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should include the actual group"),
                                                          TEXT("UI"))
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should include the valid group"),
                                                          TEXT("World"))
                && TestEqual(TEXT("The texture group should remain unchanged"),
                             static_cast<TextureGroup>(Texture->LODGroup),
                             TEXTUREGROUP_UI)
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureTextureGroupValidActionWarnsInDryRunModeTest,
                                 "RuleRanger.Actions.Texture.EnsureTextureGroupValid.WarnsInDryRunMode",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureGroupValidActionWarnsInDryRunModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureGroupValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureGroupValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Report,
        TEXT("/Game/Developers/RuleRangerTests/Texture/GroupDryRun"),
        TEXT("TextureGroupDryRunTexture"));
    if (TestNotNull(TEXT("EnsureTextureGroupValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->LODGroup = TEXTUREGROUP_UI;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureTextureGroupValidActionTests::SetTextureGroups(
                *this,
                Action,
                { TEnumAsByte(TEXTUREGROUP_World), TEnumAsByte(TEXTUREGROUP_Character) })
            && RuleRangerEnsureTextureGroupValidActionTests::SetApplyFix(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Dry-run fix mode should add one warning"),
                             Fixture.ActionContext->GetWarningMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetWarningMessages(),
                                                          TEXT("The warning should mention the original group"),
                                                          TEXT("UI"))
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetWarningMessages(),
                       TEXT("The warning should mention the first configured fix target"),
                       TEXT("World"))
                && TestEqual(TEXT("Dry-run execution should not change the texture group"),
                             static_cast<TextureGroup>(Texture->LODGroup),
                             TEXTUREGROUP_UI)
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
    FRuleRangerEnsureTextureGroupValidActionAppliesFirstConfiguredFixInSaveModeTest,
    "RuleRanger.Actions.Texture.EnsureTextureGroupValid.AppliesFirstConfiguredFixInSaveMode",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureGroupValidActionAppliesFirstConfiguredFixInSaveModeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureGroupValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerEnsureTextureGroupValidActionTests::CreateTextureFixture(
        *this,
        Fixture,
        ERuleRangerActionTrigger::AT_Save,
        TEXT("/Game/Developers/RuleRangerTests/Texture/GroupFix"),
        TEXT("TextureGroupFixTexture"));
    if (TestNotNull(TEXT("EnsureTextureGroupValidAction should be created"), Action)
        && TestNotNull(TEXT("Texture fixture should be created"), Texture))
    {
        Texture->LODGroup = TEXTUREGROUP_UI;
        RuleRangerTests::ClearPackageDirtyFlag(Texture);
        if (RuleRangerEnsureTextureGroupValidActionTests::SetTextureGroups(
                *this,
                Action,
                { TEnumAsByte(TEXTUREGROUP_World), TEnumAsByte(TEXTUREGROUP_Character) })
            && RuleRangerEnsureTextureGroupValidActionTests::SetApplyFix(*this, Action, true))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Save-mode fix execution should log one info"),
                             Fixture.ActionContext->GetInfoMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetInfoMessages(),
                                                          TEXT("The info message should mention the original group"),
                                                          TEXT("UI"))
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetInfoMessages(),
                       TEXT("The info message should mention the first configured fix target"),
                       TEXT("World"))
                && TestEqual(TEXT("Save-mode fix execution should apply the first configured valid group"),
                             static_cast<TextureGroup>(Texture->LODGroup),
                             TEXTUREGROUP_World)
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
