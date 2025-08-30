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
#include "EnsureDataOnlyBlueprintAction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "RuleRanger/RuleRangerUtilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureDataOnlyBlueprintAction)

static const FName RuleRangerDataOnlyPropertyName("RuleRangerDataOnly");

static FString GetObjectTypesAsString(const TArray<TSubclassOf<UObject>>& ObjectTypes)
{
    TArray<FString> ClassNames;
    ClassNames.Reserve(ObjectTypes.Num());

    for (const auto& ObjectType : ObjectTypes)
    {
        if (ObjectType)
        {
            ClassNames.Add(ObjectType->GetPathName());
        }
    }

    return FString::Join(ClassNames, TEXT(", "));
}

void UEnsureDataOnlyBlueprintAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    bool bMatchedViaMetaProperty = false;
    TSubclassOf<UObject> MatchedObjectType{ nullptr };
    for (const auto ObjectType : ObjectTypes)
    {
        if (FRuleRangerUtilities::IsA(Object, ObjectType))
        {
            MatchedObjectType = ObjectType;
        }
    }
    if (!MatchedObjectType)
    {
        TArray<UClass*> Classes;
        FRuleRangerUtilities::CollectTypeHierarchy(Object, Classes);
        for (const auto Class : Classes)
        {
            if (Class->HasMetaData(RuleRangerDataOnlyPropertyName))
            {
                MatchedObjectType = Class;
                bMatchedViaMetaProperty = true;
                break;
            }
        }
    }

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto Blueprint = CastChecked<UBlueprint>(Object);
    if (MatchedObjectType && !FBlueprintEditorUtils::IsDataOnlyBlueprint(Blueprint))
    {
        if (!bMatchedViaMetaProperty)
        {
            ActionContext->Error(FText::FromString(
                FString::Printf(TEXT("Object is not a DataOnlyBlueprint but it extends the type %s that is "
                                     "part of the list of DataOnlyBlueprints: %s"),
                                *MatchedObjectType->GetPathName(),
                                *GetObjectTypesAsString(ObjectTypes))));
        }
        else
        {
            ActionContext->Error(FText::FromString(
                FString::Printf(TEXT("Object is not a DataOnlyBlueprint but it extends the type %s that has "
                                     "the meta property '%s'"),
                                *MatchedObjectType->GetPathName(),
                                *RuleRangerDataOnlyPropertyName.ToString())));
        }
    }
}

UClass* UEnsureDataOnlyBlueprintAction::GetExpectedType()
{
    return UBlueprint::StaticClass();
}
