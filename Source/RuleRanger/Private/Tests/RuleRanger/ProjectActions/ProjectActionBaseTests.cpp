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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectActionBaseReportsFatalWhenOverrideCallsSuperTest,
                                 "RuleRanger.ProjectActions.Base.ReportsFatalWhenOverrideCallsSuper",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectActionBaseReportsFatalWhenOverrideCallsSuperTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<URuleRangerAutomationProjectActionFallback>();
    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("Project action fallback should be created"), Action))
    {
        AddExpectedMessagePlain(TEXT("Project action either failed to override Apply method or calls Super."),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);

        Action->Apply(Fixture.ActionContext);

        return TestEqual(TEXT("Calling the base project action implementation should promote the context to fatal"),
                         Fixture.ActionContext->GetState(),
                         ERuleRangerActionState::AS_Fatal)
            && RuleRangerTests::TestTextArrayContains(
                   *this,
                   Fixture.ActionContext->GetFatalMessages(),
                   TEXT("Calling the base project action implementation should add a fatal diagnostic"),
                   TEXT("Project action either failed to override Apply method or calls Super."));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerProjectActionBaseAllowsNullContextWhenOverrideCallsSuperTest,
                                 "RuleRanger.ProjectActions.Base.AllowsNullContextWhenOverrideCallsSuper",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerProjectActionBaseAllowsNullContextWhenOverrideCallsSuperTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<URuleRangerAutomationProjectActionFallback>();
    if (TestNotNull(TEXT("Project action fallback should be created"), Action))
    {
        AddExpectedMessagePlain(TEXT("Project action either failed to override Apply method or calls Super."),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);

        Action->Apply(nullptr);
        return true;
    }
    else
    {
        return false;
    }
}

#endif
