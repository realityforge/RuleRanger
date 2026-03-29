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
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerActionBaseExpectedTypeIsObjectTest,
                                 "RuleRanger.Actions.Common.ActionBase.ExpectedTypeIsObject",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerActionBaseExpectedTypeIsObjectTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<URuleRangerAutomationActionFallback>();
    if (TestNotNull(TEXT("Action fallback should be created"), Action))
    {
        return TestEqual(TEXT("The base action should accept UObject by default"),
                         Action->GetExpectedType(),
                         UObject::StaticClass());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerActionBaseReportsFatalWhenOverrideCallsSuperTest,
                                 "RuleRanger.Actions.Common.ActionBase.ReportsFatalWhenOverrideCallsSuper",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerActionBaseReportsFatalWhenOverrideCallsSuperTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URuleRangerAutomationActionFallback>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("Action fallback should be created"), Action))
    {
        AddExpectedMessagePlain(TEXT("Action either failed to override Apply method or calls Super."),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);

        Action->Apply(Fixture.ActionContext, Fixture.Object);

        return TestEqual(TEXT("Calling the base action implementation should promote the context to fatal"),
                         Fixture.ActionContext->GetState(),
                         ERuleRangerActionState::AS_Fatal)
            && RuleRangerTests::TestTextArrayContains(
                   *this,
                   Fixture.ActionContext->GetFatalMessages(),
                   TEXT("Calling the base action implementation should add a fatal diagnostic"),
                   TEXT("Action either failed to override Apply method or calls Super."));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerActionBaseAllowsNullContextWhenOverrideCallsSuperTest,
                                 "RuleRanger.Actions.Common.ActionBase.AllowsNullContextWhenOverrideCallsSuper",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerActionBaseAllowsNullContextWhenOverrideCallsSuperTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<URuleRangerAutomationActionFallback>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestObject>();
    if (TestNotNull(TEXT("Action fallback should be created"), Action)
        && TestNotNull(TEXT("Test object should be created"), Object))
    {
        AddExpectedMessagePlain(TEXT("Action either failed to override Apply method or calls Super."),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);

        Action->Apply(nullptr, Object);
        return true;
    }
    else
    {
        return false;
    }
}

#endif
