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

#include "RuleRanger/Actions/SetMetadataTagsAction.h"
#include "Editor.h"
#include "RuleRangerLogging.h"
#include "Subsystems/EditorAssetSubsystem.h"

void USetMetadataTagsAction::Apply_Implementation(TScriptInterface<IRuleRangerActionContext>& ActionContext,
                                                  UObject* Object)
{
    if (IsValid(Object))
    {
        if (const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>())
        {
            for (const auto& MetadataTag : MetadataTags)
            {
                if (NAME_None == MetadataTag.Key)
                {
                    UE_LOG(RuleRanger,
                           Error,
                           TEXT("SetMetadataTagsAction: Empty key specified when attempting to add MetadataTag to %s"),
                           *Object->GetName());
                }
                else if (MetadataTag.Value.IsEmpty())
                {
                    UE_LOG(RuleRanger,
                           Error,
                           TEXT("SetMetadataTagsAction: Empty value specified when attempting to "
                                "add MetadataTag to %s"),
                           *Object->GetName());
                }
                else
                {
                    FString ExistingValue = Subsystem->GetMetadataTag(Object, MetadataTag.Key);
                    if (ExistingValue.Equals(MetadataTag.Value))
                    {
                        UE_LOG(RuleRanger,
                               Verbose,
                               TEXT("SetMetadataTagsAction: MetaDataTag %s=%s already exists on %s. "
                                    "No action required."),
                               *MetadataTag.Key.ToString(),
                               *MetadataTag.Value,
                               *Object->GetName());
                    }
                    else
                    {
                        if (ActionContext->IsDryRun())
                        {
                            FFormatNamedArguments Arguments;
                            Arguments.Add(TEXT("Key"), FText::FromString(MetadataTag.Key.ToString()));
                            Arguments.Add(TEXT("Value"), FText::FromString(MetadataTag.Value));
                            const FText Message = FText::Format(
                                NSLOCTEXT(
                                    "RuleRanger",
                                    "RuleRangerMissingMetaDataTag",
                                    "MetaData tag {Key}={Value} is not present and would add if not in DryRun mode."),
                                Arguments);

                            ActionContext->Info(Message);
                        }
                        else
                        {
                            UE_LOG(RuleRanger,
                                   Verbose,
                                   TEXT("SetMetadataTagsAction: MetaDataTag %s=%s is not present on %s. "
                                        "Adding."),
                                   *MetadataTag.Key.ToString(),
                                   *MetadataTag.Value,
                                   *Object->GetName());
                            Subsystem->SetMetadataTag(Object, MetadataTag.Key, MetadataTag.Value);
                        }
                    }
                }
            }
        }
    }
}
