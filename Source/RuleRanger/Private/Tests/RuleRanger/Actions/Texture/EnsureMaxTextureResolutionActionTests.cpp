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
    #include "RuleRanger/Actions/Texture/EnsureMaxTextureResolutionAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureMaxTextureResolutionActionTests
{
    bool SetMaxSize(FAutomationTestBase& Test,
                    UEnsureMaxTextureResolutionAction* const Action,
                    const int32 MaxSizeX,
                    const int32 MaxSizeY)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("MaxSizeX"), MaxSizeX)
            && RuleRangerTests::SetPropertyValue(Test, Action, TEXT("MaxSizeY"), MaxSizeY);
    }
} // namespace RuleRangerEnsureMaxTextureResolutionActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMaxTextureResolutionActionAcceptsTextureWithinBoundsTest,
                                 "RuleRanger.Actions.Texture.EnsureMaxTextureResolution.AcceptsTextureWithinBounds",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaxTextureResolutionActionAcceptsTextureWithinBoundsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaxTextureResolutionAction>();
    const auto Texture = RuleRangerTests::NewTransientTexture2D(512, 256, TEXT("MaxResolutionValid"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureMaxTextureResolutionAction should be created"), Action)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture);
        if (RuleRangerEnsureMaxTextureResolutionActionTests::SetMaxSize(*this, Action, 1024, 1024))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestTrue(TEXT("Textures within the configured bounds should not add errors"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty())
                && TestEqual(TEXT("The action context should remain successful"),
                             Fixture.ActionContext->GetState(),
                             ERuleRangerActionState::AS_Success);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMaxTextureResolutionActionRejectsOversizedTexturesTest,
                                 "RuleRanger.Actions.Texture.EnsureMaxTextureResolution.RejectsOversizedTextures",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureMaxTextureResolutionActionRejectsOversizedTexturesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureMaxTextureResolutionAction>();
    const auto Texture = RuleRangerTests::NewTransientTexture2D(2048, 4096, TEXT("MaxResolutionInvalid"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureMaxTextureResolutionAction should be created"), Action)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture);
        if (RuleRangerEnsureMaxTextureResolutionActionTests::SetMaxSize(*this, Action, 1024, 2048))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Oversized textures should add exactly one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should include the actual dimensions"),
                                                          TEXT("2048x4096"))
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should include the configured maximum"),
                                                          TEXT("1024x2048"));
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
