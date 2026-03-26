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
    #include "RuleRanger/Actions/Common/EnsureRequiredPropertiesPresentAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

namespace RuleRangerEnsureRequiredPropertiesPresentActionTests
{
    bool SetObjectReferenceProperty(FAutomationTestBase& Test,
                                    UObject* Object,
                                    const TCHAR* PropertyName,
                                    UObject* Value,
                                    const int32 ArrayIndex = 0)
    {
        return RuleRangerTests::SetPropertyValue(Test, Object, PropertyName, TObjectPtr<UObject>(Value), ArrayIndex);
    }

    bool SetRequiredComponentProperty(FAutomationTestBase& Test,
                                      UObject* Object,
                                      const TCHAR* PropertyName,
                                      URuleRangerAutomationRequiredComponentObject* Value)
    {
        return RuleRangerTests::SetPropertyValue(Test,
                                                 Object,
                                                 PropertyName,
                                                 TObjectPtr<URuleRangerAutomationRequiredComponentObject>(Value));
    }
} // namespace RuleRangerEnsureRequiredPropertiesPresentActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureRequiredPropertiesPresentActionDetectsMissingDirectReferenceTest,
    "RuleRanger.Actions.Common.EnsureRequiredPropertiesPresent.DetectsMissingDirectReference",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRequiredPropertiesPresentActionDetectsMissingDirectReferenceTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRequiredPropertiesPresentAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredObjectOwner>();
    const auto DefaultObject = RuleRangerTests::GetClassDefaultObject<URuleRangerAutomationRequiredObjectOwner>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRequiredPropertiesPresentAction should be created"), Action)
        && TestNotNull(TEXT("Object should be created"), Object)
        && TestNotNull(TEXT("Class default object should be available"), DefaultObject))
    {
        if (RuleRangerEnsureRequiredPropertiesPresentActionTests::SetObjectReferenceProperty(*this,
                                                                                             DefaultObject,
                                                                                             TEXT("RequiredObject"),
                                                                                             nullptr))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Missing direct required references should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Missing direct required references should identify the property"),
                       TEXT("Required Object"));
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
    FRuleRangerEnsureRequiredPropertiesPresentActionAllowsPresentDirectReferenceTest,
    "RuleRanger.Actions.Common.EnsureRequiredPropertiesPresent.AllowsPresentDirectReference",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRequiredPropertiesPresentActionAllowsPresentDirectReferenceTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRequiredPropertiesPresentAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredObjectOwner>();
    const auto DefaultObject = RuleRangerTests::GetClassDefaultObject<URuleRangerAutomationRequiredObjectOwner>();
    const auto Leaf =
        RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredLeafObject>(DefaultObject,
                                                                                     TEXT("PresentRequiredObject"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRequiredPropertiesPresentAction should be created"), Action)
        && TestNotNull(TEXT("Object should be created"), Object)
        && TestNotNull(TEXT("Class default object should be available"), DefaultObject)
        && TestNotNull(TEXT("Required leaf object should be created"), Leaf))
    {
        if (RuleRangerEnsureRequiredPropertiesPresentActionTests::SetObjectReferenceProperty(*this,
                                                                                             DefaultObject,
                                                                                             TEXT("RequiredObject"),
                                                                                             Leaf))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestTrue(TEXT("Present direct required references should not add errors"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty())
                && TestEqual(TEXT("The action context should remain successful"),
                             Fixture.ActionContext->GetState(),
                             ERuleRangerActionState::AS_Success);
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
    FRuleRangerEnsureRequiredPropertiesPresentActionDetectsMissingIndexedReferenceTest,
    "RuleRanger.Actions.Common.EnsureRequiredPropertiesPresent.DetectsMissingIndexedReference",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRequiredPropertiesPresentActionDetectsMissingIndexedReferenceTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRequiredPropertiesPresentAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredArrayOwner>();
    const auto DefaultObject = RuleRangerTests::GetClassDefaultObject<URuleRangerAutomationRequiredArrayOwner>();
    const auto PresentLeaf =
        RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredLeafObject>(DefaultObject,
                                                                                     TEXT("PresentIndexedObject"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRequiredPropertiesPresentAction should be created"), Action)
        && TestNotNull(TEXT("Object should be created"), Object)
        && TestNotNull(TEXT("Class default object should be available"), DefaultObject)
        && TestNotNull(TEXT("Indexed required leaf object should be created"), PresentLeaf))
    {
        if (RuleRangerEnsureRequiredPropertiesPresentActionTests::SetObjectReferenceProperty(*this,
                                                                                             DefaultObject,
                                                                                             TEXT("RequiredObjects"),
                                                                                             PresentLeaf,
                                                                                             0)
            && RuleRangerEnsureRequiredPropertiesPresentActionTests::SetObjectReferenceProperty(*this,
                                                                                                DefaultObject,
                                                                                                TEXT("RequiredObjects"),
                                                                                                nullptr,
                                                                                                1))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Missing indexed required references should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Indexed required references should identify the missing index"),
                       TEXT("index 1"));
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
    FRuleRangerEnsureRequiredPropertiesPresentActionLogsUnsupportedDirectPropertyTypesTest,
    "RuleRanger.Actions.Common.EnsureRequiredPropertiesPresent.LogsUnsupportedDirectPropertyTypes",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRequiredPropertiesPresentActionLogsUnsupportedDirectPropertyTypesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRequiredPropertiesPresentAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationUnsupportedRequiredOwner>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRequiredPropertiesPresentAction should be created"), Action)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        AddExpectedMessagePlain(TEXT("not supported property types other than references"),
                                ELogVerbosity::VeryVerbose,
                                EAutomationExpectedMessageFlags::Contains,
                                1);
        Action->Apply(Fixture.ActionContext, Object);

        return TestTrue(TEXT("Unsupported direct property types should not add context errors"),
                        Fixture.ActionContext->GetErrorMessages().IsEmpty());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureRequiredPropertiesPresentActionDetectsMissingComponentReferenceTest,
    "RuleRanger.Actions.Common.EnsureRequiredPropertiesPresent.DetectsMissingComponentReference",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRequiredPropertiesPresentActionDetectsMissingComponentReferenceTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRequiredPropertiesPresentAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredComponentOwner>();
    const auto DefaultObject = RuleRangerTests::GetClassDefaultObject<URuleRangerAutomationRequiredComponentOwner>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRequiredPropertiesPresentAction should be created"), Action)
        && TestNotNull(TEXT("Object should be created"), Object)
        && TestNotNull(TEXT("Class default object should be available"), DefaultObject))
    {
        if (RuleRangerEnsureRequiredPropertiesPresentActionTests::SetRequiredComponentProperty(
                *this,
                DefaultObject,
                TEXT("RequiredComponent"),
                nullptr))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Missing required component references should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Missing component references should mention the null sub-object"),
                       TEXT("sub-object property is null"));
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
    FRuleRangerEnsureRequiredPropertiesPresentActionDetectsMissingDeclaredComponentPropertyTest,
    "RuleRanger.Actions.Common.EnsureRequiredPropertiesPresent.DetectsMissingDeclaredComponentProperty",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRequiredPropertiesPresentActionDetectsMissingDeclaredComponentPropertyTest::RunTest(
    const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRequiredPropertiesPresentAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationMissingComponentOwner>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRequiredPropertiesPresentAction should be created"), Action)
        && TestNotNull(TEXT("Object should be created"), Object))
    {
        Action->Apply(Fixture.ActionContext, Object);

        return TestEqual(TEXT("Missing declared component properties should add one error"),
                         Fixture.ActionContext->GetErrorMessages().Num(),
                         1)
            && RuleRangerTests::TestTextArrayContains(
                   *this,
                   Fixture.ActionContext->GetErrorMessages(),
                   TEXT("Missing declared component properties should mention the missing sub-object"),
                   TEXT("no such sub-object property was defined"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureRequiredPropertiesPresentActionDetectsMissingNestedComponentPropertyTest,
    "RuleRanger.Actions.Common.EnsureRequiredPropertiesPresent.DetectsMissingNestedComponentProperty",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRequiredPropertiesPresentActionDetectsMissingNestedComponentPropertyTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRequiredPropertiesPresentAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationMissingComponentPropertyOwner>();
    const auto DefaultObject =
        RuleRangerTests::GetClassDefaultObject<URuleRangerAutomationMissingComponentPropertyOwner>();
    const auto Component = RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredComponentObject>(
        DefaultObject,
        TEXT("MissingPropertyComponent"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRequiredPropertiesPresentAction should be created"), Action)
        && TestNotNull(TEXT("Object should be created"), Object)
        && TestNotNull(TEXT("Class default object should be available"), DefaultObject)
        && TestNotNull(TEXT("Required component should be created"), Component))
    {
        if (RuleRangerEnsureRequiredPropertiesPresentActionTests::SetRequiredComponentProperty(
                *this,
                DefaultObject,
                TEXT("RequiredComponent"),
                Component))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Missing nested component properties should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Missing nested component properties should mention the missing property"),
                       TEXT("property is not present on the sub-object"));
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
    FRuleRangerEnsureRequiredPropertiesPresentActionDetectsInvalidNestedReferenceTypesTest,
    "RuleRanger.Actions.Common.EnsureRequiredPropertiesPresent.DetectsInvalidNestedReferenceTypes",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRequiredPropertiesPresentActionDetectsInvalidNestedReferenceTypesTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRequiredPropertiesPresentAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationInvalidComponentPropertyOwner>();
    const auto DefaultObject =
        RuleRangerTests::GetClassDefaultObject<URuleRangerAutomationInvalidComponentPropertyOwner>();
    const auto Component = RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredComponentObject>(
        DefaultObject,
        TEXT("InvalidPropertyComponent"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRequiredPropertiesPresentAction should be created"), Action)
        && TestNotNull(TEXT("Object should be created"), Object)
        && TestNotNull(TEXT("Class default object should be available"), DefaultObject)
        && TestNotNull(TEXT("Required component should be created"), Component))
    {
        if (RuleRangerEnsureRequiredPropertiesPresentActionTests::SetRequiredComponentProperty(
                *this,
                DefaultObject,
                TEXT("RequiredComponent"),
                Component))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Invalid nested reference types should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Invalid nested reference types should mention the invalid reference"),
                       TEXT("not a valid object reference"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureRequiredPropertiesPresentActionUsesInheritedClassMetadataTest,
                                 "RuleRanger.Actions.Common.EnsureRequiredPropertiesPresent.UsesInheritedClassMetadata",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRequiredPropertiesPresentActionUsesInheritedClassMetadataTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRequiredPropertiesPresentAction>();
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationInheritedRequiredDerivedOwner>();
    const auto DefaultObject =
        RuleRangerTests::GetClassDefaultObject<URuleRangerAutomationInheritedRequiredDerivedOwner>();
    const auto Component = RuleRangerTests::NewTransientObject<URuleRangerAutomationRequiredComponentObject>(
        DefaultObject,
        TEXT("InheritedRequiredComponent"));
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRequiredPropertiesPresentAction should be created"), Action)
        && TestNotNull(TEXT("Object should be created"), Object)
        && TestNotNull(TEXT("Class default object should be available"), DefaultObject)
        && TestNotNull(TEXT("Required component should be created"), Component))
    {
        if (RuleRangerEnsureRequiredPropertiesPresentActionTests::SetRequiredComponentProperty(
                *this,
                DefaultObject,
                TEXT("RequiredComponent"),
                Component))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Inherited required-property metadata should still add an error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Inherited required-property metadata should reference the required object"),
                       TEXT("RequiredObject"));
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
    FRuleRangerEnsureRequiredPropertiesPresentActionSkipsAbstractTypesByDefaultTest,
    "RuleRanger.Actions.Common.EnsureRequiredPropertiesPresent.SkipsAbstractTypesByDefault",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRequiredPropertiesPresentActionSkipsAbstractTypesByDefaultTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRequiredPropertiesPresentAction>();
    const auto Object = RuleRangerTests::GetClassDefaultObject<URuleRangerAutomationAbstractRequiredOwner>();
    const auto DefaultObject = RuleRangerTests::GetClassDefaultObject<URuleRangerAutomationAbstractRequiredOwner>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRequiredPropertiesPresentAction should be created"), Action)
        && TestNotNull(TEXT("Abstract class default object should be available"), Object)
        && TestNotNull(TEXT("Abstract class default object should be available"), DefaultObject))
    {
        if (RuleRangerEnsureRequiredPropertiesPresentActionTests::SetRequiredComponentProperty(
                *this,
                DefaultObject,
                TEXT("RequiredComponent"),
                nullptr))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestTrue(TEXT("Abstract types should be skipped by default"),
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
    FRuleRangerEnsureRequiredPropertiesPresentActionCanValidateAbstractTypesWhenConfiguredTest,
    "RuleRanger.Actions.Common.EnsureRequiredPropertiesPresent.CanValidateAbstractTypesWhenConfigured",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureRequiredPropertiesPresentActionCanValidateAbstractTypesWhenConfiguredTest::RunTest(const FString&)
{
    RuleRangerTests::FRuleFixture Fixture;
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureRequiredPropertiesPresentAction>();
    const auto Object = RuleRangerTests::GetClassDefaultObject<URuleRangerAutomationAbstractRequiredOwner>();
    const auto DefaultObject = RuleRangerTests::GetClassDefaultObject<URuleRangerAutomationAbstractRequiredOwner>();
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture)
        && TestNotNull(TEXT("EnsureRequiredPropertiesPresentAction should be created"), Action)
        && TestNotNull(TEXT("Abstract class default object should be available"), Object)
        && TestNotNull(TEXT("Abstract class default object should be available"), DefaultObject))
    {
        if (RuleRangerTests::SetPropertyValue(*this, Action, TEXT("bSkipCheckOnAbstractTypes"), false)
            && RuleRangerEnsureRequiredPropertiesPresentActionTests::SetRequiredComponentProperty(
                *this,
                DefaultObject,
                TEXT("RequiredComponent"),
                nullptr))
        {
            Action->Apply(Fixture.ActionContext, Object);

            return TestEqual(TEXT("Configured abstract-type validation should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(
                       *this,
                       Fixture.ActionContext->GetErrorMessages(),
                       TEXT("Configured abstract-type validation should still evaluate required properties"),
                       TEXT("sub-object property is null"));
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
