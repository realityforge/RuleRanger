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
    #include "RuleRanger/Actions/Common/FailAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerFailActionAddsErrorMessageTest,
                                 "RuleRanger.Actions.Common.Fail.AddsErrorMessage",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerFailActionAddsErrorMessageTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UFailAction>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("FailAction should be created"), Action))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Action, TEXT("Message"), FString(TEXT("Configured failure"))))
        {
            Action->Apply(Fixture.ActionContext, Fixture.Object);

            return TestEqual(TEXT("Error mode should produce one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && TestEqual(TEXT("Error mode should set the error state"),
                             Fixture.ActionContext->GetState(),
                             ERuleRangerActionState::AS_Error)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("Error mode should preserve the configured message"),
                                                          TEXT("Configured failure"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerFailActionAddsFatalMessageTest,
                                 "RuleRanger.Actions.Common.Fail.AddsFatalMessage",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerFailActionAddsFatalMessageTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UFailAction>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("FailAction should be created"), Action))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Action, TEXT("Message"), FString(TEXT("Fatal failure")))
            && RuleRangerTests::SetPropertyValue(*this, Action, TEXT("bFatal"), true))
        {
            Action->Apply(Fixture.ActionContext, Fixture.Object);

            return TestEqual(TEXT("Fatal mode should produce one fatal"),
                             Fixture.ActionContext->GetFatalMessages().Num(),
                             1)
                && TestEqual(TEXT("Fatal mode should set the fatal state"),
                             Fixture.ActionContext->GetState(),
                             ERuleRangerActionState::AS_Fatal)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetFatalMessages(),
                                                          TEXT("Fatal mode should preserve the configured message"),
                                                          TEXT("Fatal failure"));
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
