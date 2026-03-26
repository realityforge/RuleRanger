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
    #include "RuleRanger/Actions/Path/CheckTopLevelFolderContentIsValidAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerCheckTopLevelFolderContentIsValidActionTests
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
} // namespace RuleRangerCheckTopLevelFolderContentIsValidActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckTopLevelFolderAcceptsRootAssetsByNameTest,
                                 "RuleRanger.Actions.Path.CheckTopLevelFolder.AcceptsRootAssetsByName",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckTopLevelFolderAcceptsRootAssetsByNameTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UCheckTopLevelFolderContentIsValidAction>();
    const auto Object = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(TEXT("/Game/AllowedAsset"),
                                                                                            TEXT("AllowedAsset"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("CheckTopLevelFolderContentIsValidAction should be created"), Action)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        if (RuleRangerCheckTopLevelFolderContentIsValidActionTests::SetStringArray(*this,
                                                                                   Action,
                                                                                   TEXT("ValidAssetNames"),
                                                                                   { TEXT("AllowedAsset") }))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestTrue(TEXT("Root assets listed in ValidAssetNames should be accepted"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty());
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckTopLevelFolderAcceptsTopLevelFoldersByRegexTest,
                                 "RuleRanger.Actions.Path.CheckTopLevelFolder.AcceptsTopLevelFoldersByRegex",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckTopLevelFolderAcceptsTopLevelFoldersByRegexTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UCheckTopLevelFolderContentIsValidAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(TEXT("/Game/Characters/Hero"),
                                                                            TEXT("Hero"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("CheckTopLevelFolderContentIsValidAction should be created"), Action)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        if (RuleRangerCheckTopLevelFolderContentIsValidActionTests::SetString(*this,
                                                                              Action,
                                                                              TEXT("ValidFolderRegexPattern"),
                                                                              TEXT("^Char.*$")))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestTrue(TEXT("Top-level folders matched by regex should be accepted"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty());
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerCheckTopLevelFolderUsesConfiguredMessageForInvalidRootAssetsTest,
    "RuleRanger.Actions.Path.CheckTopLevelFolder.UsesConfiguredMessageForInvalidRootAssets",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckTopLevelFolderUsesConfiguredMessageForInvalidRootAssetsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UCheckTopLevelFolderContentIsValidAction>();
    const auto Object = RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(TEXT("/Game/DeniedAsset"),
                                                                                            TEXT("DeniedAsset"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("CheckTopLevelFolderContentIsValidAction should be created"), Action)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        if (RuleRangerCheckTopLevelFolderContentIsValidActionTests::SetString(*this,
                                                                              Action,
                                                                              TEXT("Message"),
                                                                              TEXT("Custom top-level failure")))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestEqual(TEXT("Invalid root assets should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Invalid root assets should preserve the configured message"),
                       TEXT("Custom top-level failure"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckTopLevelFolderRejectsInvalidTopLevelFoldersTest,
                                 "RuleRanger.Actions.Path.CheckTopLevelFolder.RejectsInvalidTopLevelFolders",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckTopLevelFolderRejectsInvalidTopLevelFoldersTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UCheckTopLevelFolderContentIsValidAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(TEXT("/Game/Misc/Hero"), TEXT("Hero"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("CheckTopLevelFolderContentIsValidAction should be created"), Action)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        if (RuleRangerCheckTopLevelFolderContentIsValidActionTests::SetStringArray(*this,
                                                                                   Action,
                                                                                   TEXT("ValidFolderNames"),
                                                                                   { TEXT("Characters") }))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestEqual(TEXT("Invalid top-level folders should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Invalid top-level folder errors should mention the folder"),
                       TEXT("Misc"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCheckTopLevelFolderLogsAndSkipsObjectsOutsideGameTest,
                                 "RuleRanger.Actions.Path.CheckTopLevelFolder.LogsAndSkipsObjectsOutsideGame",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCheckTopLevelFolderLogsAndSkipsObjectsOutsideGameTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UCheckTopLevelFolderContentIsValidAction>();
    const auto Object =
        RuleRangerTests::NewPackagedObject<URuleRangerAutomationTestObject>(TEXT("/Engine/Test/Hero"), TEXT("Hero"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("CheckTopLevelFolderContentIsValidAction should be created"), Action)
        && TestNotNull(TEXT("Packaged object should be created"), Object))
    {
        AddExpectedMessagePlain(TEXT("Object is not under /Game. The action does not support this scenario."),
                                ELogVerbosity::Error,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        Action->Apply(Fixture.ActionContext, Object);
        return TestTrue(TEXT("Objects outside /Game should only log and not add context errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

#endif
