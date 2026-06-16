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
    #include "RuleRanger/UI/RuleRangerDeveloperSettings.h"
    #include "RuleRanger/UI/RuleRangerEditorSubsystem.h"
    #include "RuleRangerConfig.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerDeveloperSettingsExposeExpectedSettingsLocationTest,
                                 "RuleRanger.UI.DeveloperSettings.ExposeExpectedSettingsLocation",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerDeveloperSettingsExposeExpectedSettingsLocationTest::RunTest(const FString&)
{
    const auto Settings = GetDefault<URuleRangerDeveloperSettings>();
    if (!TestNotNull(TEXT("Developer settings should be available"), Settings))
    {
        return false;
    }

    return TestEqual(TEXT("Settings should live in the Project container"),
                     Settings->GetContainerName(),
                     FName(TEXT("Project")))
        && TestEqual(TEXT("Settings should live in the Editor category"),
                     Settings->GetCategoryName(),
                     FName(TEXT("Editor")))
        && TestEqual(TEXT("Settings should use the class-derived section"),
                     Settings->GetSectionName(),
                     FName(TEXT("RuleRangerDeveloperSettings")));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerDeveloperSettingsPostEditInvalidatesSubsystemConfigCacheTest,
                                 "RuleRanger.UI.DeveloperSettings.PostEditInvalidatesSubsystemConfigCache",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerDeveloperSettingsPostEditInvalidatesSubsystemConfigCacheTest::RunTest(const FString&)
{
    const auto Subsystem = GEditor ? GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>() : nullptr;
    const auto Settings = GetMutableDefault<URuleRangerDeveloperSettings>();
    if (!TestNotNull(TEXT("RuleRanger editor subsystem should be available"), Subsystem)
        || !TestNotNull(TEXT("Developer settings should be available"), Settings))
    {
        return false;
    }

    const auto FirstConfig = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    const auto SecondConfig = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
    if (!TestNotNull(TEXT("First config should be created"), FirstConfig)
        || !TestNotNull(TEXT("Second config should be created"), SecondConfig))
    {
        return false;
    }

    FirstConfig->Dirs.Add({ TEXT("/Game/RuleRangerFirst/") });
    SecondConfig->Dirs.Add({ TEXT("/Game/RuleRangerSecond/") });

    RuleRangerTests::FScopedRuleRangerDeveloperSettingsOverride SettingsOverride({ FirstConfig });

    TArray<FString> Paths;
    Subsystem->CollectConfiguredPaths(Paths);
    const bool bInitialCacheUsedFirstConfig =
        TestEqual(TEXT("The cache should initially expose the first configured path"),
                  Paths[0],
                  FString(TEXT("/Game/RuleRangerFirst/")));

    Settings->Configs = { SecondConfig };
    FPropertyChangedEvent Event(FindFProperty<FProperty>(Settings->GetClass(), TEXT("Configs")));
    Settings->PostEditChangeProperty(Event);

    Paths.Reset();
    Subsystem->CollectConfiguredPaths(Paths);

    return bInitialCacheUsedFirstConfig
        && TestEqual(TEXT("PostEditChangeProperty should refresh cached config paths"),
                     Paths[0],
                     FString(TEXT("/Game/RuleRangerSecond/")));
}

#endif
