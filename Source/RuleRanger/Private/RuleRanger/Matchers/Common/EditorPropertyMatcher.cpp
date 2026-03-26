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
#include "EditorPropertyMatcher.h"
#include "Editor.h"
#include "Kismet/KismetStringLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EditorPropertyMatcher)

namespace RuleRanger::EditorPropertyMatcher
{
    bool MatchEnumValue(const UEnum* Enum, const int64 EnumValue, const FString& ExpectedValue)
    {
        const int32 EnumIndex = Enum->GetIndexByValue(EnumValue);
        const FName EnumName = Enum->GetNameByIndex(EnumIndex);
        const FString EnumAuthoredName = Enum->GetAuthoredNameStringByIndex(EnumIndex);
        const FText EnumDisplayName = Enum->GetDisplayNameTextByIndex(EnumIndex);
        return EnumName.ToString().Equals(ExpectedValue) || EnumAuthoredName.Equals(ExpectedValue)
            || EnumDisplayName.ToString().Equals(ExpectedValue);
    }
} // namespace RuleRanger::EditorPropertyMatcher

bool UEditorPropertyMatcher::TestEditorProperty(UObject* Object, UObject* Instance, FProperty* Property) const
{
    if (const auto BoolProperty = CastField<const FBoolProperty>(Property))
    {
        return BoolProperty->GetPropertyValue_InContainer(Instance) == Value.Equals("true", ESearchCase::IgnoreCase);
    }
    else if (const auto EnumProperty = CastField<const FEnumProperty>(Property))
    {
        const UEnum* Enum = EnumProperty->GetEnum();
        const void* const EnumValuePtr = EnumProperty->ContainerPtrToValuePtr<void>(Instance);
        const int64 EnumValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(EnumValuePtr);
        return RuleRanger::EditorPropertyMatcher::MatchEnumValue(Enum, EnumValue, Value);
    }
    else if (const auto NumericProperty = CastField<FNumericProperty>(Property))
    {
        if (NumericProperty->IsEnum())
        {
            const UEnum* Enum = NumericProperty->GetIntPropertyEnum();
            const int64 EnumValue = NumericProperty->GetSignedIntPropertyValue_InContainer(Instance);
            return RuleRanger::EditorPropertyMatcher::MatchEnumValue(Enum, EnumValue, Value);
        }
        else if (NumericProperty->IsInteger())
        {
            const void* const NumericValue = NumericProperty->ContainerPtrToValuePtr<void>(Instance);
            return NumericProperty->GetSignedIntPropertyValue(NumericValue)
                == UKismetStringLibrary::Conv_StringToInt64(Value);
        }
        else if (NumericProperty->IsFloatingPoint())
        {
            const void* const NumericValue = NumericProperty->ContainerPtrToValuePtr<void>(Instance);
            return NumericProperty->GetFloatingPointPropertyValue(NumericValue)
                == UKismetStringLibrary::Conv_StringToDouble(Value);
        }
    }
    return false;
}
