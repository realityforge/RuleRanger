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
    #include "RuleRangerLogging.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerLoggingCategoryEmitsDeterministicWarningsTest,
                                 "RuleRanger.Logging.Category.EmitsDeterministicWarnings",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerLoggingCategoryEmitsDeterministicWarningsTest::RunTest(const FString&)
{
    AddExpectedMessagePlain(TEXT("RuleRanger automation logging smoke"),
                            ELogVerbosity::Warning,
                            EAutomationExpectedMessageFlags::Contains,
                            1);

    UE_LOG(LogRuleRanger, Warning, TEXT("RuleRanger automation logging smoke"));
    return true;
}

#endif
