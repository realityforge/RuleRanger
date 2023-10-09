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

#include "RuleRanger/Actions/NameConventionRenameAction.h"
#include "Editor.h"
#include "Misc/UObjectToken.h"
#include "RuleRangerLogging.h"
#include "Subsystems/EditorAssetSubsystem.h"

void UNameConventionRenameAction::Apply_Implementation(TScriptInterface<IRuleRangerActionContext>& ActionContext,
                                                       UObject* Object)
{
    if (IsValid(Object) && IsValid(NameConventionsTable))
    {
        RebuildNameConventionsCacheIfNecessary();

        if (!NameConventionsMap.IsEmpty())
        {
            const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
            const auto Variant = Subsystem ? Subsystem->GetMetadataTag(Object, FName("RuleRanger.Variant")) : TEXT("");

            TArray<UClass*> Classes;
            CollectTypeHierarchy(Object, Classes);
            for (auto Class : Classes)
            {
                UE_LOG(RuleRanger,
                       VeryVerbose,
                       TEXT("NameConventionRenameAction: Looking for NamingConvention rules for class %s"),
                       *Class->GetName());
                if (TArray<FNameConvention>* NameConventions = NameConventionsMap.Find(Class))
                {
                    UE_LOG(RuleRanger,
                           VeryVerbose,
                           TEXT("NameConventionRenameAction: Found NamingConvention %d rules for %s"),
                           NameConventions->Num(),
                           *Class->GetName());
                    for (int i = 0; i < NameConventions->Num(); i++)
                    {
                        const FNameConvention& NameConvention = (*NameConventions)[i];
                        if (NameConvention.Variant.Equals(Variant)
                            || NameConvention.Variant.Equals(NameConvention_DefaultVariant))
                        {
                            const FString OriginalName{ Object->GetName() };
                            FString NewName{ OriginalName };
                            if (!NameConvention.Prefix.IsEmpty() && !NewName.StartsWith(NameConvention.Prefix))
                            {
                                NewName.InsertAt(0, NameConvention.Prefix);
                            }
                            if (!NameConvention.Suffix.IsEmpty() && !NewName.EndsWith(NameConvention.Suffix))
                            {
                                NewName.Append(NameConvention.Suffix);
                            }
                            if (!Object->Rename(*NewName))
                            {
                                FText InMessage =
                                    FText::Format(NSLOCTEXT("RuleRanger",
                                                            "RenameFailed",
                                                            "Attempt to rename object '{0}' to '{1}' failed."),
                                                  FText::FromString(*OriginalName),
                                                  FText::FromString(*NewName));
                                FMessageLog(RuleRangerMessageLogName)
                                    .Error()
                                    ->AddToken(FUObjectToken::Create(Object))
                                    ->AddToken(FTextToken::Create(InMessage));
                            }
                        }
                    }
                }
            }
        }
    }
}

void UNameConventionRenameAction::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    const FName TableName = GET_MEMBER_NAME_CHECKED(UNameConventionRenameAction, NameConventionsTable);
    if (TableName == PropertyName)
    {
        ResetNameConventionsCache();
    }
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UNameConventionRenameAction::ResetCacheIfTableModified(UObject* Object)
{
    // This is called on any object edit in editor so match against conventions table and bust cache as appropriate;
    if (Object && Object == NameConventionsTable)
    {
        ResetNameConventionsCache();
    }
}

void UNameConventionRenameAction::ResetNameConventionsCache()
{
    UE_LOG(RuleRanger, Verbose, TEXT("NameConventionRenameAction: Resetting the Name Conventions Cache"));

    NameConventionsMap.Empty();
    FCoreUObjectDelegates::OnObjectModified.Remove(OnObjectModifiedDelegateHandle);
    OnObjectModifiedDelegateHandle.Reset();
}

void UNameConventionRenameAction::RebuildNameConventionsCacheIfNecessary()
{
    if (NameConventionsMap.IsEmpty() && 0 != NameConventionsTable->GetTableData().Num())
    {
        ResetNameConventionsCache();
        // Add a callback for when ANY object is modified in the editor so that we can bust the cache
        OnObjectModifiedDelegateHandle =
            FCoreUObjectDelegates::OnObjectModified.AddUObject(this,
                                                               &UNameConventionRenameAction::ResetCacheIfTableModified);
        for (const auto RowName : NameConventionsTable->GetRowNames())
        {
            const auto NameConvention = NameConventionsTable->FindRow<FNameConvention>(RowName, TEXT(""));
            const auto ObjectType = NameConvention->ObjectType.Get();
            if (NameConvention && IsValid(ObjectType))
            {
                TArray<FNameConvention>& TypeConventions = NameConventionsMap.FindOrAdd(ObjectType);
                TypeConventions.Add(*NameConvention);
                TypeConventions.Sort();
            }
        }
        for (auto NameConventionEntry : NameConventionsMap)
        {
            UE_LOG(RuleRanger,
                   VeryVerbose,
                   TEXT("NameConventionRenameAction: Object %s contains %d conventions in cache"),
                   *NameConventionEntry.Key->GetName(),
                   NameConventionEntry.Value.Num());
        }
    }
}

void UNameConventionRenameAction::CollectTypeHierarchy(const UObject* Object, TArray<UClass*>& Classes)
{
    bool bProcessedBlueprintHierarchy{ false };
    UClass* Class = Object->GetClass();
    while (Class)
    {
        if (!bProcessedBlueprintHierarchy && Object->IsA<UBlueprint>())
        {
            bProcessedBlueprintHierarchy = true;
            // If Object is a Blueprint then we have an alternate hierarchy accessible via the ParentClass property.
            // TODO: Assess with a similar pattern needs to be applied for other asset based class hierarchies...
            UClass* BlueprintClass{ Cast<UBlueprint>(Object)->ParentClass };
            while (BlueprintClass)
            {
                Classes.Add(BlueprintClass);
                BlueprintClass = BlueprintClass->GetSuperClass();
            }
        }
        Classes.Add(Class);
        Class = Class->GetSuperClass();
    }
}
