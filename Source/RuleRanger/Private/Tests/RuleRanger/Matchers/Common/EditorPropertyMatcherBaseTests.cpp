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
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerEditorPropertyMatcherBaseTests
{
    bool SetName(FAutomationTestBase& Test, UObject* Matcher, const TCHAR* const Name)
    {
        return RuleRangerTests::SetPropertyValue(Test, Matcher, TEXT("Name"), FName(Name));
    }
} // namespace RuleRangerEditorPropertyMatcherBaseTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyMatcherBaseReturnsFalseWithoutConfiguredNameTest,
                                 "RuleRanger.Matchers.Common.EditorPropertyBase.ReturnsFalseWithoutConfiguredName",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherBaseReturnsFalseWithoutConfiguredNameTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyProbeMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    if (TestNotNull(TEXT("Editor-property probe matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object))
    {
        Matcher->ResetObservations();
        return TestFalse(TEXT("Matchers without a configured property name should fail"), Matcher->Test(Object))
            && TestEqual(TEXT("TestEditorProperty should not be called when the name is unset"),
                         Matcher->GetTestEditorPropertyCallCount(),
                         0);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyMatcherBaseInvokesOverrideForMatchingPropertiesTest,
                                 "RuleRanger.Matchers.Common.EditorPropertyBase.InvokesOverrideForMatchingProperties",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherBaseInvokesOverrideForMatchingPropertiesTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyProbeMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationDerivedEditorPropertyObject>();
    if (TestNotNull(TEXT("Editor-property probe matcher should be created"), Matcher)
        && TestNotNull(TEXT("Derived editor-property object should be created"), Object))
    {
        Matcher->ResetObservations();
        if (RuleRangerEditorPropertyMatcherBaseTests::SetName(*this, Matcher, TEXT("EditorInteger")))
        {
            return TestTrue(TEXT("Matching properties should invoke the derived TestEditorProperty implementation"),
                            Matcher->Test(Object))
                && TestEqual(TEXT("TestEditorProperty should be called once for the first matching property"),
                             Matcher->GetTestEditorPropertyCallCount(),
                             1)
                && TestEqual(TEXT("The original object should be forwarded to TestEditorProperty"),
                             Matcher->GetLastMatchedObject(),
                             static_cast<const UObject*>(Object))
                && TestEqual(TEXT("The matching instance should be forwarded to TestEditorProperty"),
                             Matcher->GetLastMatchedInstance(),
                             static_cast<const UObject*>(Object))
                && TestEqual(TEXT("The matching property name should be captured"),
                             Matcher->GetLastMatchedPropertyName(),
                             FName(TEXT("EditorInteger")));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorPropertyMatcherBaseLogsAndFailsWhenOverrideCallsSuperTest,
                                 "RuleRanger.Matchers.Common.EditorPropertyBase.LogsAndFailsWhenOverrideCallsSuper",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorPropertyMatcherBaseLogsAndFailsWhenOverrideCallsSuperTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyFallbackMatcher>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationEditorPropertyObject>();
    if (TestNotNull(TEXT("Editor-property fallback matcher should be created"), Matcher)
        && TestNotNull(TEXT("Editor-property object should be created"), Object))
    {
        if (RuleRangerEditorPropertyMatcherBaseTests::SetName(*this, Matcher, TEXT("bEditorBool")))
        {
            AddExpectedMessagePlain(TEXT("Matcher failed to override TestEditorProperty."),
                                    ELogVerbosity::Error,
                                    EAutomationExpectedMessageFlags::Contains,
                                    1);

            return TestFalse(TEXT("Calling the base TestEditorProperty implementation should fail"),
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
