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
    #include "RuleRanger/Matchers/Path/PathFolderMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerPathFolderMatcherTests
{
    bool SetFolderNames(FAutomationTestBase& Test, UPathFolderMatcher* Matcher, const TArray<FString>& FolderNames)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("FolderNames"), FolderNames);
    }
} // namespace RuleRangerPathFolderMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerPathFolderMatcherMatchesExplicitFolderNamesTest,
                                 "RuleRanger.Matchers.Path.Folder.MatchesExplicitFolderNames",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerPathFolderMatcherMatchesExplicitFolderNamesTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UPathFolderMatcher>();
    const auto Object = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(
        TEXT("/Game/Developers/Tests/RuleRanger/FolderRules/FolderAsset"),
        TEXT("FolderAsset"));
    if (TestNotNull(TEXT("PathFolder matcher should be created"), Matcher)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        if (RuleRangerPathFolderMatcherTests::SetFolderNames(*this, Matcher, { TEXT("FolderRules") })
            && RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("RegexPattern"), FString(TEXT(""))))
        {
            return TestTrue(TEXT("Configured folder names should match path segments"), Matcher->Test(Object));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerPathFolderMatcherHonorsRegexCaseSensitivityTest,
                                 "RuleRanger.Matchers.Path.Folder.HonorsRegexCaseSensitivity",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerPathFolderMatcherHonorsRegexCaseSensitivityTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UPathFolderMatcher>();
    const auto Object = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(
        TEXT("/Game/Developers/Tests/RuleRanger/rules_123/RegexAsset"),
        TEXT("RegexAsset"));
    if (TestNotNull(TEXT("PathFolder matcher should be created"), Matcher)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        if (RuleRangerPathFolderMatcherTests::SetFolderNames(*this, Matcher, {})
            && RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("RegexPattern"), FString(TEXT("^RULES_\\d+$"))))
        {
            const auto bCaseSensitiveMiss =
                TestFalse(TEXT("Regex matching should be case-sensitive by default"), Matcher->Test(Object));
            if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("bCaseSensitive"), false))
            {
                const auto bCaseInsensitiveMatch =
                    TestTrue(TEXT("Regex matching should honor case-insensitive mode"), Matcher->Test(Object));
                return bCaseSensitiveMiss && bCaseInsensitiveMatch;
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
    else
    {
        return false;
    }
}

#endif
