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
    #include "RuleRanger/Actions/Common/RunDataValidationAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRunDataValidationSkipsValidateTriggerTest,
                                 "RuleRanger.Actions.Common.RunDataValidation.SkipsValidateTrigger",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRunDataValidationSkipsValidateTriggerTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URunDataValidationAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestValidationObject>();
    if (RuleRangerTests::CreateRuleFixture(*this,
                                           Fixture,
                                           TEXT("ValidationObject"),
                                           ERuleRangerActionTrigger::AT_Validate)
        && TestNotNull(TEXT("RunDataValidationAction should be created"), Action)
        && TestNotNull(TEXT("Validation object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("ValidationResult"), EDataValidationResult::Invalid)
            && RuleRangerTests::SetPropertyValue(*this, Object, TEXT("bEmitError"), true))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestTrue(TEXT("Validate-trigger execution should not emit messages"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty()
                                && Fixture.ActionContext->GetWarningMessages().IsEmpty()
                                && Fixture.ActionContext->GetInfoMessages().IsEmpty());
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRunDataValidationSkipsSaveTriggerTest,
                                 "RuleRanger.Actions.Common.RunDataValidation.SkipsSaveTrigger",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRunDataValidationSkipsSaveTriggerTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URunDataValidationAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestValidationObject>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture, TEXT("ValidationObject"), ERuleRangerActionTrigger::AT_Save)
        && TestNotNull(TEXT("RunDataValidationAction should be created"), Action)
        && TestNotNull(TEXT("Validation object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("ValidationResult"), EDataValidationResult::Invalid)
            && RuleRangerTests::SetPropertyValue(*this, Object, TEXT("bEmitError"), true))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestTrue(TEXT("Save-trigger execution should not emit messages"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty()
                                && Fixture.ActionContext->GetWarningMessages().IsEmpty()
                                && Fixture.ActionContext->GetInfoMessages().IsEmpty());
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRunDataValidationMapsIssueSeveritiesTest,
                                 "RuleRanger.Actions.Common.RunDataValidation.MapsIssueSeverities",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRunDataValidationMapsIssueSeveritiesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URunDataValidationAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestValidationObject>();
    if (RuleRangerTests::CreateRuleFixture(*this,
                                           Fixture,
                                           TEXT("ValidationObject"),
                                           ERuleRangerActionTrigger::AT_Report)
        && TestNotNull(TEXT("RunDataValidationAction should be created"), Action)
        && TestNotNull(TEXT("Validation object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("ValidationResult"), EDataValidationResult::Invalid)
            && RuleRangerTests::SetPropertyValue(*this, Object, TEXT("bEmitInfo"), true)
            && RuleRangerTests::SetPropertyValue(*this, Object, TEXT("bEmitWarning"), true)
            && RuleRangerTests::SetPropertyValue(*this, Object, TEXT("bEmitPerformanceWarning"), true)
            && RuleRangerTests::SetPropertyValue(*this, Object, TEXT("bEmitError"), true))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Info messages should be forwarded"),
                             Fixture.ActionContext->GetInfoMessages().Num(),
                             1)
                && TestEqual(TEXT("Warning and performance warning should both map to warnings"),
                             Fixture.ActionContext->GetWarningMessages().Num(),
                             2)
                && TestEqual(TEXT("Errors should map to errors"), Fixture.ActionContext->GetErrorMessages().Num(), 1)
                && TestEqual(TEXT("Any error should leave the context in the error state"),
                             Fixture.ActionContext->GetState(),
                             ERuleRangerActionState::AS_Error)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetWarningMessages(),
                                                          TEXT("Warning text should be preserved"),
                                                          TEXT("Validation warning"))
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("Error text should be preserved"),
                                                          TEXT("Validation error"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRunDataValidationAddsGenericFailureWhenNeededTest,
                                 "RuleRanger.Actions.Common.RunDataValidation.AddsGenericFailureWhenNeeded",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRunDataValidationAddsGenericFailureWhenNeededTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URunDataValidationAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestValidationObject>();
    if (RuleRangerTests::CreateRuleFixture(*this,
                                           Fixture,
                                           TEXT("ValidationObject"),
                                           ERuleRangerActionTrigger::AT_Report)
        && TestNotNull(TEXT("RunDataValidationAction should be created"), Action)
        && TestNotNull(TEXT("Validation object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("ValidationResult"), EDataValidationResult::Invalid))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Generic invalid result should produce one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Generic invalid result should add the fallback message"),
                       TEXT("Data Validation reported failure for this asset"));
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
