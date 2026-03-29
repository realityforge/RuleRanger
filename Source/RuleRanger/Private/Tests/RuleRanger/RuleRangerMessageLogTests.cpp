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

    #include "MessageLogModule.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRangerMessageLog.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerMessageLogInitializeAndShutdownManageRegistrationTest,
                                 "RuleRanger.MessageLog.InitializeAndShutdownManageRegistration",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerMessageLogInitializeAndShutdownManageRegistrationTest::RunTest(const FString&)
{
    const auto& LogName = FRuleRangerMessageLog::GetMessageLogName();
    const auto& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));

    FRuleRangerMessageLog::Shutdown();
    const auto bUnregisteredInitially = !MessageLogModule.IsRegisteredLogListing(LogName);

    FRuleRangerMessageLog::Initialize();
    const auto bRegisteredAfterInitialize = MessageLogModule.IsRegisteredLogListing(LogName);

    FRuleRangerMessageLog::Shutdown();
    const auto bUnregisteredAfterShutdown = !MessageLogModule.IsRegisteredLogListing(LogName);

    FRuleRangerMessageLog::Initialize();

    return TestTrue(TEXT("Shutdown should leave the RuleRanger message log unregistered"), bUnregisteredInitially)
        && TestTrue(TEXT("Initialize should register the RuleRanger message log"), bRegisteredAfterInitialize)
        && TestTrue(TEXT("Shutdown should unregister the RuleRanger message log"), bUnregisteredAfterShutdown)
        && TestTrue(TEXT("The RuleRanger message log should be restored for later tests"),
                    MessageLogModule.IsRegisteredLogListing(LogName));
}

#endif
