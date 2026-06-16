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
    #include "RuleRanger/UI/RuleRangerEditorValidator.h"
    #include "RuleRangerActionContext.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorValidatorDefaultsToEnabledTest,
                                 "RuleRanger.UI.EditorValidator.DefaultsToEnabled",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorValidatorDefaultsToEnabledTest::RunTest(const FString&)
{
    const auto Validator = RuleRangerTests::NewTransientObject<URuleRangerEditorValidator>();
    if (!TestNotNull(TEXT("Editor validator should be created"), Validator))
    {
        return false;
    }

    return TestTrue(TEXT("RuleRanger editor validator should be enabled by default"), Validator->IsEnabled());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorValidatorOnRuleAppliedTranslatesErrorsToInvalidResultTest,
                                 "RuleRanger.UI.EditorValidator.OnRuleAppliedTranslatesErrorsToInvalidResult",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorValidatorOnRuleAppliedTranslatesErrorsToInvalidResultTest::RunTest(const FString&)
{
    const auto Validator = RuleRangerTests::NewTransientObject<URuleRangerEditorValidator>();
    RuleRangerTests::FRuleFixture Fixture;
    if (!TestNotNull(TEXT("Editor validator should be created"), Validator)
        || !RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        return false;
    }

    Fixture.ActionContext->Warning(FText::FromString(TEXT("Validator warning")));
    Fixture.ActionContext->Error(FText::FromString(TEXT("Validator error")));
    Fixture.ActionContext->Fatal(FText::FromString(TEXT("Validator fatal")));
    AddExpectedMessagePlain(TEXT("Validator warning"),
                            ELogVerbosity::Warning,
                            EAutomationExpectedMessageFlags::Contains,
                            1);
    AddExpectedMessagePlain(TEXT("Validator error"),
                            ELogVerbosity::Error,
                            EAutomationExpectedMessageFlags::Contains,
                            1);
    AddExpectedMessagePlain(TEXT("Validator fatal"),
                            ELogVerbosity::Error,
                            EAutomationExpectedMessageFlags::Contains,
                            1);
    Validator->OnRuleApplied(Fixture.ActionContext);

    return TestEqual(TEXT("Errors and fatals should make the asset invalid"),
                     Validator->GetValidationResult(),
                     EDataValidationResult::Invalid);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEditorValidatorCannotValidateAssetsWhenNoRulesApplyTest,
                                 "RuleRanger.UI.EditorValidator.CannotValidateAssetsWhenNoRulesApply",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEditorValidatorCannotValidateAssetsWhenNoRulesApplyTest::RunTest(const FString&)
{
    const auto Validator = RuleRangerTests::NewTransientObject<URuleRangerEditorValidator>();
    const auto Object =
        RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("ValidatorNoRulesObject"));
    if (!TestNotNull(TEXT("Editor validator should be created"), Validator)
        || !TestNotNull(TEXT("Validation object should be created"), Object))
    {
        return false;
    }

    RuleRangerTests::FScopedRuleRangerDeveloperSettingsOverride SettingsOverride({});
    auto Context = RuleRangerTests::CreateValidationContext();
    const bool bCanValidate = Validator->CanValidateAsset_Implementation(FAssetData(Object), Object, Context);

    return TestFalse(TEXT("Assets with no matching RuleRanger rules should not be validated"), bCanValidate);
}

#endif
