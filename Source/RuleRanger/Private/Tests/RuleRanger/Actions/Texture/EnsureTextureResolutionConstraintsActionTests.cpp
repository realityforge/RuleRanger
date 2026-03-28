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
    #include "RuleRanger/Actions/Texture/EnsureTextureResolutionConstraintsAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureTextureResolutionConstraintsActionTests
{
    bool SetConstraint(FAutomationTestBase& Test,
                       UEnsureTextureResolutionConstraintsAction* const Action,
                       const ETextureResolutionConstraint Constraint)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Constraint"), Constraint);
    }
} // namespace RuleRangerEnsureTextureResolutionConstraintsActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureTextureResolutionConstraintsActionLogsInfoForPowerOfTwoTexturesTest,
    "RuleRanger.Actions.Texture.EnsureTextureResolutionConstraints.LogsInfoForPowerOfTwoTextures",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureResolutionConstraintsActionLogsInfoForPowerOfTwoTexturesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureResolutionConstraintsAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerTests::NewTransientTexture2D(512, 1024, TEXT("PowerOfTwoValid"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureTextureResolutionConstraintsAction should be created"), Action)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture);
        if (RuleRangerEnsureTextureResolutionConstraintsActionTests::SetConstraint(
                *this,
                Action,
                ETextureResolutionConstraint::PowerOfTwo))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestTrue(TEXT("Valid power-of-two textures should not add info messages to the action context"),
                            Fixture.ActionContext->GetInfoMessages().IsEmpty())
                && TestTrue(TEXT("Valid power-of-two textures should not add errors"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty());
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
    FRuleRangerEnsureTextureResolutionConstraintsActionRejectsNonPowerOfTwoWidthTest,
    "RuleRanger.Actions.Texture.EnsureTextureResolutionConstraints.RejectsNonPowerOfTwoWidth",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureResolutionConstraintsActionRejectsNonPowerOfTwoWidthTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureResolutionConstraintsAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerTests::NewTransientTexture2D(300, 512, TEXT("PowerOfTwoWidthInvalid"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureTextureResolutionConstraintsAction should be created"), Action)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture);
        if (RuleRangerEnsureTextureResolutionConstraintsActionTests::SetConstraint(
                *this,
                Action,
                ETextureResolutionConstraint::PowerOfTwo))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("A non-power-of-two width should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should mention the width dimension"),
                                                          TEXT("width is not a power of two"));
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
    FRuleRangerEnsureTextureResolutionConstraintsActionRejectsNonPowerOfTwoHeightTest,
    "RuleRanger.Actions.Texture.EnsureTextureResolutionConstraints.RejectsNonPowerOfTwoHeight",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureResolutionConstraintsActionRejectsNonPowerOfTwoHeightTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureResolutionConstraintsAction>();
    const auto Texture = RuleRangerTests::NewTransientTexture2D(512, 300, TEXT("PowerOfTwoHeightInvalid"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureTextureResolutionConstraintsAction should be created"), Action)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture);
        if (RuleRangerEnsureTextureResolutionConstraintsActionTests::SetConstraint(
                *this,
                Action,
                ETextureResolutionConstraint::PowerOfTwo))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("A non-power-of-two height should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should mention the height dimension"),
                                                          TEXT("height is not a power of two"));
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
    FRuleRangerEnsureTextureResolutionConstraintsActionRejectsNonPowerOfTwoWidthAndHeightTest,
    "RuleRanger.Actions.Texture.EnsureTextureResolutionConstraints.RejectsNonPowerOfTwoWidthAndHeight",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureResolutionConstraintsActionRejectsNonPowerOfTwoWidthAndHeightTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureResolutionConstraintsAction>();
    RuleRangerTests::FRuleFixture Fixture;
    const auto Texture = RuleRangerTests::NewTransientTexture2D(300, 600, TEXT("PowerOfTwoBothInvalid"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureTextureResolutionConstraintsAction should be created"), Action)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture);
        if (RuleRangerEnsureTextureResolutionConstraintsActionTests::SetConstraint(
                *this,
                Action,
                ETextureResolutionConstraint::PowerOfTwo))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Two non-power-of-two dimensions should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should mention both dimensions"),
                                                          TEXT("neither width nor height is a power of two"));
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
    FRuleRangerEnsureTextureResolutionConstraintsActionLogsInfoForDivisibleTexturesTest,
    "RuleRanger.Actions.Texture.EnsureTextureResolutionConstraints.LogsInfoForDivisibleTextures",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureResolutionConstraintsActionLogsInfoForDivisibleTexturesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureResolutionConstraintsAction>();
    const auto Texture = RuleRangerTests::NewTransientTexture2D(24, 36, TEXT("DivisibleValid"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureTextureResolutionConstraintsAction should be created"), Action)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture);
        if (RuleRangerEnsureTextureResolutionConstraintsActionTests::SetConstraint(
                *this,
                Action,
                ETextureResolutionConstraint::DivisibleByTwelve))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestTrue(TEXT("Valid divisible dimensions should not add info messages to the action context"),
                            Fixture.ActionContext->GetInfoMessages().IsEmpty());
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
    FRuleRangerEnsureTextureResolutionConstraintsActionRejectsNonDivisibleWidthAndHeightTest,
    "RuleRanger.Actions.Texture.EnsureTextureResolutionConstraints.RejectsNonDivisibleWidthAndHeight",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureResolutionConstraintsActionRejectsNonDivisibleWidthAndHeightTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureResolutionConstraintsAction>();
    const auto Texture = RuleRangerTests::NewTransientTexture2D(10, 14, TEXT("DivisibleInvalid"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureTextureResolutionConstraintsAction should be created"), Action)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture);
        if (RuleRangerEnsureTextureResolutionConstraintsActionTests::SetConstraint(
                *this,
                Action,
                ETextureResolutionConstraint::DivisibleByFour))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestEqual(TEXT("Dimensions that fail the divisor on both axes should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should mention the configured divisor"),
                                                          TEXT("divisible by 4"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureTextureResolutionConstraintsActionSkipsAutoConstraintTest,
                                 "RuleRanger.Actions.Texture.EnsureTextureResolutionConstraints.SkipsAutoConstraint",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureResolutionConstraintsActionSkipsAutoConstraintTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureResolutionConstraintsAction>();
    const auto Texture = RuleRangerTests::NewTransientTexture2D(300, 600, TEXT("AutoConstraintTexture"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureTextureResolutionConstraintsAction should be created"), Action)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture);
        if (RuleRangerEnsureTextureResolutionConstraintsActionTests::SetConstraint(*this,
                                                                                   Action,
                                                                                   ETextureResolutionConstraint::Auto))
        {
            Action->Apply(Fixture.ActionContext, Texture);

            return TestTrue(TEXT("Auto constraint should not emit errors"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty())
                && TestTrue(TEXT("Auto constraint should not emit infos"),
                            Fixture.ActionContext->GetInfoMessages().IsEmpty())
                && TestEqual(TEXT("Auto constraint should leave the context successful"),
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

#endif
