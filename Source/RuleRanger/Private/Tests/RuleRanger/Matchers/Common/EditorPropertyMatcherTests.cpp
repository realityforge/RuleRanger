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

    #include "Materials/Material.h"
    #include "Materials/MaterialInstanceConstant.h"
    #include "Misc/AutomationTest.h"
    #include "RuleRanger/Matchers/Common/EditorPropertyMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerEditorPropertyMatcherTests
{
    bool SetName(FAutomationTestBase& Test, UObject* Matcher, const TCHAR* const Name)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Name"), FName(Name));
    }

    bool SetValue(FAutomationTestBase& Test, UEditorPropertyMatcher* Matcher, const TCHAR* const Value)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Value"), FString(Value));
    }

    bool
    SetMaterialParent(FAutomationTestBase& Test, UMaterialInstanceConstant* Object, UMaterialInterface* const Parent)
    {
        return RuleRangerTests::SetPropertyValue(Test, Object, TEXT("Parent"), TObjectPtr<UMaterialInterface>(Parent));
    }
} // namespace RuleRangerEditorPropertyMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyMatcherMatchesBoolValuesIgnoringCaseTest,
                                 "RuleRanger.Matchers.Common.EditorProperty.MatchesBoolValuesIgnoringCase",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherMatchesBoolValuesIgnoringCaseTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    if (TestNotNull(TEXT("EditorProperty matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("bEditorBool"), true)
            && RuleRangerEditorPropertyMatcherTests::SetName(*this, Matcher, TEXT("bEditorBool"))
            && RuleRangerEditorPropertyMatcherTests::SetValue(*this, Matcher, TEXT("TRUE")))
        {
            return TestTrue(TEXT("Bool matching should ignore case when parsing the expected value"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyMatcherMatchesIntegersTest,
                                 "RuleRanger.Matchers.Common.EditorProperty.MatchesIntegers",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherMatchesIntegersTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    if (TestNotNull(TEXT("EditorProperty matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("EditorInteger"), 42)
            && RuleRangerEditorPropertyMatcherTests::SetName(*this, Matcher, TEXT("EditorInteger"))
            && RuleRangerEditorPropertyMatcherTests::SetValue(*this, Matcher, TEXT("42")))
        {
            return TestTrue(TEXT("Integer editor-property values should match parsed integer strings"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyMatcherMatchesFloatingPointValuesTest,
                                 "RuleRanger.Matchers.Common.EditorProperty.MatchesFloatingPointValues",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherMatchesFloatingPointValuesTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    if (TestNotNull(TEXT("EditorProperty matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("EditorNumber"), 3.5)
            && RuleRangerEditorPropertyMatcherTests::SetName(*this, Matcher, TEXT("EditorNumber"))
            && RuleRangerEditorPropertyMatcherTests::SetValue(*this, Matcher, TEXT("3.5")))
        {
            return TestTrue(TEXT("Floating-point editor-property values should match parsed decimal strings"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyMatcherMatchesEnumNameAuthoredNameAndDisplayNameTest,
                                 "RuleRanger.Matchers.Common.EditorProperty.MatchesEnumNameAuthoredNameAndDisplayName",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherMatchesEnumNameAuthoredNameAndDisplayNameTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    const auto Enum = StaticEnum<ERuleRangerAutomationDisplayEnum>();
    if (TestNotNull(TEXT("EditorProperty matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object)
        && TestNotNull(TEXT("Display enum should be available"), Enum))
    {
        if (RuleRangerTests::SetPropertyValue(*this,
                                              Object,
                                              TEXT("EditorEnum"),
                                              ERuleRangerAutomationDisplayEnum::Enabled)
            && RuleRangerEditorPropertyMatcherTests::SetName(*this, Matcher, TEXT("EditorEnum")))
        {
            const auto bEnumNameMatch =
                RuleRangerEditorPropertyMatcherTests::SetValue(
                    *this,
                    Matcher,
                    *Enum->GetNameByValue(static_cast<int64>(ERuleRangerAutomationDisplayEnum::Enabled)).ToString())
                && TestTrue(TEXT("Enum matching should accept the raw enum name"), Matcher->Test(Object));
            const auto bAuthoredNameMatch =
                RuleRangerEditorPropertyMatcherTests::SetValue(*this, Matcher, TEXT("Enabled"))
                && TestTrue(TEXT("Enum matching should accept the authored enum name"), Matcher->Test(Object));
            const auto bDisplayNameMatch =
                RuleRangerEditorPropertyMatcherTests::SetValue(*this, Matcher, TEXT("Display Enabled"))
                && TestTrue(TEXT("Enum matching should accept the display name"), Matcher->Test(Object));
            return bEnumNameMatch && bAuthoredNameMatch && bDisplayNameMatch;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyMatcherMatchesLegacyNumericEnumsTest,
                                 "RuleRanger.Matchers.Common.EditorProperty.MatchesLegacyNumericEnums",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherMatchesLegacyNumericEnumsTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    if (TestNotNull(TEXT("EditorProperty matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this,
                                              Object,
                                              TEXT("EditorLegacyEnum"),
                                              TEnumAsByte<ERuleRangerAutomationLegacyEnum>(LegacyEnabled))
            && RuleRangerEditorPropertyMatcherTests::SetName(*this, Matcher, TEXT("EditorLegacyEnum"))
            && RuleRangerEditorPropertyMatcherTests::SetValue(*this, Matcher, TEXT("Legacy Enabled")))
        {
            return TestTrue(TEXT("Legacy numeric enum matching should use the enum metadata names"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyMatcherMatchesInheritedPropertiesOnDerivedTypesTest,
                                 "RuleRanger.Matchers.Common.EditorProperty.MatchesInheritedPropertiesOnDerivedTypes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherMatchesInheritedPropertiesOnDerivedTypesTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationDerivedEditorPropertyObject>();
    if (TestNotNull(TEXT("EditorProperty matcher should be created"), Matcher)
        && TestNotNull(TEXT("Derived editor-property object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("EditorInteger"), 7)
            && RuleRangerEditorPropertyMatcherTests::SetName(*this, Matcher, TEXT("EditorInteger"))
            && RuleRangerEditorPropertyMatcherTests::SetValue(*this, Matcher, TEXT("7")))
        {
            return TestTrue(TEXT("Editor-property lookup should traverse the reflected type hierarchy"),
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
    FRuleRangerEditorPropertyMatcherTraversesParentMaterialHierarchyWhenEnabledTest,
    "RuleRanger.Matchers.Common.EditorProperty.TraversesParentMaterialHierarchyWhenEnabled",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherTraversesParentMaterialHierarchyWhenEnabledTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyMatcher>();
    const auto Parent = RuleRangerTests::NewTransientObject<UMaterial>();
    const auto Object = RuleRangerTests::NewTransientObject<UMaterialInstanceConstant>();
    if (TestNotNull(TEXT("EditorProperty matcher should be created"), Matcher)
        && TestNotNull(TEXT("Parent material should be created"), Parent)
        && TestNotNull(TEXT("Material instance should be created"), Object))
    {
        if (RuleRangerTests::SetBoolPropertyValue(*this, Parent, TEXT("bUsedWithSkeletalMesh"), true)
            && RuleRangerEditorPropertyMatcherTests::SetMaterialParent(*this, Object, Parent)
            && RuleRangerEditorPropertyMatcherTests::SetName(*this, Matcher, TEXT("bUsedWithSkeletalMesh"))
            && RuleRangerEditorPropertyMatcherTests::SetValue(*this, Matcher, TEXT("true")))
        {
            return TestTrue(TEXT("Editor-property lookup should traverse the parent material hierarchy by default"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyMatcherSkipsParentMaterialHierarchyWhenDisabledTest,
                                 "RuleRanger.Matchers.Common.EditorProperty.SkipsParentMaterialHierarchyWhenDisabled",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherSkipsParentMaterialHierarchyWhenDisabledTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyMatcher>();
    const auto Parent = RuleRangerTests::NewTransientObject<UMaterial>();
    const auto Object = RuleRangerTests::NewTransientObject<UMaterialInstanceConstant>();
    if (TestNotNull(TEXT("EditorProperty matcher should be created"), Matcher)
        && TestNotNull(TEXT("Parent material should be created"), Parent)
        && TestNotNull(TEXT("Material instance should be created"), Object))
    {
        if (RuleRangerTests::SetBoolPropertyValue(*this, Parent, TEXT("bUsedWithSkeletalMesh"), true)
            && RuleRangerEditorPropertyMatcherTests::SetMaterialParent(*this, Object, Parent)
            && RuleRangerEditorPropertyMatcherTests::SetName(*this, Matcher, TEXT("bUsedWithSkeletalMesh"))
            && RuleRangerEditorPropertyMatcherTests::SetValue(*this, Matcher, TEXT("true"))
            && RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("bTraverseInstanceHierarchy"), false))
        {
            return TestFalse(TEXT("Disabling parent traversal should limit lookup to the current object"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyMatcherRejectsUnsupportedPropertyTypesTest,
                                 "RuleRanger.Matchers.Common.EditorProperty.RejectsUnsupportedPropertyTypes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherRejectsUnsupportedPropertyTypesTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UEditorPropertyMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    if (TestNotNull(TEXT("EditorProperty matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("EditorText"), FString(TEXT("Hello")))
            && RuleRangerEditorPropertyMatcherTests::SetName(*this, Matcher, TEXT("EditorText"))
            && RuleRangerEditorPropertyMatcherTests::SetValue(*this, Matcher, TEXT("Hello")))
        {
            return TestFalse(TEXT("Unsupported editor-property types should not be considered matches"),
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
