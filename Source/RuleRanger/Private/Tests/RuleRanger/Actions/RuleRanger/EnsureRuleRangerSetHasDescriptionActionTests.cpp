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
    #include "RuleRanger/Actions/RuleRanger/EnsureRuleRangerSetHasDescriptionAction.h"
    #include "RuleRangerRuleSet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureRuleSetHasDescriptionActionDetectsBlankDescriptionsTest,
                                 "RuleRanger.Actions.RuleRanger.RuleSetHasDescription.DetectsBlankDescriptions",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRuleSetHasDescriptionActionDetectsBlankDescriptionsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRuleRangerSetHasDescriptionAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRuleRangerSetHasDescriptionAction should be created"), Action)
        && TestNotNull(TEXT("RuleRangerRuleSet should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Object, TEXT("Description"), FText::FromString(TEXT("  "))))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Description validation should target RuleRangerRuleSet"),
                             Action->GetExpectedType(),
                             URuleRangerRuleSet::StaticClass())
                && TestEqual(TEXT("Blank descriptions should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("Blank description errors should mention the RuleSet"),
                                                          TEXT("blank description"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureRuleSetHasDescriptionActionAcceptsNonBlankDescriptionsTest,
                                 "RuleRanger.Actions.RuleRanger.RuleSetHasDescription.AcceptsNonBlankDescriptions",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRuleSetHasDescriptionActionAcceptsNonBlankDescriptionsTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRuleRangerSetHasDescriptionAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRuleRangerSetHasDescriptionAction should be created"), Action)
        && TestNotNull(TEXT("RuleRangerRuleSet should be created"), Object))
    {
        if (RuleRangerTests::SetPropertyValue(*this,
                                              Object,
                                              TEXT("Description"),
                                              FText::FromString(TEXT("Useful RuleSet description"))))
        {
            Action->Apply(Fixture.ActionContext, Object);
            return TestTrue(TEXT("Non-blank descriptions should not add context errors"),
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

#endif
