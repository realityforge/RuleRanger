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
#include "EnsureRequiredPropertiesPresentAction.h"
#include "RuleRanger/RuleRangerUtilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureRequiredPropertiesPresentAction)

static const FName RequiredPropertyName("RuleRangerRequired");

void UEnsureRequiredPropertiesPresentAction::PerformChecksOnObject(URuleRangerActionContext* ActionContext,
                                                                   const UObject* Object) const
{
    for (TFieldIterator<FProperty> PropertyIt(Object->GetClass()); PropertyIt; ++PropertyIt)
    {
        const FProperty* Property = *PropertyIt;
        if (PropertyIt->GetMetaDataMap() && PropertyIt->GetMetaDataMap()->Contains(RequiredPropertyName))
        {
            if (const auto ObjectProperty = CastField<FObjectPropertyBase>(Property))
            {
                if (1 == ObjectProperty->ArrayDim)
                {
                    if (!ObjectProperty->LoadObjectPropertyValue_InContainer(Object))
                    {
                        ActionContext->Error(
                            FText::Format(NSLOCTEXT("RuleRanger",
                                                    "ReqPropMissingSingle",
                                                    "Object contains property named '{0}' that is not set but "
                                                    "the property is annotated with the {1} meta property "
                                                    "which indicates it MUST be set."),
                                          Property->GetDisplayNameText(),
                                          FText::FromName(RequiredPropertyName)));
                    }
                }
                else
                {
                    for (int32 i = 0; i < ObjectProperty->ArrayDim; i++)
                    {
                        if (!ObjectProperty->LoadObjectPropertyValue_InContainer(Object, i))
                        {
                            ActionContext->Error(
                                FText::Format(NSLOCTEXT("RuleRanger",
                                                        "ReqPropMissingArray",
                                                        "Object contains property named '{0}' that has "
                                                        "index {1} that is not set but the property is "
                                                        "annotated with the {2} meta property which indicates "
                                                        "it MUST be set."),
                                              Property->GetDisplayNameText(),
                                              FText::AsNumber(i),
                                              FText::FromName(RequiredPropertyName)));
                        }
                    }
                }
            }
            else
            {
                LogError(Object,
                         FString::Printf(TEXT("Property named %s is annotated with the meta property %s "
                                              "but this is not supported property types other than references."),
                                         *Property->GetDisplayNameText().ToString(),
                                         *RequiredPropertyName.ToString()));
            }
        }
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void UEnsureRequiredPropertiesPresentAction::DeriveRequiredComponentProperties(
    const UClass* Class,
    TMap<FString, TArray<FString>>& RequiredComponentProperties)
{
    for (const UStruct* TestStruct = Class; TestStruct; TestStruct = TestStruct->GetSuperStruct())
    {
        if (const FString* Value = TestStruct->FindMetaData(RequiredPropertyName))
        {
            TArray<FString> Rules;
            Value->ParseIntoArray(Rules, TEXT(","));
            for (const auto& Rule : Rules)
            {
                FString ComponentName;
                FString PropertyName;
                if (Rule.Split(TEXT("."), &ComponentName, &PropertyName))
                {
                    // Only add require rule for component properties
                    if (RequiredComponentProperties.Contains(ComponentName))
                    {
                        RequiredComponentProperties.FindChecked(ComponentName).Add(PropertyName);
                    }
                    else
                    {
                        TArray<FString> Values;
                        Values.Add(PropertyName);
                        RequiredComponentProperties.Add(ComponentName, Values);
                    }
                }
            }
        }
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void UEnsureRequiredPropertiesPresentAction::PerformChecksForComponentProperties(
    URuleRangerActionContext* ActionContext,
    const UClass* Class,
    const FString& ComponentName,
    const UObject* Component,
    const TArray<FString>& PropertyNames)
{
    for (const auto& PropertyName : PropertyNames)
    {
        if (const FProperty* Property = Component->GetClass()->FindPropertyByName(FName(PropertyName)))
        {
            if (const auto ObjectProperty = CastField<FObjectPropertyBase>(Property))
            {
                if (1 == ObjectProperty->ArrayDim)
                {
                    if (!ObjectProperty->LoadObjectPropertyValue_InContainer(Component))
                    {
                        ActionContext->Error(
                            FText::Format(NSLOCTEXT("RuleRanger",
                                                    "ReqCompPropUnset",
                                                    "The {0} class (or a parent class) declared that the property {1} "
                                                    "must be present on the sub-object named {2} and the property "
                                                    "is present but value is not set."),
                                          FText::FromString(Class->GetAuthoredName()),
                                          FText::FromString(PropertyName),
                                          FText::FromString(ComponentName)));
                    }
                }
                else
                {
                    for (int32 i = 0; i < ObjectProperty->ArrayDim; i++)
                    {
                        if (!ObjectProperty->LoadObjectPropertyValue_InContainer(Component, i))
                        {
                            ActionContext->Error(FText::Format(
                                NSLOCTEXT("RuleRanger",
                                          "ReqCompPropUnsetIndexed",
                                          "The {0} class (or a parent class) declared that the property {1} "
                                          "must be present on the sub-object named {2} and the property is "
                                          "present but value is not set at index {3}."),
                                FText::FromString(Class->GetAuthoredName()),
                                FText::FromString(PropertyName),
                                FText::FromString(ComponentName),
                                FText::AsNumber(i)));
                        }
                    }
                }
            }
            else
            {
                ActionContext->Error(
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "ReqCompPropInvalidRef",
                                            "The {0} class (or a parent class) declared that the property {1} "
                                            "must be present on the sub-object named {2} and the property is "
                                            "present on the sub-object but the property is not a valid "
                                            "object reference."),
                                  FText::FromString(Class->GetAuthoredName()),
                                  FText::FromString(PropertyName),
                                  FText::FromString(ComponentName)));
            }
        }
        else
        {
            ActionContext->Error(
                FText::Format(NSLOCTEXT("RuleRanger",
                                        "ReqCompPropMissing",
                                        "The {0} class (or a parent class) declared that the property {1} "
                                        "must be present on the sub-object named {2} and the property is "
                                        "not present on the sub-object."),
                              FText::FromString(Class->GetAuthoredName()),
                              FText::FromString(PropertyName),
                              FText::FromString(ComponentName)));
        }
    }
}

void UEnsureRequiredPropertiesPresentAction::PerformChecksForComponentProperties(
    URuleRangerActionContext* ActionContext,
    const UObject* Object,
    const TMap<FString, TArray<FString>>& RequiredComponentProperties)
{
    const UClass* Class = Object->GetClass();
    for (const auto& Entry : RequiredComponentProperties)
    {
        const auto& ComponentName = Entry.Key;
        const auto& PropertyNames = Entry.Value;
        if (const FProperty* Property = Class->FindPropertyByName(FName(ComponentName)))
        {
            if (const auto ComponentProperty = CastField<FObjectPropertyBase>(Property))
            {
                if (1 == ComponentProperty->ArrayDim)
                {
                    if (const UObject* Component = ComponentProperty->LoadObjectPropertyValue_InContainer(Object))
                    {
                        PerformChecksForComponentProperties(ActionContext,
                                                            Class,
                                                            ComponentName,
                                                            Component,
                                                            PropertyNames);
                    }
                    else
                    {
                        // TODO: We could add configuration, that optionally ignored null sub-object references
                        ActionContext->Error(FText::Format(
                            NSLOCTEXT("RuleRanger",
                                      "ReqCompPropsNull",
                                      "The {0} class (or a parent class) declared that the properties [{1}] "
                                      "must be present on the sub-object named {2} but the sub-object "
                                      "property is null."),
                            FText::FromString(Class->GetAuthoredName()),
                            FText::FromString(FString::Join(PropertyNames, TEXT(","))),
                            FText::FromString(ComponentName)));
                    }
                }
                else
                {
                    // TODO: We could support this in the future but will not bother until there is a need
                    ActionContext->Error(
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "ReqCompPropsArrayUnsupported",
                                                "The {0} class (or a parent class) declared that the properties "
                                                "[{1}] must be present on the sub-object named {2} but the "
                                                "sub-object property is an array which is not supported."),
                                      FText::FromString(Class->GetAuthoredName()),
                                      FText::FromString(FString::Join(PropertyNames, TEXT(","))),
                                      FText::FromString(ComponentName)));
                }
            }
            else
            {
                ActionContext->Error(
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "ReqCompPropsInvalidRef",
                                            "The {0} class (or a parent class) declared that the properties "
                                            "[{1}] must be present on the sub-object named {2} but the "
                                            "sub-object property is not a valid object reference."),
                                  FText::FromString(Class->GetAuthoredName()),
                                  FText::FromString(FString::Join(PropertyNames, TEXT(","))),
                                  FText::FromString(ComponentName)));
            }
        }
        else
        {
            ActionContext->Error(
                FText::Format(NSLOCTEXT("RuleRanger",
                                        "ReqCompPropsMissingSubobject",
                                        "The {0} class (or a parent class) declared that the properties "
                                        "[{1}] must be present on the sub-object named {2} but no such "
                                        "sub-object property was defined for the class."),
                              FText::FromString(Class->GetAuthoredName()),
                              FText::FromString(FString::Join(PropertyNames, TEXT(","))),
                              FText::FromString(ComponentName)));
        }
    }
}

void UEnsureRequiredPropertiesPresentAction::Apply(URuleRangerActionContext* ActionContext, UObject* Object)
{
    const UBlueprint* Blueprint = Cast<UBlueprint>(Object);
    UClass* Class;
    if (Blueprint && Blueprint->GeneratedClass)
    {
        Class = Blueprint->GeneratedClass;
    }
    else
    {
        Class = Object->GetClass();
    }

    if (bSkipCheckOnAbstractTypes && FRuleRangerUtilities::IsAbstract(Object))
    {
        return;
    }

    // A map of ComponentName -> [PropertyName] of properties that are required
    TMap<FString, TArray<FString>> RequiredComponentProperties;
    DeriveRequiredComponentProperties(Class, RequiredComponentProperties);

    const UObject* DefaultObject = Class->GetDefaultObject(true);

    if (!RequiredComponentProperties.IsEmpty())
    {
        PerformChecksForComponentProperties(ActionContext, DefaultObject, RequiredComponentProperties);
    }

    // Perform checks for properties directly on the object
    PerformChecksOnObject(ActionContext, DefaultObject);
}
