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
#include "MetasoundSource.h"
#include "Misc/PackageName.h"
#include "RuleRanger/MetaSound/MetaSoundPresetUtilities.h"
#include "RuleRanger/RuleRangerUtilities.h"
#include "RuleRangerActionContext.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnsureNoMetaSoundSourceReferenceAction)

class IAssetRegistry;

namespace RuleRanger::MetaSoundAction
{
    UObject* TryLoadPackageRepresentativeAsset(const FName PackageName)
    {
        if (PackageName.IsNone())
        {
            return nullptr;
        }

        const auto& AssetRegistry = FAssetRegistryModule::GetRegistry();
        TArray<FAssetData> Assets;
        if (AssetRegistry.GetAssetsByPackageName(PackageName, Assets, true))
        {
            TArray<FAssetData> RepresentativeAssets;
            FRuleRangerUtilities::AddPackageRepresentativeAssets(Assets, RepresentativeAssets);
            for (const auto& Asset : RepresentativeAssets)
            {
                if (UObject* const Object = Asset.GetAsset())
                {
                    return Object;
                }
            }
        }

        const FString PackageNameString = PackageName.ToString();
        const FString ObjectPath =
            FString::Printf(TEXT("%s.%s"), *PackageNameString, *FPackageName::GetShortName(PackageNameString));
        return FSoftObjectPath(ObjectPath).TryLoad();
    }

    bool ShouldIgnorePackageDependency(const FName PackageName)
    {
        const FString PackageNameString = PackageName.ToString();
        return PackageNameString.StartsWith(TEXT("/Script/"))
            || PackageNameString.Contains(TEXT("/__ExternalActors__/"))
            || PackageNameString.Contains(TEXT("/__ExternalObjects__/"));
    }
} // namespace RuleRanger::MetaSoundAction

UEnsureNoMetaSoundSourceReferenceAction::UEnsureNoMetaSoundSourceReferenceAction()
{
    AllowList.Add(UMetaSoundSource::StaticClass());
}

bool UEnsureNoMetaSoundSourceReferenceAction::IsReferenceAllowed(const UObject* MetaSoundSource,
                                                                 const UObject* Other) const
{
    // Allow if the referenced MetaSoundSource is a preset.
    if (RuleRanger::MetaSound::IsPreset(CastChecked<UMetaSoundSource>(MetaSoundSource)))
    {
        return true;
    }
    if (const auto OtherMetaSoundSource = Cast<UMetaSoundSource>(Other))
    {
        // This means that the referencing object is a preset derived from this MetaSoundSource which is fine
        if (RuleRanger::MetaSound::IsPreset(OtherMetaSoundSource))
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
}

void UEnsureNoMetaSoundSourceReferenceAction::Apply(URuleRangerActionContext* ActionContext, UObject* Object)
{
    if (!Object)
    {
        return;
    }

    const auto PackageName = FSoftObjectPath(Object).GetAssetPath().GetPackageName();
    const auto& AssetRegistry = FAssetRegistryModule::GetRegistry();

    if (Object->IsA(UMetaSoundSource::StaticClass()))
    {
        TArray<FName> Referencers;
        AssetRegistry.GetReferencers(PackageName, Referencers, UE::AssetRegistry::EDependencyCategory::All);

        for (const auto Referencer : Referencers)
        {
            const auto Ref = RuleRanger::MetaSoundAction::TryLoadPackageRepresentativeAsset(Referencer);
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
        AssetRegistry.GetDependencies(PackageName, Dependencies, UE::AssetRegistry::EDependencyCategory::Package);

        for (const auto Referencer : Dependencies)
        {
            if (!RuleRanger::MetaSoundAction::ShouldIgnorePackageDependency(Referencer))
            {
                const auto Ref = RuleRanger::MetaSoundAction::TryLoadPackageRepresentativeAsset(Referencer);
                if (Ref && Ref->IsA(UMetaSoundSource::StaticClass()) && !IsReferenceAllowed(Ref, Object))
                {
                    FFormatNamedArguments Arguments;
                    Arguments.Add(TEXT("Object"), FText::FromString(GetNameSafe(Object)));
                    Arguments.Add(TEXT("MetaSoundSource"), FText::FromString(GetNameSafe(Ref)));

                    ActionContext->Error(FText::Format(
                        NSLOCTEXT("RuleRanger",
                                  "EnsureNoMetaSoundSourceReferenceAction_BadReference",
                                  "Object {Object} directly references MetaSoundSource {MetaSoundSource}. "
                                  "Policy indicates that the reference should be to a preset."),
                        Arguments));
                }
            }
        }
    }
}

void UEnsureNoMetaSoundSourceReferenceAction::PreSave(const FObjectPreSaveContext SaveContext)
{
    // Remove null/empty paths
    AllowList.RemoveAll([](const auto& Value) { return Value.IsNull(); });

    // Deduplicate entries
    {
        TSet<FSoftObjectPath> Unique;
        Unique.Reserve(AllowList.Num());
        for (const auto& Path : AllowList)
        {
            Unique.Add(Path);
        }
        AllowList = Unique.Array();
    }

    // Sort by path string (stable, case-sensitive)
    AllowList.Sort([](const auto& A, const auto& B) { return A.GetAssetName() < B.GetAssetName(); });

    Super::PreSave(SaveContext);
}
