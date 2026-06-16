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
    #include "RuleRanger/UI/ToolTab/RuleRangerToolProjectResultHandler.h"
    #include "RuleRanger/UI/ToolTab/RuleRangerToolResultHandler.h"
    #include "RuleRanger/UI/ToolTab/SRuleRangerToolPanel.h"
    #include "RuleRangerProjectResultHandler.h"
    #include "RuleRangerResultHandler.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerToolResultHandlerTests
{
    bool TestMessageRow(FAutomationTestBase& Test,
                        const TSharedPtr<FRuleRangerMessageRow>& Row,
                        const UObject* ExpectedAsset,
                        const URuleRangerRuleSet* ExpectedRuleSet,
                        const URuleRangerRule* ExpectedRule,
                        const URuleRangerProjectRule* ExpectedProjectRule,
                        const ERuleRangerToolSeverity ExpectedSeverity,
                        const TCHAR* ExpectedText)
    {
        return Test.TestNotNull(TEXT("Message row should be created"), Row.Get())
            && Test.TestEqual(TEXT("Message row asset should match"), Row->Asset.Get(), ExpectedAsset)
            && Test.TestEqual(TEXT("Message row rule set should match"), Row->RuleSet.Get(), ExpectedRuleSet)
            && Test.TestEqual(TEXT("Message row rule should match"), Row->Rule.Get(), ExpectedRule)
            && Test.TestEqual(TEXT("Message row project rule should match"),
                              Row->ProjectRule.Get(),
                              ExpectedProjectRule)
            && Test.TestEqual(TEXT("Message row severity should match"), Row->Severity, ExpectedSeverity)
            && Test.TestEqual(TEXT("Message row text should match"), Row->Text.ToString(), FString(ExpectedText));
    }
} // namespace RuleRangerToolResultHandlerTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolResultHandlerImplementsResultHandlerInterfaceTest,
                                 "RuleRanger.UI.ToolTab.ResultHandler.ImplementsInterface",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolResultHandlerImplementsResultHandlerInterfaceTest::RunTest(const FString&)
{
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerToolResultHandler>();
    if (!TestNotNull(TEXT("Tool result handler should be created"), Handler))
    {
        return false;
    }

    return TestTrue(TEXT("Tool result handler should implement the result handler interface"),
                    Handler->GetClass()->ImplementsInterface(URuleRangerResultHandler::StaticClass()));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolResultHandlerTranslatesActionMessagesToRowsTest,
                                 "RuleRanger.UI.ToolTab.ResultHandler.TranslatesActionMessagesToRows",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolResultHandlerTranslatesActionMessagesToRowsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerToolResultHandler>();
    const auto Run = MakeShared<FRuleRangerRun>();
    if (!RuleRangerTests::CreateRuleFixture(*this, Fixture)
        || !TestNotNull(TEXT("Tool result handler should be created"), Handler))
    {
        return false;
    }

    Fixture.ActionContext->Info(FText::FromString(TEXT("Tool info")));
    Fixture.ActionContext->Warning(FText::FromString(TEXT("Tool warning")));
    Fixture.ActionContext->Error(FText::FromString(TEXT("Tool error")));
    Fixture.ActionContext->Fatal(FText::FromString(TEXT("Tool fatal")));

    Handler->Init(Run);
    Handler->OnRuleApplied(Fixture.ActionContext);

    return TestEqual(TEXT("The run should receive one row per emitted action message"), Run->Messages.Num(), 4)
        && RuleRangerToolResultHandlerTests::TestMessageRow(*this,
                                                            Run->Messages[0],
                                                            Fixture.Object,
                                                            Fixture.RuleSet,
                                                            Fixture.Rule,
                                                            nullptr,
                                                            ERuleRangerToolSeverity::Info,
                                                            TEXT("Tool info"))
        && RuleRangerToolResultHandlerTests::TestMessageRow(*this,
                                                            Run->Messages[1],
                                                            Fixture.Object,
                                                            Fixture.RuleSet,
                                                            Fixture.Rule,
                                                            nullptr,
                                                            ERuleRangerToolSeverity::Warning,
                                                            TEXT("Tool warning"))
        && RuleRangerToolResultHandlerTests::TestMessageRow(*this,
                                                            Run->Messages[2],
                                                            Fixture.Object,
                                                            Fixture.RuleSet,
                                                            Fixture.Rule,
                                                            nullptr,
                                                            ERuleRangerToolSeverity::Error,
                                                            TEXT("Tool error"))
        && RuleRangerToolResultHandlerTests::TestMessageRow(*this,
                                                            Run->Messages[3],
                                                            Fixture.Object,
                                                            Fixture.RuleSet,
                                                            Fixture.Rule,
                                                            nullptr,
                                                            ERuleRangerToolSeverity::Error,
                                                            TEXT("Tool fatal"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolResultHandlerIgnoresExpiredRunTest,
                                 "RuleRanger.UI.ToolTab.ResultHandler.IgnoresExpiredRun",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolResultHandlerIgnoresExpiredRunTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerToolResultHandler>();
    if (!RuleRangerTests::CreateRuleFixture(*this, Fixture)
        || !TestNotNull(TEXT("Tool result handler should be created"), Handler))
    {
        return false;
    }

    Fixture.ActionContext->Error(FText::FromString(TEXT("Expired run message")));
    Handler->Init(TWeakPtr<FRuleRangerRun>());
    Handler->OnRuleApplied(Fixture.ActionContext);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolProjectResultHandlerImplementsProjectResultHandlerInterfaceTest,
                                 "RuleRanger.UI.ToolTab.ProjectResultHandler.ImplementsInterface",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolProjectResultHandlerImplementsProjectResultHandlerInterfaceTest::RunTest(const FString&)
{
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerToolProjectResultHandler>();
    if (!TestNotNull(TEXT("Tool project result handler should be created"), Handler))
    {
        return false;
    }

    return TestTrue(TEXT("Tool project result handler should implement the project result handler interface"),
                    Handler->GetClass()->ImplementsInterface(URuleRangerProjectResultHandler::StaticClass()));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolProjectResultHandlerTranslatesProjectMessagesToRowsTest,
                                 "RuleRanger.UI.ToolTab.ProjectResultHandler.TranslatesProjectMessagesToRows",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolProjectResultHandlerTranslatesProjectMessagesToRowsTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerToolProjectResultHandler>();
    const auto Run = MakeShared<FRuleRangerRun>();
    if (!RuleRangerTests::CreateProjectRuleFixture(*this, Fixture)
        || !TestNotNull(TEXT("Tool project result handler should be created"), Handler))
    {
        return false;
    }

    Fixture.ActionContext->Info(FText::FromString(TEXT("Project info")));
    Fixture.ActionContext->Warning(FText::FromString(TEXT("Project warning")));
    Fixture.ActionContext->Error(FText::FromString(TEXT("Project error")));
    Fixture.ActionContext->Fatal(FText::FromString(TEXT("Project fatal")));

    Handler->Init(Run);
    Handler->OnProjectRuleApplied(Fixture.ActionContext);

    return TestEqual(TEXT("The run should receive one row per emitted project message"), Run->Messages.Num(), 4)
        && RuleRangerToolResultHandlerTests::TestMessageRow(*this,
                                                            Run->Messages[0],
                                                            nullptr,
                                                            Fixture.RuleSet,
                                                            nullptr,
                                                            Fixture.Rule,
                                                            ERuleRangerToolSeverity::Info,
                                                            TEXT("Project info"))
        && RuleRangerToolResultHandlerTests::TestMessageRow(*this,
                                                            Run->Messages[1],
                                                            nullptr,
                                                            Fixture.RuleSet,
                                                            nullptr,
                                                            Fixture.Rule,
                                                            ERuleRangerToolSeverity::Warning,
                                                            TEXT("Project warning"))
        && RuleRangerToolResultHandlerTests::TestMessageRow(*this,
                                                            Run->Messages[2],
                                                            nullptr,
                                                            Fixture.RuleSet,
                                                            nullptr,
                                                            Fixture.Rule,
                                                            ERuleRangerToolSeverity::Error,
                                                            TEXT("Project error"))
        && RuleRangerToolResultHandlerTests::TestMessageRow(*this,
                                                            Run->Messages[3],
                                                            nullptr,
                                                            Fixture.RuleSet,
                                                            nullptr,
                                                            Fixture.Rule,
                                                            ERuleRangerToolSeverity::Error,
                                                            TEXT("Project fatal"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolProjectResultHandlerIgnoresExpiredRunTest,
                                 "RuleRanger.UI.ToolTab.ProjectResultHandler.IgnoresExpiredRun",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolProjectResultHandlerIgnoresExpiredRunTest::RunTest(const FString&)
{
    RuleRangerTests::FProjectRuleFixture Fixture;
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerToolProjectResultHandler>();
    if (!RuleRangerTests::CreateProjectRuleFixture(*this, Fixture)
        || !TestNotNull(TEXT("Tool project result handler should be created"), Handler))
    {
        return false;
    }

    Fixture.ActionContext->Error(FText::FromString(TEXT("Expired project run message")));
    Handler->Init(TWeakPtr<FRuleRangerRun>());
    Handler->OnProjectRuleApplied(Fixture.ActionContext);

    return true;
}

#endif
