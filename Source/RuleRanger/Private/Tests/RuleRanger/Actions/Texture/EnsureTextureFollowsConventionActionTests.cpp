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
    #include "RuleRanger/Actions/Texture/EnsureTextureFollowsConventionAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureTextureFollowsConventionActionTests
{
    static const FName VariantKey(TEXT("RuleRanger.Variant"));

    bool SetConventionsTables(FAutomationTestBase& Test,
                              UEnsureTextureFollowsConventionAction* const Action,
                              const TArray<TObjectPtr<UDataTable>>& Tables)
    {
        return RuleRangerTests::SetPropertyValue(Test, Action, TEXT("ConventionsTables"), Tables);
    }

    UDataTable* CreateTextureConventionTable(const TCHAR* const PackageName,
                                             const TCHAR* const ObjectName,
                                             const FName RowName,
                                             const TCHAR* const Suffix,
                                             const TextureGroup Group)
    {
        const auto Table =
            RuleRangerTests::NewDataTable(PackageName, ObjectName, FRuleRangerTextureConvention::StaticStruct());
        if (Table)
        {
            FRuleRangerTextureConvention Convention;
            Convention.Suffix = Suffix;
            Convention.ColorSpace = ERuleRangerTextureColorSpace::SRGB;
            Convention.TextureCompressionSettings = { TEnumAsByte<TextureCompressionSettings>(TC_Default) };
            Convention.TextureGroups = { TEnumAsByte<TextureGroup>(Group) };
            Convention.TextureMipGenSettings = { TEnumAsByte<TextureMipGenSettings>(TMGS_FromTextureGroup) };
            Convention.TextureResolutionConstraint = ETextureResolutionConstraint::PowerOfTwo;
            RuleRangerTests::AddDataTableRow(Table, RowName, Convention);
        }
        return Table;
    }
} // namespace RuleRangerEnsureTextureFollowsConventionActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureTextureFollowsConventionActionUsesConfigTablesForVariantChecksTest,
    "RuleRanger.Actions.Texture.EnsureTextureFollowsConvention.UsesConfigTablesForVariantChecks",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureFollowsConventionActionUsesConfigTablesForVariantChecksTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureFollowsConventionAction>();
    const auto Table = RuleRangerEnsureTextureFollowsConventionActionTests::CreateTextureConventionTable(
        TEXT("/Game/Developers/Tests/RuleRanger/Convention/Texture/ConfigTable"),
        TEXT("TextureConventionConfigTable"),
        TEXT("BaseColor"),
        TEXT("_BC"),
        TEXTUREGROUP_World);
    const auto Texture = RuleRangerTests::NewPackagedTexture2D(
        TEXT("/Game/Developers/Tests/RuleRanger/Convention/Texture/ConfigTexture"),
        TEXT("Stone_BC"),
        128,
        128);
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureTextureFollowsConventionAction should be created"), Action)
        && TestNotNull(TEXT("Convention table should be created"), Table)
        && TestNotNull(TEXT("Texture should be created"), Texture))
    {
        Fixture.Config->DataTables = { Table };
        Texture->LODGroup = TEXTUREGROUP_World;
        Texture->CompressionSettings = TC_Default;
        Texture->MipGenSettings = TMGS_FromTextureGroup;
        Texture->SRGB = true;
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, Texture);
        return TestEqual(TEXT("Missing variant metadata should add one dry-run error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention the missing variant metadata"),
                                                      TEXT("RuleRanger.Variant"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureTextureFollowsConventionActionRejectsInvalidConfiguredSettingsTest,
    "RuleRanger.Actions.Texture.EnsureTextureFollowsConvention.RejectsInvalidConfiguredSettings",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureFollowsConventionActionRejectsInvalidConfiguredSettingsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureFollowsConventionAction>();
    const auto Table = RuleRangerEnsureTextureFollowsConventionActionTests::CreateTextureConventionTable(
        TEXT("/Game/Developers/Tests/RuleRanger/Convention/Texture/PropertyTable"),
        TEXT("TextureConventionPropertyTable"),
        TEXT("BaseColor"),
        TEXT("_BC"),
        TEXTUREGROUP_UI);
    const auto Texture = RuleRangerTests::NewPackagedTexture2D(
        TEXT("/Game/Developers/Tests/RuleRanger/Convention/Texture/PropertyTexture"),
        TEXT("Brick_BC"),
        128,
        128);
    const TArray<TObjectPtr<UDataTable>> Tables{ Table };
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureTextureFollowsConventionAction should be created"), Action)
        && TestNotNull(TEXT("Convention table should be created"), Table)
        && TestNotNull(TEXT("Texture should be created"), Texture)
        && RuleRangerEnsureTextureFollowsConventionActionTests::SetConventionsTables(*this, Action, Tables)
        && TestTrue(TEXT("Texture variant metadata should be set"),
                    RuleRangerTests::SetAssetMetaData(Texture,
                                                      RuleRangerEnsureTextureFollowsConventionActionTests::VariantKey,
                                                      TEXT("BaseColor"))))
    {
        Texture->LODGroup = TEXTUREGROUP_World;
        Texture->CompressionSettings = TC_Default;
        Texture->MipGenSettings = TMGS_FromTextureGroup;
        Texture->SRGB = true;
        RuleRangerTests::ResetRuleFixtureObject(Fixture, Texture, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, Texture);
        return TestEqual(TEXT("Invalid configured texture settings should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("The error should mention the configured texture group"),
                                                      TEXT("TextureGroup"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureTextureFollowsConventionActionSupportsRepeatedCachedAppliesTest,
    "RuleRanger.Actions.Texture.EnsureTextureFollowsConvention.SupportsRepeatedCachedApplies",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureTextureFollowsConventionActionSupportsRepeatedCachedAppliesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureTextureFollowsConventionAction>();
    const auto Table = RuleRangerEnsureTextureFollowsConventionActionTests::CreateTextureConventionTable(
        TEXT("/Game/Developers/Tests/RuleRanger/Convention/Texture/CachedTable"),
        TEXT("TextureConventionCachedTable"),
        TEXT("BaseColor"),
        TEXT("_BC"),
        TEXTUREGROUP_World);
    const auto FirstTexture =
        RuleRangerTests::NewPackagedTexture2D(TEXT("/Game/Developers/Tests/RuleRanger/Convention/Texture/CachedFirst"),
                                              TEXT("Rock_BC"),
                                              128,
                                              128);
    const auto SecondTexture =
        RuleRangerTests::NewPackagedTexture2D(TEXT("/Game/Developers/Tests/RuleRanger/Convention/Texture/CachedSecond"),
                                              TEXT("Mud_BC"),
                                              128,
                                              128);
    const TArray<TObjectPtr<UDataTable>> Tables{ Table };
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureTextureFollowsConventionAction should be created"), Action)
        && TestNotNull(TEXT("Convention table should be created"), Table)
        && TestNotNull(TEXT("First texture should be created"), FirstTexture)
        && TestNotNull(TEXT("Second texture should be created"), SecondTexture)
        && RuleRangerEnsureTextureFollowsConventionActionTests::SetConventionsTables(*this, Action, Tables)
        && TestTrue(TEXT("First texture variant metadata should be set"),
                    RuleRangerTests::SetAssetMetaData(FirstTexture,
                                                      RuleRangerEnsureTextureFollowsConventionActionTests::VariantKey,
                                                      TEXT("BaseColor")))
        && TestTrue(TEXT("Second texture variant metadata should be set"),
                    RuleRangerTests::SetAssetMetaData(SecondTexture,
                                                      RuleRangerEnsureTextureFollowsConventionActionTests::VariantKey,
                                                      TEXT("BaseColor"))))
    {
        FirstTexture->LODGroup = TEXTUREGROUP_World;
        FirstTexture->CompressionSettings = TC_Default;
        FirstTexture->MipGenSettings = TMGS_FromTextureGroup;
        FirstTexture->SRGB = true;

        SecondTexture->LODGroup = TEXTUREGROUP_World;
        SecondTexture->CompressionSettings = TC_Default;
        SecondTexture->MipGenSettings = TMGS_FromTextureGroup;
        SecondTexture->SRGB = true;

        RuleRangerTests::ResetRuleFixtureObject(Fixture, FirstTexture, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, FirstTexture);
        const bool bFirstPasses = TestTrue(TEXT("The first cached apply should not add errors"),
                                           Fixture.ActionContext->GetErrorMessages().IsEmpty());

        RuleRangerTests::ResetRuleFixtureObject(Fixture, SecondTexture, ERuleRangerActionTrigger::AT_Report);
        Action->Apply(Fixture.ActionContext, SecondTexture);
        const bool bSecondPasses = TestTrue(TEXT("The second cached apply should not add errors"),
                                            Fixture.ActionContext->GetErrorMessages().IsEmpty());

        return bFirstPasses && bSecondPasses;
    }
    else
    {
        return false;
    }
}

#endif
