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

    #include "AssetRegistry/AssetRegistryModule.h"
    #include "Engine/Blueprint.h"
    #include "Misc/AutomationTest.h"
    #include "Misc/PackageName.h"
    #include "RuleRanger/RuleRangerUtilities.h"
    #include "RuleRanger/UI/Commandlet/RuleRangerCommandlet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "UObject/Package.h"
    #include "UObject/SavePackage.h"

class FRuleRangerCommandletTestAccessor
{
public:
    static bool CollectAssetsFromPackageAllowlist(URuleRangerCommandlet* const Commandlet,
                                                  const TArray<FString>& AllowlistPackages,
                                                  TArray<FAssetData>& Assets)
    {
        return Commandlet->CollectAssetsFromPackageAllowlist(AllowlistPackages, Assets);
    }

    static bool CollectAssetsFromPathAllowlist(URuleRangerCommandlet* const Commandlet,
                                               const TArray<FString>& AllowlistPaths,
                                               TArray<FAssetData>& Assets)
    {
        return Commandlet->CollectAssetsFromPathAllowlist(AllowlistPaths, Assets);
    }
};

namespace RuleRangerCommandletTests
{
    struct FBlueprintFixture
    {
        FString PackageName;
        FString DirectoryPath;
        FString ObjectName;
        TObjectPtr<UBlueprint> Blueprint{ nullptr };
    };

    static bool SaveBlueprintAndRefreshAssetRegistry(UBlueprint* const Blueprint)
    {
        const auto Package = Blueprint ? Blueprint->GetPackage() : nullptr;
        if (!Package)
        {
            return false;
        }

        const FString Filename =
            FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
        FSavePackageArgs SaveArgs;
        SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
        SaveArgs.SaveFlags = SAVE_NoError;
        if (!UPackage::SavePackage(Package, Blueprint, *Filename, SaveArgs))
        {
            return false;
        }

        UPackage::WaitForAsyncFileWrites();

        auto& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
        AssetRegistry.ScanModifiedAssetFiles({ Filename });
        AssetRegistry.WaitForCompletion();
        return true;
    }

    static bool CreateBlueprintFixture(FAutomationTestBase& Test,
                                       const FString& PackageName,
                                       const FString& ObjectName,
                                       FBlueprintFixture& OutFixture)
    {
        const auto Blueprint = RuleRangerTests::NewBlueprint(URuleRangerAutomationBlueprintParentObject::StaticClass(),
                                                             *PackageName,
                                                             *ObjectName);
        if (!Test.TestNotNull(TEXT("Blueprint should be created"), Blueprint))
        {
            return false;
        }

        RuleRangerTests::CompileBlueprint(Blueprint);
        if (!Test.TestTrue(TEXT("Blueprint package should save cleanly"),
                           SaveBlueprintAndRefreshAssetRegistry(Blueprint)))
        {
            return false;
        }

        OutFixture.PackageName = PackageName;
        OutFixture.DirectoryPath = FPackageName::GetLongPackagePath(PackageName);
        OutFixture.ObjectName = ObjectName;
        OutFixture.Blueprint = Blueprint;
        return true;
    }

    static bool VerifyRepresentativeSelectionFromCandidateRows(FAutomationTestBase& Test, UBlueprint* const Blueprint)
    {
        UClass* const GeneratedClass = Blueprint ? Blueprint->GeneratedClass.Get() : nullptr;
        if (!Test.TestNotNull(TEXT("Generated class should be available"), GeneratedClass))
        {
            return false;
        }

        TArray<FAssetData> CandidateAssets;
        CandidateAssets.Emplace(Blueprint);
        CandidateAssets.Emplace(GeneratedClass, /*bAllowBlueprintClass=*/true);

        TArray<FAssetData> RepresentativeAssets;
        FRuleRangerUtilities::AddPackageRepresentativeAssets(CandidateAssets, RepresentativeAssets);

        return Test.TestEqual(TEXT("Exactly one package representative should be selected"),
                              RepresentativeAssets.Num(),
                              1)
            && Test.TestEqual(TEXT("The package representative should be the Blueprint asset"),
                              RepresentativeAssets[0].GetObjectPathString(),
                              Blueprint->GetPathName())
            && Test.TestFalse(TEXT("The package representative should not be the generated class"),
                              RepresentativeAssets[0].AssetName.ToString().EndsWith(TEXT("_C")));
    }

    static bool VerifyRepresentativeSelection(FAutomationTestBase& Test,
                                              const FBlueprintFixture& Fixture,
                                              const bool bCollectByPath)
    {
        const auto Commandlet = NewObject<URuleRangerCommandlet>();
        if (!Test.TestNotNull(TEXT("Commandlet should be created"), Commandlet))
        {
            return false;
        }

        TArray<FAssetData> Assets;
        const bool bResolved = bCollectByPath
            ? FRuleRangerCommandletTestAccessor::CollectAssetsFromPathAllowlist(Commandlet,
                                                                                { Fixture.DirectoryPath },
                                                                                Assets)
            : FRuleRangerCommandletTestAccessor::CollectAssetsFromPackageAllowlist(Commandlet,
                                                                                   { Fixture.PackageName },
                                                                                   Assets);
        if (!Test.TestTrue(TEXT("Allowlist query should resolve"), bResolved))
        {
            return false;
        }

        const FString ExpectedObjectPath = Fixture.Blueprint->GetPathName();
        return Test.TestEqual(TEXT("Exactly one editor-facing representative asset should be selected"),
                              Assets.Num(),
                              1)
            && Test.TestEqual(TEXT("The representative asset should be the Blueprint asset"),
                              Assets[0].GetObjectPathString(),
                              ExpectedObjectPath)
            && Test.TestFalse(TEXT("The representative asset should not be the generated class"),
                              Assets[0].AssetName.ToString().EndsWith(TEXT("_C")));
    }
} // namespace RuleRangerCommandletTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCommandletPackageAllowlistUsesEditorFacingAssetTest,
                                 "RuleRanger.Commandlet.PackageAllowlist.UsesEditorFacingAsset",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCommandletPackageAllowlistUsesEditorFacingAssetTest::RunTest(const FString&)
{
    RuleRangerCommandletTests::FBlueprintFixture Fixture;
    const FString PackageName = FString::Printf(TEXT("%s/Commandlet/PackageRepresentativeAsset"),
                                                RuleRangerTests::GetRuleRangerTestMountRoot());
    const FString ObjectName = TEXT("BP_CommandletPackageRepresentative");
    if (!RuleRangerCommandletTests::CreateBlueprintFixture(*this, PackageName, ObjectName, Fixture))
    {
        return false;
    }

    return RuleRangerCommandletTests::VerifyRepresentativeSelectionFromCandidateRows(*this, Fixture.Blueprint)
        && RuleRangerCommandletTests::VerifyRepresentativeSelection(*this, Fixture, false);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCommandletPathAllowlistUsesEditorFacingAssetTest,
                                 "RuleRanger.Commandlet.PathAllowlist.UsesEditorFacingAsset",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCommandletPathAllowlistUsesEditorFacingAssetTest::RunTest(const FString&)
{
    RuleRangerCommandletTests::FBlueprintFixture Fixture;
    const FString PackageName =
        FString::Printf(TEXT("%s/Commandlet/PathRepresentativeAsset"), RuleRangerTests::GetRuleRangerTestMountRoot());
    const FString ObjectName = TEXT("BP_CommandletPathRepresentative");
    if (!RuleRangerCommandletTests::CreateBlueprintFixture(*this, PackageName, ObjectName, Fixture))
    {
        return false;
    }

    return RuleRangerCommandletTests::VerifyRepresentativeSelectionFromCandidateRows(*this, Fixture.Blueprint)
        && RuleRangerCommandletTests::VerifyRepresentativeSelection(*this, Fixture, true);
}

#endif
