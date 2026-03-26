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
    #include "RuleRanger/Matchers/Source/SourceFilenameEndsWithMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerSourceFilenameEndsWithMatcherTests
{
    bool SetText(FAutomationTestBase& Test, USourceFilenameEndsWithMatcher* Matcher, const TCHAR* Text)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Text"), FString(Text));
    }
} // namespace RuleRangerSourceFilenameEndsWithMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerSourceFilenameEndsWithMatcherHonorsCaseSensitivityTest,
                                 "RuleRanger.Matchers.Source.EndsWith.HonorsCaseSensitivity",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerSourceFilenameEndsWithMatcherHonorsCaseSensitivityTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<USourceFilenameEndsWithMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationImportDataObject>();
    if (TestNotNull(TEXT("SourceFilenameEndsWith matcher should be created"), Matcher)
        && TestNotNull(TEXT("Import-data object should be created"), Object))
    {
        if (RuleRangerTests::SetImportFilename(*this, Object, TEXT("C:/Imports/SM_TestAsset_LOD0.fbx"))
            && RuleRangerSourceFilenameEndsWithMatcherTests::SetText(*this, Matcher, TEXT("lod0")))
        {
            const auto bCaseInsensitiveMatch =
                TestTrue(TEXT("EndsWith matching should be case-insensitive by default"), Matcher->Test(Object));
            if (RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("bCaseSensitive"), true))
            {
                const auto bCaseSensitiveMiss =
                    TestFalse(TEXT("EndsWith matching should honor case-sensitive mode"), Matcher->Test(Object));
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
