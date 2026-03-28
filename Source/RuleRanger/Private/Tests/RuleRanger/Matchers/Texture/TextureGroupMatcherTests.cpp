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

    #include "Engine/Texture.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Matchers/Texture/TextureGroupMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerTextureGroupMatcherTests
{
    bool SetTextureGroups(FAutomationTestBase& Test,
                          UTextureGroupMatcher* const Matcher,
                          const TSet<TEnumAsByte<TextureGroup>>& TextureGroups)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("TextureGroups"), TextureGroups);
    }
} // namespace RuleRangerTextureGroupMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerTextureGroupMatcherMatchesConfiguredGroupTest,
                                 "RuleRanger.Matchers.Texture.TextureGroup.MatchesConfiguredGroup",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerTextureGroupMatcherMatchesConfiguredGroupTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UTextureGroupMatcher>();
    const auto Texture = RuleRangerTests::NewTransientTexture2D(64, 64, TEXT("TextureGroupMatch"));
    if (TestNotNull(TEXT("TextureGroupMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        Texture->LODGroup = TEXTUREGROUP_World;
        if (RuleRangerTextureGroupMatcherTests::SetTextureGroups(
                *this,
                Matcher,
                { TEnumAsByte(TEXTUREGROUP_UI), TEnumAsByte(TEXTUREGROUP_World) }))
        {
            return TestTrue(TEXT("Textures in one of the configured texture groups should match"),
                            Matcher->Test(Texture));
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerTextureGroupMatcherRejectsOtherGroupsTest,
                                 "RuleRanger.Matchers.Texture.TextureGroup.RejectsOtherGroups",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerTextureGroupMatcherRejectsOtherGroupsTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UTextureGroupMatcher>();
    const auto Texture = RuleRangerTests::NewTransientTexture2D(64, 64, TEXT("TextureGroupReject"));
    if (TestNotNull(TEXT("TextureGroupMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        Texture->LODGroup = TEXTUREGROUP_World;
        if (RuleRangerTextureGroupMatcherTests::SetTextureGroups(
                *this,
                Matcher,
                { TEnumAsByte(TEXTUREGROUP_UI), TEnumAsByte(TEXTUREGROUP_Pixels2D) }))
        {
            return TestFalse(TEXT("Textures outside the configured texture groups should not match"),
                             Matcher->Test(Texture));
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerTextureGroupMatcherRejectsNullAndWrongTypeTest,
                                 "RuleRanger.Matchers.Texture.TextureGroup.RejectsNullAndWrongType",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerTextureGroupMatcherRejectsNullAndWrongTypeTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UTextureGroupMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestObject>();
    if (TestNotNull(TEXT("TextureGroupMatcher should be created"), Matcher)
        && TestNotNull(TEXT("Non-texture object should be created"), Object))
    {
        if (RuleRangerTextureGroupMatcherTests::SetTextureGroups(*this, Matcher, { TEnumAsByte(TEXTUREGROUP_World) }))
        {
            return TestFalse(TEXT("Null should not match"), Matcher->Test(nullptr))
                && TestFalse(TEXT("Objects that are not textures should not match"), Matcher->Test(Object));
        }
    }

    return false;
}

#endif
