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
    #include "RuleRanger/RuleRangerUtilities.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerUtilitiesCollectTypeHierarchyIncludesNativeTypesTest,
                                 "RuleRanger.Utilities.CollectTypeHierarchy.IncludesNativeTypes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerUtilitiesCollectTypeHierarchyIncludesNativeTypesTest::RunTest(const FString&)
{
    const auto Object = RuleRangerTests::NewTransientObject<URuleRangerAutomationDerivedTestObject>();
    if (TestNotNull(TEXT("Derived test object should be created"), Object))
    {
        TArray<UClass*> Classes;
        FRuleRangerUtilities::CollectTypeHierarchy(Object, Classes);

        return TestTrue(TEXT("The most specific class should be present"), Classes.Contains(Object->GetClass()))
            && TestTrue(TEXT("The parent class should be present"),
                        Classes.Contains(URuleRangerAutomationTestObject::StaticClass()))
            && TestTrue(TEXT("UObject should be present"), Classes.Contains(UObject::StaticClass()));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerUtilitiesBlueprintHelpersRespectParentTypesTest,
                                 "RuleRanger.Utilities.BlueprintHelpers.RespectParentTypes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerUtilitiesBlueprintHelpersRespectParentTypesTest::RunTest(const FString&)
{
    const auto Blueprint = RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                                         TEXT("/Game/Developers/RuleRangerTests/Utilities/Blueprint"),
                                                         TEXT("UtilityBlueprint"));
    if (TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        RuleRangerTests::CompileBlueprint(Blueprint);

        TArray<UClass*> Classes;
        FRuleRangerUtilities::CollectTypeHierarchy(Blueprint, Classes);
        const auto* DefaultObject =
            FRuleRangerUtilities::ToObject<URuleRangerAutomationBlueprintParentObject>(Blueprint);

        return TestTrue(TEXT("Blueprints should be treated as instances of their parent class"),
                        FRuleRangerUtilities::IsA(Blueprint, URuleRangerAutomationBlueprintParentObject::StaticClass()))
            && TestNotNull(TEXT("ToObject should return a class default object for matching blueprints"), DefaultObject)
            && TestTrue(TEXT("The parent class should be included in the collected type hierarchy"),
                        Classes.Contains(URuleRangerAutomationBlueprintParentObject::StaticClass()));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerUtilitiesIsAbstractHandlesClassesAndBlueprintsTest,
                                 "RuleRanger.Utilities.IsAbstract.HandlesClassesAndBlueprints",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerUtilitiesIsAbstractHandlesClassesAndBlueprintsTest::RunTest(const FString&)
{
    const auto Blueprint =
        RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                      TEXT("/Game/Developers/RuleRangerTests/Utilities/AbstractBlueprint"),
                                      TEXT("AbstractUtilityBlueprint"));
    if (TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        Blueprint->bGenerateAbstractClass = true;
        RuleRangerTests::CompileBlueprint(Blueprint);

        return TestTrue(TEXT("Abstract native classes should report as abstract"),
                        FRuleRangerUtilities::IsAbstract(URuleRangerAutomationAbstractTestObject::StaticClass()))
            && TestTrue(TEXT("Blueprints flagged as abstract should report as abstract"),
                        FRuleRangerUtilities::IsAbstract(Blueprint))
            && TestFalse(TEXT("Concrete objects should not report as abstract"),
                         FRuleRangerUtilities::IsAbstract(
                             RuleRangerTests::NewTransientObject<URuleRangerAutomationTestObject>()));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerUtilitiesCollectInstanceHierarchyTraversesMaterialParentsTest,
                                 "RuleRanger.Utilities.CollectInstanceHierarchy.TraversesMaterialParents",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerUtilitiesCollectInstanceHierarchyTraversesMaterialParentsTest::RunTest(const FString&)
{
    const auto Material = RuleRangerTests::NewTransientObject<UMaterial>();
    const auto ParentInstance = RuleRangerTests::NewTransientObject<UMaterialInstanceConstant>();
    const auto ChildInstance = RuleRangerTests::NewTransientObject<UMaterialInstanceConstant>();
    if (TestNotNull(TEXT("Material should be created"), Material)
        && TestNotNull(TEXT("Parent material instance should be created"), ParentInstance)
        && TestNotNull(TEXT("Child material instance should be created"), ChildInstance))
    {
        if (RuleRangerTests::SetPropertyValue(*this,
                                              ParentInstance,
                                              TEXT("Parent"),
                                              static_cast<UMaterialInterface*>(Material))
            && RuleRangerTests::SetPropertyValue(*this,
                                                 ChildInstance,
                                                 TEXT("Parent"),
                                                 static_cast<UMaterialInterface*>(ParentInstance)))
        {
            TArray<UObject*> Instances;
            FRuleRangerUtilities::CollectInstanceHierarchy(ChildInstance, Instances);

            return TestEqual(TEXT("The child material instance should be first"),
                             Instances[0],
                             static_cast<UObject*>(ChildInstance))
                && TestEqual(TEXT("The parent material instance should be second"),
                             Instances[1],
                             static_cast<UObject*>(ParentInstance))
                && TestEqual(TEXT("The base material should be last"), Instances[2], static_cast<UObject*>(Material));
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
