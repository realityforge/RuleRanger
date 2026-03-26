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
    #include "RuleRanger/Matchers/Common/EditorPropertyPresentMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerEditorPropertyPresentMatcherTests
{
    bool SetName(FAutomationTestBase& Test, UEditorPropertyPresentMatcher* Matcher, const TCHAR* const Name)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Name"), FName(Name));
    }

    bool SetObjectReference(FAutomationTestBase& Test,
                            UObject* Object,
                            const TCHAR* const PropertyName,
                            UObject* const Value,
                            const int32 ArrayIndex = 0)
    {
        return RuleRangerTests::SetPropertyValue(Test, Object, PropertyName, TObjectPtr<UObject>(Value), ArrayIndex);
    }
} // namespace RuleRangerEditorPropertyPresentMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyPresentMatcherReturnsFalseForMissingPropertiesTest,
                                 "RuleRanger.Matchers.Common.EditorPropertyPresent.ReturnsFalseForMissingProperties",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyPresentMatcherReturnsFalseForMissingPropertiesTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyPresentMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    if (TestNotNull(TEXT("EditorPropertyPresent matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object))
    {
        if (RuleRangerEditorPropertyPresentMatcherTests::SetName(*this, Matcher, TEXT("MissingReference")))
        {
            return TestFalse(TEXT("Missing properties should not be considered present"), Matcher->Test(Object));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyPresentMatcherMatchesScalarObjectReferencesTest,
                                 "RuleRanger.Matchers.Common.EditorPropertyPresent.MatchesScalarObjectReferences",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyPresentMatcherMatchesScalarObjectReferencesTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyPresentMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    const auto Leaf = RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredLeafObject>(Object);
    if (TestNotNull(TEXT("EditorPropertyPresent matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object)
        && TestNotNull(TEXT("Leaf reference object should be created"), Leaf))
    {
        if (RuleRangerEditorPropertyPresentMatcherTests::SetObjectReference(*this,
                                                                            Object,
                                                                            TEXT("EditorReference"),
                                                                            Leaf)
            && RuleRangerEditorPropertyPresentMatcherTests::SetName(*this, Matcher, TEXT("EditorReference")))
        {
            return TestTrue(TEXT("Present scalar object references should match"), Matcher->Test(Object));
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
    FRuleRangerEditorPropertyPresentMatcherRejectsMissingScalarObjectReferencesTest,
    "RuleRanger.Matchers.Common.EditorPropertyPresent.RejectsMissingScalarObjectReferences",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyPresentMatcherRejectsMissingScalarObjectReferencesTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyPresentMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    if (TestNotNull(TEXT("EditorPropertyPresent matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object))
    {
        if (RuleRangerEditorPropertyPresentMatcherTests::SetName(*this, Matcher, TEXT("EditorReference")))
        {
            return TestFalse(TEXT("Null scalar object references should not match"), Matcher->Test(Object));
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
    FRuleRangerEditorPropertyPresentMatcherMatchesFullyPopulatedReferenceArraysTest,
    "RuleRanger.Matchers.Common.EditorPropertyPresent.MatchesFullyPopulatedReferenceArrays",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyPresentMatcherMatchesFullyPopulatedReferenceArraysTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyPresentMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    const auto FirstLeaf = RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredLeafObject>(Object);
    const auto SecondLeaf = RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredLeafObject>(Object);
    if (TestNotNull(TEXT("EditorPropertyPresent matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object)
        && TestNotNull(TEXT("First leaf reference object should be created"), FirstLeaf)
        && TestNotNull(TEXT("Second leaf reference object should be created"), SecondLeaf))
    {
        if (RuleRangerEditorPropertyPresentMatcherTests::SetObjectReference(*this,
                                                                            Object,
                                                                            TEXT("EditorReferenceArray"),
                                                                            FirstLeaf,
                                                                            0)
            && RuleRangerEditorPropertyPresentMatcherTests::SetObjectReference(*this,
                                                                               Object,
                                                                               TEXT("EditorReferenceArray"),
                                                                               SecondLeaf,
                                                                               1)
            && RuleRangerEditorPropertyPresentMatcherTests::SetName(*this, Matcher, TEXT("EditorReferenceArray")))
        {
            return TestTrue(TEXT("Fixed arrays should only match when every object reference is present"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEditorPropertyPresentMatcherRejectsPartiallyPopulatedReferenceArraysTest,
    "RuleRanger.Matchers.Common.EditorPropertyPresent.RejectsPartiallyPopulatedReferenceArrays",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyPresentMatcherRejectsPartiallyPopulatedReferenceArraysTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyPresentMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    const auto Leaf = RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredLeafObject>(Object);
    if (TestNotNull(TEXT("EditorPropertyPresent matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object)
        && TestNotNull(TEXT("Leaf reference object should be created"), Leaf))
    {
        if (RuleRangerEditorPropertyPresentMatcherTests::SetObjectReference(*this,
                                                                            Object,
                                                                            TEXT("EditorReferenceArray"),
                                                                            Leaf,
                                                                            0)
            && RuleRangerEditorPropertyPresentMatcherTests::SetObjectReference(*this,
                                                                               Object,
                                                                               TEXT("EditorReferenceArray"),
                                                                               nullptr,
                                                                               1)
            && RuleRangerEditorPropertyPresentMatcherTests::SetName(*this, Matcher, TEXT("EditorReferenceArray")))
        {
            return TestFalse(TEXT("Any null entry in a fixed array should make the present matcher fail"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyPresentMatcherRejectsUnsupportedPropertyTypesTest,
                                 "RuleRanger.Matchers.Common.EditorPropertyPresent.RejectsUnsupportedPropertyTypes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyPresentMatcherRejectsUnsupportedPropertyTypesTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyPresentMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    if (TestNotNull(TEXT("EditorPropertyPresent matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object))
    {
        if (RuleRangerEditorPropertyPresentMatcherTests::SetName(*this, Matcher, TEXT("EditorInteger")))
        {
            return TestFalse(TEXT("Non-object properties should not be considered present references"),
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
