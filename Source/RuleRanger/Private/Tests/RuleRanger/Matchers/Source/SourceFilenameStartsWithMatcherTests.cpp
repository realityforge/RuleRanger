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
    #include "RuleRanger/Matchers/Source/SourceFilenameStartsWithMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerSourceFilenameStartsWithMatcherTests
{
    bool SetText(FAutomationTestBase& Test, USourceFilenameStartsWithMatcher* Matcher, const TCHAR* Text)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Text"), FString(Text));
    }
} // namespace RuleRangerSourceFilenameStartsWithMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerSourceFilenameStartsWithMatcherHonorsCaseSensitivityTest,
                                 "RuleRanger.Matchers.Source.StartsWith.HonorsCaseSensitivity",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerSourceFilenameStartsWithMatcherHonorsCaseSensitivityTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<USourceFilenameStartsWithMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationImportDataObject>();
    if (TestNotNull(TEXT("SourceFilenameStartsWith matcher should be created"), Matcher)
        && TestNotNull(TEXT("Import-data object should be created"), Object))
    {
        if (RuleRangerTests::SetImportFilename(*this, Object, TEXT("C:/Imports/SM_TestAsset_LOD0.fbx"))
            && RuleRangerSourceFilenameStartsWithMatcherTests::SetText(*this, Matcher, TEXT("sm_")))
        {
            const auto bCaseInsensitiveMatch =
                TestTrue(TEXT("StartsWith matching should be case-insensitive by default"), Matcher->Test(Object));
            if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("bCaseSensitive"), true))
            {
                const auto bCaseSensitiveMiss =
                    TestFalse(TEXT("StartsWith matching should honor case-sensitive mode"), Matcher->Test(Object));
                return bCaseInsensitiveMatch && bCaseSensitiveMiss;
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
