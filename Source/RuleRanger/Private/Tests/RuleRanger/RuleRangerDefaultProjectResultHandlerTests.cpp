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

    #include "IMessageLogListing.h"
    #include "MessageLogModule.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRangerDefaultProjectResultHandler.h"
    #include "RuleRangerMessageLog.h"
    #include "RuleRangerProjectResultHandler.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerDefaultProjectResultHandlerImplementsProjectResultHandlerInterfaceTest,
                                 "RuleRanger.ProjectResultHandler.Interface.DefaultHandlerImplementsInterface",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerDefaultProjectResultHandlerImplementsProjectResultHandlerInterfaceTest::RunTest(const FString&)
{
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerDefaultProjectResultHandler>();
    if (TestNotNull(TEXT("Default project result handler should be created"), Handler))
    {
        return TestTrue(TEXT("Default project result handler should implement the project result handler interface"),
                        Handler->GetClass()->ImplementsInterface(URuleRangerProjectResultHandler::StaticClass()));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerDefaultProjectResultHandlerWritesContextMessagesToMessageLogTest,
                                 "RuleRanger.ProjectResultHandler.Default.WritesContextMessagesToMessageLog",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerDefaultProjectResultHandlerWritesContextMessagesToMessageLogTest::RunTest(const FString&)
{
    FRuleRangerMessageLog::Initialize();

    auto& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
    const auto Listing = MessageLogModule.GetLogListing(FRuleRangerMessageLog::GetMessageLogName());
    Listing->ClearMessages();

    RuleRangerTests::FProjectRuleFixture Fixture;
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerDefaultProjectResultHandler>();
    if (RuleRangerTests::CreateProjectRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("Default project result handler should be created"), Handler))
    {
        AddExpectedMessagePlain(TEXT("Project warning message"),
                                ELogVerbosity::Warning,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        AddExpectedMessagePlain(TEXT("Project error message"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        AddExpectedMessagePlain(TEXT("Project fatal message"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);

        Fixture.ActionContext->Info(FText::FromString(TEXT("Project info message")));
        Fixture.ActionContext->Warning(FText::FromString(TEXT("Project warning message")));
        Fixture.ActionContext->Error(FText::FromString(TEXT("Project error message")));
        Fixture.ActionContext->Fatal(FText::FromString(TEXT("Project fatal message")));

        Handler->OnProjectRuleApplied(Fixture.ActionContext);

        const auto AllMessages = Listing->GetAllMessagesAsString();
        const auto bAllMessagesLogged = TestEqual(TEXT("The listing should contain four project messages"),
                                                  Listing->NumMessages(EMessageSeverity::Info),
                                                  4)
            && TestTrue(TEXT("The info message should be logged"), AllMessages.Contains(TEXT("Project info message")))
            && TestTrue(TEXT("The warning message should be logged"),
                        AllMessages.Contains(TEXT("Project warning message")))
            && TestTrue(TEXT("The error message should be logged"), AllMessages.Contains(TEXT("Project error message")))
            && TestTrue(TEXT("The fatal message should be logged"),
                        AllMessages.Contains(TEXT("Project fatal message")));

        Listing->ClearMessages();
        return bAllMessagesLogged;
    }
    else
    {
        Listing->ClearMessages();
        return false;
    }
}

#endif
