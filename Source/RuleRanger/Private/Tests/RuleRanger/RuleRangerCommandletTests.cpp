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
    #include "Dom/JsonObject.h"
    #include "Engine/Blueprint.h"
    #include "Misc/AutomationTest.h"
    #include "Misc/PackageName.h"
    #include "RuleRanger/RuleRangerUtilities.h"
    #include "RuleRanger/UI/Commandlet/RuleRangerCommandlet.h"
    #include "RuleRangerActionContext.h"
    #include "RuleRangerProjectRule.h"
    #include "RuleRangerRuleSet.h"
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

    static void DeriveAllowlistPaths(URuleRangerCommandlet* const Commandlet,
                                     const FString& Params,
                                     TArray<FString>& AllowlistPaths)
    {
        Commandlet->DeriveAllowlistPaths(Params, AllowlistPaths);
    }

    static void DeriveAllowlistPackages(URuleRangerCommandlet* const Commandlet,
                                        const FString& Params,
                                        TArray<FString>& AllowlistPackages)
    {
        Commandlet->DeriveAllowlistPackages(Params, AllowlistPackages);
    }

    static void SetCurrentAsset(URuleRangerCommandlet* const Commandlet, const FAssetData& Asset)
    {
        Commandlet->CurrentAsset = Asset;
    }

    static int32 GetNumErrors(const URuleRangerCommandlet* const Commandlet) { return Commandlet->NumErrors; }

    static int32 GetNumWarnings(const URuleRangerCommandlet* const Commandlet) { return Commandlet->NumWarnings; }

    static int32 GetNumFatals(const URuleRangerCommandlet* const Commandlet) { return Commandlet->NumFatals; }

    static int32 GetNumAssetsScanned(const URuleRangerCommandlet* const Commandlet)
    {
        return Commandlet->NumAssetsScanned;
    }

    static int32 GetNumProjectRulesScanned(const URuleRangerCommandlet* const Commandlet)
    {
        return Commandlet->NumProjectRulesScanned;
    }

    static void ResetState(URuleRangerCommandlet* const Commandlet) { Commandlet->ResetState(); }

    static void ExecuteProjectRulesForConfigs(URuleRangerCommandlet* const Commandlet,
                                              const TConstArrayView<TWeakObjectPtr<URuleRangerConfig>> Configs,
                                              const bool bFix)
    {
        Commandlet->ExecuteProjectRulesForConfigs(Configs, bFix);
    }

    static const TArray<TSharedPtr<FJsonValue>>& GetAssetRuleResults(const URuleRangerCommandlet* const Commandlet)
    {
        return Commandlet->AssetRuleResults;
    }

    static const TArray<TSharedPtr<FJsonValue>>& GetProjectRuleResults(const URuleRangerCommandlet* const Commandlet)
    {
        return Commandlet->ProjectRuleResults;
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

    static URuleRangerConfig* CreateProjectRuleConfig(FAutomationTestBase& Test,
                                                      URuleRangerAutomationTestProjectAction*& OutAction)
    {
        const auto Config = RuleRangerTests::NewTransientObject<URuleRangerConfig>();
        const auto RuleSet = RuleRangerTests::NewTransientObject<URuleRangerRuleSet>(Config, TEXT("CommandletRuleSet"));
        const auto Rule = RuleRangerTests::NewTransientObject<URuleRangerProjectRule>(RuleSet, TEXT("CommandletRule"));
        OutAction = RuleRangerTests::NewTransientObject<URuleRangerAutomationTestProjectAction>(Rule);
        if (Test.TestNotNull(TEXT("Commandlet config should be created"), Config)
            && Test.TestNotNull(TEXT("Commandlet rule set should be created"), RuleSet)
            && Test.TestNotNull(TEXT("Commandlet project rule should be created"), Rule)
            && Test.TestNotNull(TEXT("Commandlet project action should be created"), OutAction))
        {
            OutAction->Outcome = ERuleRangerAutomationTestActionOutcome::Warning;
            Rule->Actions = { OutAction };
            RuleSet->ProjectRules = { Rule };
            Config->RuleSets = { RuleSet };
            return Config;
        }
        return nullptr;
    }
} // namespace RuleRangerCommandletTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCommandletDerivesAllowlistParametersTest,
                                 "RuleRanger.Commandlet.Parameters.DerivesAllowlists",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCommandletDerivesAllowlistParametersTest::RunTest(const FString&)
{
    const auto Commandlet = NewObject<URuleRangerCommandlet>();
    if (!TestNotNull(TEXT("Commandlet should be created"), Commandlet))
    {
        return false;
    }

    TArray<FString> Paths;
    TArray<FString> Packages;
    FRuleRangerCommandletTestAccessor::DeriveAllowlistPaths(Commandlet,
                                                            TEXT("-paths=/Game/One,/Game/Two -packages=/Game/A"),
                                                            Paths);
    FRuleRangerCommandletTestAccessor::DeriveAllowlistPackages(Commandlet,
                                                               TEXT("-paths=/Game/One -packages=/Game/A,/Game/B"),
                                                               Packages);

    return TestEqual(TEXT("Two path allowlist entries should be parsed"), Paths.Num(), 2)
        && TestEqual(TEXT("The first path should be preserved"), Paths[0], FString(TEXT("/Game/One")))
        && TestEqual(TEXT("The second path should be preserved"), Paths[1], FString(TEXT("/Game/Two")))
        && TestEqual(TEXT("Two package allowlist entries should be parsed"), Packages.Num(), 2)
        && TestEqual(TEXT("The first package should be preserved"), Packages[0], FString(TEXT("/Game/A")))
        && TestEqual(TEXT("The second package should be preserved"), Packages[1], FString(TEXT("/Game/B")));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCommandletOnRuleAppliedAggregatesJsonResultsTest,
                                 "RuleRanger.Commandlet.Results.OnRuleAppliedAggregatesJson",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCommandletOnRuleAppliedAggregatesJsonResultsTest::RunTest(const FString&)
{
    const auto Commandlet = NewObject<URuleRangerCommandlet>();
    RuleRangerTests::FRuleFixture Fixture;
    if (!TestNotNull(TEXT("Commandlet should be created"), Commandlet)
        || !RuleRangerTests::CreateRuleFixture(*this, Fixture))
    {
        return false;
    }

    FRuleRangerCommandletTestAccessor::SetCurrentAsset(Commandlet, FAssetData(Fixture.Object));
    Fixture.ActionContext->Warning(FText::FromString(TEXT("Commandlet warning")));
    Fixture.ActionContext->Error(FText::FromString(TEXT("Commandlet error")));
    Fixture.ActionContext->Fatal(FText::FromString(TEXT("Commandlet fatal")));

    Commandlet->OnRuleApplied(Fixture.ActionContext);

    const auto& Results = FRuleRangerCommandletTestAccessor::GetAssetRuleResults(Commandlet);
    const auto ResultObject = Results.Num() == 1 ? Results[0]->AsObject() : nullptr;
    return TestEqual(TEXT("Warnings should be aggregated"),
                     FRuleRangerCommandletTestAccessor::GetNumWarnings(Commandlet),
                     1)
        && TestEqual(TEXT("Errors should be aggregated"),
                     FRuleRangerCommandletTestAccessor::GetNumErrors(Commandlet),
                     1)
        && TestEqual(TEXT("Fatals should be aggregated"),
                     FRuleRangerCommandletTestAccessor::GetNumFatals(Commandlet),
                     1)
        && TestEqual(TEXT("One JSON asset result should be emitted"), Results.Num(), 1)
        && TestNotNull(TEXT("The JSON result object should be present"), ResultObject.Get())
        && TestEqual(TEXT("The asset result should include warning messages"),
                     ResultObject->GetArrayField(TEXT("Warnings")).Num(),
                     1)
        && TestEqual(TEXT("The asset result should include errors and fatals together"),
                     ResultObject->GetArrayField(TEXT("Errors")).Num(),
                     2);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCommandletMainHelpAndInvalidAssetsReturnExpectedCodesTest,
                                 "RuleRanger.Commandlet.Main.HelpAndInvalidAssetsReturnExpectedCodes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCommandletMainHelpAndInvalidAssetsReturnExpectedCodesTest::RunTest(const FString&)
{
    const auto Commandlet = NewObject<URuleRangerCommandlet>();
    if (!TestNotNull(TEXT("Commandlet should be created"), Commandlet))
    {
        return false;
    }

    const int32 HelpResult = Commandlet->Main(TEXT("-help"));

    AddExpectedMessagePlain(TEXT("Invalid staged content root /RuleRangerMissing"),
                            ELogVerbosity::Error,
                            EAutomationExpectedMessageFlags::Contains,
                            1);
    const int32 MissingPathResult = Commandlet->Main(TEXT("-assetsOnly -paths=/RuleRangerMissing"));

    return TestEqual(TEXT("Help should exit successfully"), HelpResult, 0)
        && TestEqual(TEXT("Missing asset paths should fail"), MissingPathResult, 1)
        && TestEqual(TEXT("Failure should reset asset count"),
                     FRuleRangerCommandletTestAccessor::GetNumAssetsScanned(Commandlet),
                     0);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerCommandletExecuteProjectRulesAggregatesReportAndFixResultsTest,
                                 "RuleRanger.Commandlet.ProjectRules.AggregatesReportAndFixResults",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerCommandletExecuteProjectRulesAggregatesReportAndFixResultsTest::RunTest(const FString&)
{
    URuleRangerAutomationTestProjectAction* Action = nullptr;
    const auto Config = RuleRangerCommandletTests::CreateProjectRuleConfig(*this, Action);
    if (!TestNotNull(TEXT("Commandlet project config should be available"), Config)
        || !TestNotNull(TEXT("Commandlet project action should be available"), Action))
    {
        return false;
    }

    const auto Commandlet = NewObject<URuleRangerCommandlet>();
    if (!TestNotNull(TEXT("Commandlet should be created"), Commandlet))
    {
        return false;
    }

    TArray<TWeakObjectPtr<URuleRangerConfig>> Configs;
    Configs.Add(Config);

    FRuleRangerCommandletTestAccessor::ExecuteProjectRulesForConfigs(Commandlet, Configs, false);
    const bool bReportRun = TestEqual(TEXT("Project report should apply the rule once"), Action->GetApplyCount(), 1)
        && TestEqual(TEXT("Project report should use the report trigger"),
                     Action->GetLastTrigger(),
                     ERuleRangerProjectActionTrigger::AT_Report)
        && TestEqual(TEXT("Project report should count the warning"),
                     FRuleRangerCommandletTestAccessor::GetNumWarnings(Commandlet),
                     1)
        && TestEqual(TEXT("Project report should aggregate one JSON result"),
                     FRuleRangerCommandletTestAccessor::GetProjectRuleResults(Commandlet).Num(),
                     1);

    FRuleRangerCommandletTestAccessor::ResetState(Commandlet);
    FRuleRangerCommandletTestAccessor::ExecuteProjectRulesForConfigs(Commandlet, Configs, true);

    return bReportRun && TestEqual(TEXT("Project fix should apply the rule again"), Action->GetApplyCount(), 2)
        && TestEqual(TEXT("The fix invocation should use the fix trigger"),
                     Action->GetLastTrigger(),
                     ERuleRangerProjectActionTrigger::AT_Fix)
        && TestEqual(TEXT("Project fix should count the project rule"),
                     FRuleRangerCommandletTestAccessor::GetNumProjectRulesScanned(Commandlet),
                     1)
        && TestEqual(TEXT("Project fix should count the warning"),
                     FRuleRangerCommandletTestAccessor::GetNumWarnings(Commandlet),
                     1);
}

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
