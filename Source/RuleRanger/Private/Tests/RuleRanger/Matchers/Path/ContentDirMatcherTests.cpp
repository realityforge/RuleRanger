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
    #include "RuleRanger/Matchers/Path/ContentDirMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerContentDirMatcherTests
{
    bool SetDirectory(FAutomationTestBase& Test, UContentDirMatcher* Matcher, const TCHAR* Path)
    {
        FDirectoryPath Directory;
        Directory.Path = Path;
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Dir"), Directory);
    }
} // namespace RuleRangerContentDirMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerContentDirMatcherMatchesConfiguredDirectoryTest,
                                 "RuleRanger.Matchers.Path.ContentDir.MatchesConfiguredDirectory",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerContentDirMatcherMatchesConfiguredDirectoryTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UContentDirMatcher>();
    const auto Object = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(
        TEXT("/Game/Developers/Tests/RuleRanger/ContentDir/AssetPackage"),
        TEXT("ContentDirAsset"));
    if (TestNotNull(TEXT("ContentDir matcher should be created"), Matcher)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        if (RuleRangerContentDirMatcherTests::SetDirectory(*this,
                                                           Matcher,
                                                           TEXT("/Game/Developers/Tests/RuleRanger/ContentDir")))
        {
            return TestTrue(TEXT("Objects under the configured content directory should match"), Matcher->Test(Object));
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerContentDirMatcherRejectsOtherDirectoriesTest,
                                 "RuleRanger.Matchers.Path.ContentDir.RejectsOtherDirectories",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerContentDirMatcherRejectsOtherDirectoriesTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UContentDirMatcher>();
    const auto Object = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(
        TEXT("/Game/Developers/Tests/RuleRanger/ContentDir/AssetPackage"),
        TEXT("ContentDirAsset"));
    if (TestNotNull(TEXT("ContentDir matcher should be created"), Matcher)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        if (RuleRangerContentDirMatcherTests::SetDirectory(*this,
                                                           Matcher,
                                                           TEXT("/Game/Developers/Tests/RuleRanger/OtherDir")))
        {
            return TestFalse(TEXT("Objects outside the configured content directory should not match"),
                             Matcher->Test(Object));
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

#endif
