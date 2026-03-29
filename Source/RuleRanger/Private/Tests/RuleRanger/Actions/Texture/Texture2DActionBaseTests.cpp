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

    #include "Engine/Texture2D.h"
    #include "Misc/AutomationTest.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerTexture2DActionBaseExpectedTypeIsTexture2DTest,
                                 "RuleRanger.Actions.Texture.Texture2DActionBase.ExpectedTypeIsTexture2D",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerTexture2DActionBaseExpectedTypeIsTexture2DTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<URuleRangerAutomationTexture2DAction>();
    if (TestNotNull(TEXT("Automation Texture2D action should be created"), Action))
    {
        return TestEqual(TEXT("Texture2DActionBase should require Texture2D assets"),
                         Action->GetExpectedType(),
                         UTexture2D::StaticClass());
    }
    else
    {
        return false;
    }
}

#endif
