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
    #include "RuleRanger/Matchers/Source/SourceFilenameContainsMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerSourcePathMatcherBaseTests
{
    template <typename TObject>
    bool SetText(FAutomationTestBase& Test, TObject* Matcher, const TCHAR* Text)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Text"), FString(Text));
    }
} // namespace RuleRangerSourcePathMatcherBaseTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerSourceFilenameMatcherReturnsFalseWithoutImportDataPropertyTest,
                                 "RuleRanger.Matchers.Source.FilenameBase.ReturnsFalseWithoutImportDataProperty",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerSourceFilenameMatcherReturnsFalseWithoutImportDataPropertyTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<USourceFilenameContainsMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestObject>();
    if (TestNotNull(TEXT("SourceFilenameContains matcher should be created"), Matcher)
        && TestNotNull(TEXT("Test object should be created"), Object))
    {
        if (RuleRangerSourcePathMatcherBaseTests::SetText(*this, Matcher, TEXT("TestAsset")))
        {
            return TestFalse(TEXT("Objects without AssetImportData should not match source filename matchers"),
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
