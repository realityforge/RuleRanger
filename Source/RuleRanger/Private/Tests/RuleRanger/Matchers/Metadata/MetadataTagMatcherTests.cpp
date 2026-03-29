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
    #include "RuleRanger/Matchers/Metadata/MetadataTagMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerMetadataTagMatcherTests
{
    bool SetKey(FAutomationTestBase& Test, UMetadataTagMatcher* const Matcher, const FName Key)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Key"), Key);
    }

    bool SetValue(FAutomationTestBase& Test, UMetadataTagMatcher* const Matcher, const TCHAR* const Value)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Value"), FString(Value));
    }
} // namespace RuleRangerMetadataTagMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerMetadataTagMatcherMatchesConfiguredTagTest,
                                 "RuleRanger.Matchers.Metadata.Tag.MatchesConfiguredTag",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerMetadataTagMatcherMatchesConfiguredTagTest::RunTest(const FString&)
{
    static const FName VariantKey(TEXT("RuleRanger.Variant"));

    const auto Matcher = RuleRangerTests::NewTransientObject<UMetadataTagMatcher>();
    const auto Material = RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/RuleRangerTests/Metadata/Tag"),
                                                               TEXT("MetadataTagMatchMaterial"));
    if (TestNotNull(TEXT("MetadataTagMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Material should be created"), Material)
        && RuleRangerMetadataTagMatcherTests::SetKey(*this, Matcher, VariantKey)
        && RuleRangerMetadataTagMatcherTests::SetValue(*this, Matcher, TEXT("BaseColor"))
        && TestTrue(TEXT("Asset metadata should be set"),
                    RuleRangerTests::SetAssetMetaData(Material, VariantKey, TEXT("BaseColor"))))
    {
        return TestTrue(TEXT("Objects with the configured metadata key and value should match"),
                        Matcher->Test(Material));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerMetadataTagMatcherRejectsMissingOrMismatchedValuesTest,
                                 "RuleRanger.Matchers.Metadata.Tag.RejectsMissingOrMismatchedValues",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerMetadataTagMatcherRejectsMissingOrMismatchedValuesTest::RunTest(const FString&)
{
    static const FName VariantKey(TEXT("RuleRanger.Variant"));

    const auto Matcher = RuleRangerTests::NewTransientObject<UMetadataTagMatcher>();
    const auto Material =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/RuleRangerTests/Metadata/TagMismatch"),
                                             TEXT("MetadataTagMismatchMaterial"));
    if (TestNotNull(TEXT("MetadataTagMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Material should be created"), Material)
        && RuleRangerMetadataTagMatcherTests::SetKey(*this, Matcher, VariantKey)
        && RuleRangerMetadataTagMatcherTests::SetValue(*this, Matcher, TEXT("BaseColor")))
    {
        const bool bMissingMatches = Matcher->Test(Material);
        if (!TestFalse(TEXT("Objects without the configured metadata should not match"), bMissingMatches))
        {
            return false;
        }
        else if (!TestTrue(TEXT("Asset metadata should be set"),
                           RuleRangerTests::SetAssetMetaData(Material, VariantKey, TEXT("Normal"))))
        {
            return false;
        }
        else
        {
            return TestFalse(TEXT("Objects with a mismatched metadata value should not match"),
                             Matcher->Test(Material));
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerMetadataTagMatcherRejectsNullObjectsAndNoneKeysTest,
                                 "RuleRanger.Matchers.Metadata.Tag.RejectsNullObjectsAndNoneKeys",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerMetadataTagMatcherRejectsNullObjectsAndNoneKeysTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UMetadataTagMatcher>();
    const auto Material =
        RuleRangerTests::NewPackagedMaterial(TEXT("/Game/Developers/RuleRangerTests/Metadata/TagInvalid"),
                                             TEXT("MetadataTagInvalidMaterial"));
    if (TestNotNull(TEXT("MetadataTagMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Material should be created"), Material)
        && RuleRangerMetadataTagMatcherTests::SetValue(*this, Matcher, TEXT("BaseColor")))
    {
        const bool bNullMatches = Matcher->Test(nullptr);
        const bool bNoneKeyMatches = Matcher->Test(Material);
        return TestFalse(TEXT("Null objects should not match metadata tags"), bNullMatches)
            && TestFalse(TEXT("A None metadata key should not match"), bNoneKeyMatches);
    }
    else
    {
        return false;
    }
}

#endif
