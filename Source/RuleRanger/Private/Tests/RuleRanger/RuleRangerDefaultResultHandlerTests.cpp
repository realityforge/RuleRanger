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
    #include "RuleRangerDefaultResultHandler.h"
    #include "RuleRangerMessageLog.h"
    #include "RuleRangerResultHandler.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerDefaultResultHandlerImplementsResultHandlerInterfaceTest,
                                 "RuleRanger.ResultHandler.Interface.DefaultHandlerImplementsInterface",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerDefaultResultHandlerImplementsResultHandlerInterfaceTest::RunTest(const FString&)
{
    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerDefaultResultHandler>();
    if (TestNotNull(TEXT("Default result handler should be created"), Handler))
    {
        return TestTrue(TEXT("Default result handler should implement the result handler interface"),
                        Handler->GetClass()->ImplementsInterface(URuleRangerResultHandler::StaticClass()));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerDefaultResultHandlerWritesContextMessagesToMessageLogTest,
                                 "RuleRanger.ResultHandler.Default.WritesContextMessagesToMessageLog",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerDefaultResultHandlerWritesContextMessagesToMessageLogTest::RunTest(const FString&)
{
    FRuleRangerMessageLog::Initialize();

    auto& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
    const auto Listing = MessageLogModule.GetLogListing(FRuleRangerMessageLog::GetMessageLogName());
    Listing->ClearMessages();

    const auto Handler = RuleRangerTests::NewTransientObject<URuleRangerDefaultResultHandler>();
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("Default result handler should be created"), Handler))
    {
        AddExpectedMessagePlain(TEXT("Action warning message"),
                                ELogVerbosity::Warning,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        AddExpectedMessagePlain(TEXT("Action error message"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        AddExpectedMessagePlain(TEXT("Action fatal message"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);

        Fixture.ActionContext->Info(FText::FromString(TEXT("Action info message")));
        Fixture.ActionContext->Warning(FText::FromString(TEXT("Action warning message")));
        Fixture.ActionContext->Error(FText::FromString(TEXT("Action error message")));
        Fixture.ActionContext->Fatal(FText::FromString(TEXT("Action fatal message")));

        Handler->OnRuleApplied(Fixture.ActionContext);

        const auto AllMessages = Listing->GetAllMessagesAsString();
        const auto bAllMessagesLogged = TestEqual(TEXT("The listing should contain four action messages"),
                                                  Listing->NumMessages(EMessageSeverity::Info),
                                                  4)
            && TestTrue(TEXT("The info message should be logged"), AllMessages.Contains(TEXT("Action info message")))
            && TestTrue(TEXT("The warning message should be logged"),
                        AllMessages.Contains(TEXT("Action warning message")))
            && TestTrue(TEXT("The error message should be logged"), AllMessages.Contains(TEXT("Action error message")))
            && TestTrue(TEXT("The fatal message should be logged"), AllMessages.Contains(TEXT("Action fatal message")))
            && TestTrue(TEXT("The object token should be logged"), AllMessages.Contains(Fixture.Object->GetName()))
            && TestTrue(TEXT("The rule token should be logged"), AllMessages.Contains(Fixture.Rule->GetName()))
            && TestTrue(TEXT("The rule set token should be logged"), AllMessages.Contains(Fixture.RuleSet->GetName()));

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
