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
    #include "RuleRanger/Actions/SoundWave/EnsureSoundWaveSampleRateValidAction.h"
    #include "RuleRangerRule.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

class FEnsureSoundWaveSampleRateValidActionTestAccessor
{
public:
    static void ApplySampleRate(const UEnsureSoundWaveSampleRateValidAction* const Action,
                                URuleRangerActionContext* const ActionContext,
                                const float SampleRate)
    {
        Action->ApplySampleRate(ActionContext, SampleRate);
    }
};

namespace RuleRangerEnsureSoundWaveSampleRateValidActionTests
{
    bool SetValidSampleRates(FAutomationTestBase& Test,
                             UEnsureSoundWaveSampleRateValidAction* const Action,
                             const TArray<int32>& ValidSampleRates)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("ValidSampleRates"), ValidSampleRates);
    }

    bool SetActions(FAutomationTestBase& Test,
                    URuleRangerRule* const Rule,
                    const TArray<TObjectPtr<URuleRangerAction>>& Actions)
    {
        return RuleRangerTests::SetPropertyValue(Test, Rule, TEXT("Actions"), Actions);
    }

    bool SetActionOutcome(FAutomationTestBase& Test,
                          URuleRangerAutomationTestAction* const Action,
                          const ERuleRangerAutomationTestActionOutcome Outcome)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("Outcome"), Outcome);
    }
} // namespace RuleRangerEnsureSoundWaveSampleRateValidActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureSoundWaveSampleRateValidActionAcceptsDefaultSampleRateTest,
                                 "RuleRanger.Actions.SoundWave.EnsureSoundWaveSampleRateValid.AcceptsDefaultSampleRate",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureSoundWaveSampleRateValidActionAcceptsDefaultSampleRateTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureSoundWaveSampleRateValidAction>();
    const auto SoundWave =
        RuleRangerTests::NewPackagedSoundWave(TEXT("/Game/Developers/Tests/RuleRanger/SoundWave/DefaultRate"),
                                              TEXT("DefaultRateSoundWave"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureSoundWaveSampleRateValidAction should be created"), Action)
        && TestNotNull(TEXT("SoundWave should be created"), SoundWave))
    {
        RuleRangerTests::SetSoundWaveSampleRate(SoundWave, 48000);
        RuleRangerTests::ResetRuleFixtureObject(Fixture, SoundWave);

        Action->Apply(Fixture.ActionContext, SoundWave);

        return TestTrue(TEXT("Default valid sample rate should not add errors"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureSoundWaveSampleRateValidActionAcceptsNearlyEqualRateTest,
                                 "RuleRanger.Actions.SoundWave.EnsureSoundWaveSampleRateValid.AcceptsNearlyEqualRate",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureSoundWaveSampleRateValidActionAcceptsNearlyEqualRateTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureSoundWaveSampleRateValidAction>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureSoundWaveSampleRateValidAction should be created"), Action))
    {
        if (RuleRangerEnsureSoundWaveSampleRateValidActionTests::SetValidSampleRates(*this, Action, { 48000 }))
        {
            FEnsureSoundWaveSampleRateValidActionTestAccessor::ApplySampleRate(Action, Fixture.ActionContext, 48000.4f);

            return TestTrue(TEXT("Nearly-equal sample rates should not add errors"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty())
                && TestEqual(TEXT("The action context should remain successful"),
                             Fixture.ActionContext->GetState(),
                             ERuleRangerActionState::AS_Success);
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureSoundWaveSampleRateValidActionRejectsInvalidRateTest,
                                 "RuleRanger.Actions.SoundWave.EnsureSoundWaveSampleRateValid.RejectsInvalidRate",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureSoundWaveSampleRateValidActionRejectsInvalidRateTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureSoundWaveSampleRateValidAction>();
    const auto SoundWave =
        RuleRangerTests::NewPackagedSoundWave(TEXT("/Game/Developers/Tests/RuleRanger/SoundWave/InvalidRate"),
                                              TEXT("InvalidRateSoundWave"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureSoundWaveSampleRateValidAction should be created"), Action)
        && TestNotNull(TEXT("SoundWave should be created"), SoundWave))
    {
        RuleRangerTests::SetSoundWaveSampleRate(SoundWave, 44100);
        RuleRangerTests::ResetRuleFixtureObject(Fixture, SoundWave);
        if (RuleRangerEnsureSoundWaveSampleRateValidActionTests::SetValidSampleRates(*this, Action, { 48000, 96000 }))
        {
            Action->Apply(Fixture.ActionContext, SoundWave);

            return TestEqual(TEXT("Invalid sample rates should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should include the actual sample rate"),
                                                          TEXT("44100.000000"))
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should include the valid sample rates"),
                                                          TEXT("48000, 96000"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureSoundWaveSampleRateValidActionRejectsWrongTypeViaRuleTest,
                                 "RuleRanger.Actions.SoundWave.EnsureSoundWaveSampleRateValid.RejectsWrongTypeViaRule",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureSoundWaveSampleRateValidActionRejectsWrongTypeViaRuleTest::RunTest(const FString&)
{
    const auto CompatibleAction = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestAction>();
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureSoundWaveSampleRateValidAction>();
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("Compatible action should be created"), CompatibleAction)
        && TestNotNull(TEXT("EnsureSoundWaveSampleRateValidAction should be created"), Action))
    {
        if (RuleRangerEnsureSoundWaveSampleRateValidActionTests::SetActionOutcome(
                *this,
                CompatibleAction,
                ERuleRangerAutomationTestActionOutcome::Info)
            && RuleRangerEnsureSoundWaveSampleRateValidActionTests::SetActions(*this,
                                                                               Fixture.Rule,
                                                                               { CompatibleAction, Action }))
        {
            AddExpectedMessagePlain(TEXT("Attempt to run on Object that is not an instance of the type SoundWave."),
                                    ELogVerbosity::Error,
                                    EAutomationExpectedMessageFlags::Contains,
                                    1);
            Fixture.Rule->Apply(Fixture.ActionContext, Fixture.Object);

            return TestEqual(TEXT("The compatible action should still run"), CompatibleAction->GetApplyCount(), 1)
                && TestEqual(TEXT("The context should remain successful after the rule skips the wrong-type action"),
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
