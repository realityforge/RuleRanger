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
    #include "RuleRanger/Matchers/Metadata/MetadataTagPresent.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerMetadataTagPresentTests
{
    bool SetKey(FAutomationTestBase& Test, UMetadataTagPresent* const Matcher, const FName Key)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Key"), Key);
    }
} // namespace RuleRangerMetadataTagPresentTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerMetadataTagPresentMatchesPresentTagTest,
                                 "RuleRanger.Matchers.Metadata.TagPresent.MatchesPresentTag",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerMetadataTagPresentMatchesPresentTagTest::RunTest(const FString&)
{
    static const FName VariantKey(TEXT("RuleRanger.Variant"));

    const auto Matcher = RuleRangerTests::NewTransientObject<UMetadataTagPresent>();
    const auto Material =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/RuleRangerTests/Metadata/Present"),
                                             TEXT("MetadataTagPresentMaterial"));
    if (TestNotNull(TEXT("MetadataTagPresent should be created"), Matcher)
        && TestNotNull(TEXT("Material should be created"), Material)
        && RuleRangerMetadataTagPresentTests::SetKey(*this, Matcher, VariantKey)
        && TestTrue(TEXT("Package metadata should be set"),
                    RuleRangerTests::SetPackageMetaData(Material, VariantKey, TEXT("BaseColor"))))
    {
        return TestTrue(TEXT("Objects with package metadata for the configured key should match"),
                        Matcher->Test(Material));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerMetadataTagPresentRejectsMissingTagsTest,
                                 "RuleRanger.Matchers.Metadata.TagPresent.RejectsMissingTags",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerMetadataTagPresentRejectsMissingTagsTest::RunTest(const FString&)
{
    static const FName VariantKey(TEXT("RuleRanger.Variant"));

    const auto Matcher = RuleRangerTests::NewTransientObject<UMetadataTagPresent>();
    const auto Material =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/RuleRangerTests/Metadata/PresentMissing"),
                                             TEXT("MetadataTagMissingMaterial"));
    if (TestNotNull(TEXT("MetadataTagPresent should be created"), Matcher)
        && TestNotNull(TEXT("Material should be created"), Material)
        && RuleRangerMetadataTagPresentTests::SetKey(*this, Matcher, VariantKey))
    {
        return TestFalse(TEXT("Objects without package metadata for the configured key should not match"),
                         Matcher->Test(Material));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerMetadataTagPresentRejectsNullObjectsAndNoneKeysTest,
                                 "RuleRanger.Matchers.Metadata.TagPresent.RejectsNullObjectsAndNoneKeys",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerMetadataTagPresentRejectsNullObjectsAndNoneKeysTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UMetadataTagPresent>();
    const auto Material =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/RuleRangerTests/Metadata/PresentInvalid"),
                                             TEXT("MetadataTagPresentInvalidMaterial"));
    if (TestNotNull(TEXT("MetadataTagPresent should be created"), Matcher)
        && TestNotNull(TEXT("Material should be created"), Material))
    {
        return TestFalse(TEXT("Null objects should not match"), Matcher->Test(nullptr))
            && TestFalse(TEXT("A None metadata key should not match"), Matcher->Test(Material));
    }
    else
    {
        return false;
    }
}

#endif
