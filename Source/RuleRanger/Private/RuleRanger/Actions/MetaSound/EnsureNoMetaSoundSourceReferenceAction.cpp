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
#include "EnsureNoMetaSoundSourceReferenceAction.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Logging/StructuredLog.h"
#include "RuleRangerActionContext.h"
#if WITH_RULERANGER_METASOUND_RULES
    #include "MetasoundSource.h"
#else
    #include "Sound/SoundWaveProcedural.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureNoMetaSoundSourceReferenceAction)

class IAssetRegistry;

UEnsureNoMetaSoundSourceReferenceAction::UEnsureNoMetaSoundSourceReferenceAction()
{
#if WITH_RULERANGER_METASOUND_RULES
    AllowList.Add(UMetaSoundSource::StaticClass());
#endif
}

bool UEnsureNoMetaSoundSourceReferenceAction::IsReferenceAllowed(const UObject* MetaSoundSource,
                                                                 const UObject* Other) const
{
#if WITH_RULERANGER_METASOUND_RULES
    // Allow if MetaSoundSource with bIsPreset = true
    if (CastChecked<UMetaSoundSource>(MetaSoundSource)->GetConstDocument().RootGraph.PresetOptions.bIsPreset)
    {
        return true;
    }
    if (const auto OtherMetaSoundSource = Cast<UMetaSoundSource>(Other))
    {
        // This means that the referencing object is a preset derived from this MetaSoundSource which is fine
        if (OtherMetaSoundSource->GetConstDocument().RootGraph.PresetOptions.bIsPreset)
        {
            return true;
        }
    }
    // Allow if MetaSourceSource is in the allowlist
    for (const auto& Allowed : AllowList)
    {
        const auto Object = Allowed.TryLoad();
        if (IsValid(MetaSoundSource) && IsValid(Object) && MetaSoundSource == Object)
        {
            return true;
        }
    }
    return false;
#else
    return true;
#endif
}

void UEnsureNoMetaSoundSourceReferenceAction::Apply(URuleRangerActionContext* ActionContext, UObject* Object)
{
#if WITH_RULERANGER_METASOUND_RULES
    const auto PackageName = FSoftObjectPath(Object).GetAssetPath().GetPackageName();
    const auto& AssetRegistry = FAssetRegistryModule::GetRegistry();

    if (Object->IsA(UMetaSoundSource::StaticClass()))
    {
        TArray<FName> Referencers;
        AssetRegistry.GetReferencers(PackageName, Referencers, UE::AssetRegistry::EDependencyCategory::All);

        for (const auto Referencer : Referencers)
        {
            const auto Ref = FSoftObjectPath(Referencer.ToString()).TryLoad();
            if (Ref && !IsReferenceAllowed(Object, Ref))
            {
                FFormatNamedArguments Arguments;
                Arguments.Add(TEXT("Object"), FText::FromString(Referencer.ToString()));
                Arguments.Add(TEXT("MetaSoundSource"), FText::FromString(GetNameSafe(Object)));

                ActionContext->Error(
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "EnsureNoMetaSoundSourceReferenceAction_BadReference",
                                            "Object {Object} directly references MetaSoundSource {MetaSoundSource}. "
                                            "Policy indicates that the reference should be to a preset."),
                                  Arguments));
            }
        }
    }
    else
    {
        TArray<FName> Dependencies;
        AssetRegistry.GetDependencies(PackageName, Dependencies, UE::AssetRegistry::EDependencyCategory::All);

        for (const auto Referencer : Dependencies)
        {
            const auto Ref = FSoftObjectPath(Referencer.ToString()).TryLoad();
            if (Ref && Ref->IsA(UMetaSoundSource::StaticClass()) && !IsReferenceAllowed(Ref, Object))
            {
                FFormatNamedArguments Arguments;
                Arguments.Add(TEXT("Object"), FText::FromString(GetNameSafe(Object)));
                Arguments.Add(TEXT("MetaSoundSource"), FText::FromString(GetNameSafe(Ref)));

                ActionContext->Error(
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "EnsureNoMetaSoundSourceReferenceAction_BadReference",
                                            "Object {Object} directly references MetaSoundSource {MetaSoundSource}. "
                                            "Policy indicates that the reference should be to a preset."),
                                  Arguments));
            }
        }
    }
#endif
}
