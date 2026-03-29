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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerObjectBaseLogInfoHandlesObjectAndNullTest,
                                 "RuleRanger.ObjectBase.LogInfo.HandlesObjectAndNull",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerObjectBaseLogInfoHandlesObjectAndNullTest::RunTest(const FString&)
{
    const auto Probe = RuleRangerTests::NewTransientObject<URuleRangerAutomationObjectBaseProbe>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestObject>();
    if (TestNotNull(TEXT("Object-base probe should be created"), Probe)
        && TestNotNull(TEXT("Test object should be created"), Object))
    {
        Probe->EmitInfo(TEXT("Probe info without object"));
        Probe->EmitInfoForObject(Object, TEXT("Probe info with object"));
        return true;
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerObjectBaseLogErrorHandlesObjectAndNullTest,
                                 "RuleRanger.ObjectBase.LogError.HandlesObjectAndNull",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerObjectBaseLogErrorHandlesObjectAndNullTest::RunTest(const FString&)
{
    const auto Probe = RuleRangerTests::NewTransientObject<URuleRangerAutomationObjectBaseProbe>();
    const auto Object = RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("RuleProbe"));
    if (TestNotNull(TEXT("Object-base probe should be created"), Probe)
        && TestNotNull(TEXT("Named test object should be created"), Object))
    {
        AddExpectedMessagePlain(TEXT("Probe error without object"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        AddExpectedMessagePlain(TEXT("RuleProbe: Probe error with object"),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);

        Probe->EmitError(TEXT("Probe error without object"));
        Probe->EmitErrorForObject(Object, TEXT("Probe error with object"));
        return true;
    }
    else
    {
        return false;
    }
}

#endif
