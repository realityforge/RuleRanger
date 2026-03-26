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
    #include "RuleRanger/Actions/Path/CheckFolderNamesAreValidAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerCheckFolderNamesAreValidActionTests
{
    TArray<FString> MakeStringArray(std::initializer_list<const TCHAR*> Values)
    {
        TArray<FString> Result;
        for (const auto Value : Values)
        {
            Result.Add(Value);
        }
        return Result;
    }

    bool SetStringArray(FAutomationTestBase& Test,
                        UObject* Object,
                        const TCHAR* const PropertyName,
                        std::initializer_list<const TCHAR*> Values)
    {
        return RuleRangerTests::SetPropertyValue(Test, Object, PropertyName, MakeStringArray(Values));
    }

    bool SetString(FAutomationTestBase& Test, UObject* Object, const TCHAR* const PropertyName, const TCHAR* Value)
    {
        return RuleRangerTests::SetPropertyValue(Test, Object, PropertyName, FString(Value));
    }
} // namespace RuleRangerCheckFolderNamesAreValidActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckFolderNamesRejectsDefaultInvalidFolderNamesTest,
                                 "RuleRanger.Actions.Path.CheckFolderNames.RejectsDefaultInvalidFolderNames",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckFolderNamesRejectsDefaultInvalidFolderNamesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UCheckFolderNamesAreValidAction>();
    const auto Object = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(TEXT("/Game/Assets/Weapon"),
                                                                                            TEXT("Weapon"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("CheckFolderNamesAreValidAction should be created"), Action)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        Action->Apply(Fixture.ActionContext, Object);

        return TestEqual(TEXT("Default invalid folder names should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(*this,
                                                      Fixture.ActionContext->GetErrorMessages(),
                                                      TEXT("Invalid folder errors should mention the folder name"),
                                                      TEXT("Assets"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckFolderNamesUsesConfiguredMessageTest,
                                 "RuleRanger.Actions.Path.CheckFolderNames.UsesConfiguredMessage",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckFolderNamesUsesConfiguredMessageTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UCheckFolderNamesAreValidAction>();
    const auto Object = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(TEXT("/Game/Assets/Weapon"),
                                                                                            TEXT("Weapon"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("CheckFolderNamesAreValidAction should be created"), Action)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        if (RuleRangerCheckFolderNamesAreValidActionTests::SetString(*this,
                                                                     Action,
                                                                     TEXT("Message"),
                                                                     TEXT("Custom folder failure")))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Configured folder-message failures should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Configured folder-message failures should preserve the message"),
                       TEXT("Custom folder failure"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckFolderNamesHonorsRegexCaseSensitivityTest,
                                 "RuleRanger.Actions.Path.CheckFolderNames.HonorsRegexCaseSensitivity",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckFolderNamesHonorsRegexCaseSensitivityTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture CaseSensitiveFixture;
    RuleRangerTests::FRuleFixture CaseInsensitiveFixture;
    const auto Action = RuleRangerTests::NewTransientObject<UCheckFolderNamesAreValidAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(TEXT("/Game/weapons/Weapon"),
                                                                            TEXT("Weapon"));
    if (RuleRangerTests::CreateRuleFixture(*this, CaseSensitiveFixture)
        && RuleRangerTests::CreateRuleFixture(*this, CaseInsensitiveFixture)
        && TestNotNull(TEXT("CheckFolderNamesAreValidAction should be created"), Action)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        if (RuleRangerCheckFolderNamesAreValidActionTests::SetStringArray(*this, Action, TEXT("InvalidNames"), {})
            && RuleRangerCheckFolderNamesAreValidActionTests::SetString(*this,
                                                                        Action,
                                                                        TEXT("ValidFolderPattern"),
                                                                        TEXT("^[A-Z][a-z]+$")))
        {
            Action->Apply(CaseSensitiveFixture.ActionContext, Object);
            const auto bCaseSensitiveFailure =
                TestEqual(TEXT("Case-sensitive regex evaluation should reject lowercase folder names"),
                          CaseSensitiveFixture.ActionContext->GetErrorMessages().Num(),
                          1);

            if (RuleRangerTests::SetPropertyValue(*this, Action, TEXT("bCaseSensitive"), false))
            {
                Action->Apply(CaseInsensitiveFixture.ActionContext, Object);

                return bCaseSensitiveFailure
                    && TestTrue(TEXT("Case-insensitive regex evaluation should allow lowercase folder names"),
                                CaseInsensitiveFixture.ActionContext->GetErrorMessages().IsEmpty());
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckFolderNamesSkipsExternalActorAndObjectPackagesTest,
                                 "RuleRanger.Actions.Path.CheckFolderNames.SkipsExternalActorAndObjectPackages",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckFolderNamesSkipsExternalActorAndObjectPackagesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UCheckFolderNamesAreValidAction>();
    const auto Object = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(
        TEXT("/Game/__ExternalActors__/Maps/TestMap/TestActor"),
        TEXT("TestActor"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("CheckFolderNamesAreValidAction should be created"), Action)
        && TestNotNull(TEXT("External package object should be created"), Object))
    {
        Action->Apply(Fixture.ActionContext, Object);

        return TestTrue(TEXT("External actor and object packages should be ignored"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

#endif
