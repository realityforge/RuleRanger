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
    #include "RuleRanger/Matchers/Common/ObjectTypeMatcher.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerObjectTypeMatcherTests
{
    bool SetObjectType(FAutomationTestBase& Test, UObjectTypeMatcher* Matcher, const TSubclassOf<UObject> ObjectType)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("ObjectType"), ObjectType);
    }
} // namespace RuleRangerObjectTypeMatcherTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerObjectTypeMatcherTraversesHierarchyByDefaultTest,
                                 "RuleRanger.Matchers.Common.ObjectType.TraversesHierarchyByDefault",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerObjectTypeMatcherTraversesHierarchyByDefaultTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UObjectTypeMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationDerivedTestObject>();
    if (TestNotNull(TEXT("ObjectType matcher should be created"), Matcher)
        && TestNotNull(TEXT("Derived test object should be created"), Object))
    {
        if (RuleRangerObjectTypeMatcherTests::SetObjectType(
                *this,
                Matcher,
                TSubclassOf<UObject>(URuleRangerAutomationTestObject::StaticClass())))
        {
            return TestTrue(TEXT("Derived objects should match base types when hierarchy traversal is enabled"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerObjectTypeMatcherRetainsNativeInheritanceWhenTraversalDisabledTest,
                                 "RuleRanger.Matchers.Common.ObjectType.RetainsNativeInheritanceWhenTraversalDisabled",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerObjectTypeMatcherRetainsNativeInheritanceWhenTraversalDisabledTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UObjectTypeMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationDerivedTestObject>();
    if (TestNotNull(TEXT("ObjectType matcher should be created"), Matcher)
        && TestNotNull(TEXT("Derived test object should be created"), Object))
    {
        if (RuleRangerObjectTypeMatcherTests::SetObjectType(
                *this,
                Matcher,
                TSubclassOf<UObject>(URuleRangerAutomationTestObject::StaticClass()))
            && RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("bTraverseAllTypeHierarchies"), false))
        {
            return TestTrue(TEXT("Disabling alternative hierarchy traversal should still preserve native IsA matching"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerObjectTypeMatcherRejectsUnrelatedObjectsTest,
                                 "RuleRanger.Matchers.Common.ObjectType.RejectsUnrelatedObjects",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerObjectTypeMatcherRejectsUnrelatedObjectsTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UObjectTypeMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationDerivedTestObject>();
    if (TestNotNull(TEXT("ObjectType matcher should be created"), Matcher)
        && TestNotNull(TEXT("Derived test object should be created"), Object))
    {
        if (RuleRangerObjectTypeMatcherTests::SetObjectType(
                *this,
                Matcher,
                TSubclassOf<UObject>(URuleRangerAutomationImportDataObject::StaticClass()))
            && RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("bTraverseAllTypeHierarchies"), false))
        {
            return TestFalse(TEXT("Unrelated objects should not match when exact native IsA is false"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerObjectTypeMatcherExactTypeAcceptsMatchingObjectsTest,
                                 "RuleRanger.Matchers.Common.ObjectType.ExactTypeAcceptsMatchingObjects",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerObjectTypeMatcherExactTypeAcceptsMatchingObjectsTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<UObjectTypeMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationDerivedTestObject>();
    if (TestNotNull(TEXT("ObjectType matcher should be created"), Matcher)
        && TestNotNull(TEXT("Derived test object should be created"), Object))
    {
        if (RuleRangerObjectTypeMatcherTests::SetObjectType(
                *this,
                Matcher,
                TSubclassOf<UObject>(URuleRangerAutomationDerivedTestObject::StaticClass()))
            && RuleRangerTests::SetPropertyValue(*this, Matcher, TEXT("bTraverseAllTypeHierarchies"), false))
        {
            return TestTrue(TEXT("Exact type matching should accept objects of the configured type"),
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
