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
    #include "DocumentTemplates/MetasoundFrontendPresetTemplate.h"
    #include "HAL/FileManager.h"
    #include "MetasoundSource.h"
    #include "Misc/AutomationTest.h"
    #include "Misc/PackageName.h"
    #include "Misc/Paths.h"
    #include "RuleRanger/Actions/MetaSound/EnsureMetaSoundAuthorBlankAction.h"
    #include "RuleRanger/Actions/MetaSound/EnsureNoMetaSoundSourceReferenceAction.h"
    #include "RuleRanger/Matchers/MetaSound/MetasoundPresetMatcher.h"
    #include "RuleRangerAction.h"
    #include "RuleRangerActionContext.h"
    #include "Tests/RuleRanger/RuleRangerMetaSoundAutomationTestTypes.h"
    #include "UObject/Package.h"
    #include "UObject/SavePackage.h"
    #include "UObject/UnrealType.h"

namespace RuleRangerMetaSoundTests
{
    constexpr auto AutomationTestFlags =
        EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

    template <typename TObject>
    TObject* NewTransientObject(UObject* const Outer = GetTransientPackage(), const FName Name = NAME_None)
    {
        return NewObject<TObject>(Outer, Name, RF_Transient);
    }

    template <typename TObject>
    TObject* NewNamedTransientObject(const TCHAR* const Name, UObject* const Outer = GetTransientPackage())
    {
        return NewTransientObject<TObject>(Outer, FName(Name));
    }

    template <typename TObject, typename TValue>
    bool
    SetPropertyValue(FAutomationTestBase& Test, TObject* const Object, const TCHAR* PropertyName, const TValue& Value)
    {
        const auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
        if (Test.TestNotNull(FString::Printf(TEXT("Property %s should exist"), PropertyName), Property))
        {
            auto ValuePtr = Property->template ContainerPtrToValuePtr<TValue>(Object);
            if (Test.TestNotNull(FString::Printf(TEXT("Property %s should be writable"), PropertyName), ValuePtr))
            {
                *ValuePtr = Value;
                return true;
            }
        }
        return false;
    }

    bool
    SetEnumPropertyValue(FAutomationTestBase& Test, UObject* const Object, const TCHAR* PropertyName, const int64 Value)
    {
        const auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
        if (!Test.TestNotNull(FString::Printf(TEXT("Enum property %s should exist"), PropertyName), Property))
        {
            return false;
        }

        void* const ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);
        if (const auto EnumProperty = CastField<FEnumProperty>(Property))
        {
            EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, Value);
            return true;
        }
        if (const auto ByteProperty = CastField<FByteProperty>(Property))
        {
            ByteProperty->SetIntPropertyValue(ValuePtr, Value);
            return true;
        }

        Test.AddError(FString::Printf(TEXT("Property %s should be enum-like"), PropertyName));
        return false;
    }

    bool TextArrayContainsFragment(const TArray<FText>& Messages, const FString& ExpectedFragment)
    {
        for (const auto& Message : Messages)
        {
            if (Message.ToString().Contains(ExpectedFragment))
            {
                return true;
            }
        }
        return false;
    }

    bool TestTextArrayContains(FAutomationTestBase& Test,
                               const TArray<FText>& Messages,
                               const TCHAR* Description,
                               const TCHAR* ExpectedFragment)
    {
        return Test.TestTrue(Description, TextArrayContainsFragment(Messages, ExpectedFragment));
    }

    const TCHAR* GetRuleRangerTestMountRoot()
    {
        return TEXT("/Game/Developers/Tests/RuleRanger");
    }

    FString NewUniquePackageName(const TCHAR* const LeafName)
    {
        return FString::Printf(TEXT("%s/MetaSound/%s_%s"),
                               GetRuleRangerTestMountRoot(),
                               LeafName,
                               *FGuid::NewGuid().ToString(EGuidFormats::Digits));
    }

    bool IsRuleRangerTestPackagePath(const FString& PackagePath)
    {
        return PackagePath.StartsWith(GetRuleRangerTestMountRoot(), ESearchCase::CaseSensitive);
    }

    struct FCreatedTestDirectory
    {
        FString MountPath;
        FString FileSystemPath;
    };

    class FTestAssetCleanupRegistrar
    {
    public:
        FTestAssetCleanupRegistrar()
        {
            auto& Framework = FAutomationTestFramework::Get();
            OnTestEndHandle = Framework.OnTestEndEvent.AddRaw(this, &FTestAssetCleanupRegistrar::HandleTestEnd);
        }

        ~FTestAssetCleanupRegistrar()
        {
            auto& Framework = FAutomationTestFramework::Get();
            Framework.OnTestEndEvent.Remove(OnTestEndHandle);
            CleanupRegisteredDirectories();
        }

        void RegisterDirectory(const FString& MountPath, const FString& FileSystemPath)
        {
            for (const auto& Directory : CreatedDirectories)
            {
                if (Directory.MountPath.Equals(MountPath, ESearchCase::CaseSensitive))
                {
                    return;
                }
            }
            CreatedDirectories.Add({ MountPath, FileSystemPath });
        }

    private:
        void CleanupRegisteredDirectories()
        {
            CreatedDirectories.Sort([](const FCreatedTestDirectory& Left, const FCreatedTestDirectory& Right) {
                return Left.FileSystemPath.Len() > Right.FileSystemPath.Len();
            });

            for (const auto& Directory : CreatedDirectories)
            {
                IFileManager::Get().DeleteDirectory(*Directory.FileSystemPath, false, true);
            }
            CreatedDirectories.Reset();
        }

        void HandleTestEnd(FAutomationTestBase*) { CleanupRegisteredDirectories(); }

        FDelegateHandle OnTestEndHandle;
        TArray<FCreatedTestDirectory> CreatedDirectories;
    };

    FTestAssetCleanupRegistrar& EnsureTestAssetCleanupRegistered()
    {
        static FTestAssetCleanupRegistrar Registrar;
        return Registrar;
    }

    void EnsurePackageDirectoryExists(const TCHAR* const PackageName)
    {
        if (!PackageName)
        {
            return;
        }

        const FString PackageNameString(PackageName);
        if (IsRuleRangerTestPackagePath(PackageNameString))
        {
            constexpr TCHAR GameRoot[] = TEXT("/Game/");
            const auto RelativePath = PackageNameString.RightChop(UE_ARRAY_COUNT(GameRoot) - 1);
            const auto DirectoryPath = FPaths::GetPath(FPaths::Combine(FPaths::ProjectContentDir(), RelativePath));
            if (!DirectoryPath.IsEmpty())
            {
                IFileManager::Get().MakeDirectory(*DirectoryPath, true);
                EnsureTestAssetCleanupRegistered().RegisterDirectory(FPaths::GetPath(PackageNameString), DirectoryPath);
            }
        }
    }

    UPackage* NewTestPackage(const TCHAR* const PackageName)
    {
        EnsurePackageDirectoryExists(PackageName);
        return CreatePackage(PackageName);
    }

    template <typename TObject>
    TObject* NewRegisteredPackagedAsset(const FString& PackageName, const TCHAR* const ObjectName)
    {
        const auto Package = NewTestPackage(*PackageName);
        const auto Object =
            Package ? NewObject<TObject>(Package, FName(ObjectName), RF_Public | RF_Standalone) : nullptr;
        if (Object)
        {
            FAssetRegistryModule::AssetCreated(Object);
            Package->ClearDirtyFlag();
        }
        return Object;
    }

    bool SaveAssetAndRefreshAssetRegistry(FAutomationTestBase& Test, UObject* const Asset)
    {
        const auto Package = Asset ? Asset->GetPackage() : nullptr;
        if (!Test.TestNotNull(TEXT("Asset package should exist"), Package))
        {
            return false;
        }

        const FString Filename =
            FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
        FSavePackageArgs SaveArgs;
        SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
        SaveArgs.SaveFlags = SAVE_NoError;
        if (Asset->IsA<UMetaSoundSource>())
        {
            Test.AddExpectedMessagePlain(TEXT("PreSaveAsset for MetaSound"),
                                         ELogVerbosity::Warning,
                                         EAutomationExpectedMessageFlags::Contains,
                                         1);
        }
        if (!Test.TestTrue(TEXT("Asset package should save cleanly"),
                           UPackage::SavePackage(Package, Asset, *Filename, SaveArgs)))
        {
            return false;
        }

        UPackage::WaitForAsyncFileWrites();

        auto& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
        AssetRegistry.ScanModifiedAssetFiles({ Filename });
        AssetRegistry.WaitForCompletion();
        return true;
    }

    URuleRangerActionContext*
    NewActionContext(FAutomationTestBase& Test,
                     const ERuleRangerActionTrigger Trigger = ERuleRangerActionTrigger::AT_Report)
    {
        const auto Context = NewTransientObject<URuleRangerActionContext>();
        if (Test.TestNotNull(TEXT("Action context should be created"), Context)
            && SetEnumPropertyValue(Test,
                                    Context,
                                    TEXT("ActionState"),
                                    static_cast<int64>(ERuleRangerActionState::AS_Success))
            && SetEnumPropertyValue(Test, Context, TEXT("ActionTrigger"), static_cast<int64>(Trigger)))
        {
            return Context;
        }
        return nullptr;
    }

    void SetAuthor(UMetaSoundSource* const Source, const FString& Author)
    {
        Source->GetDocumentChecked().RootGraph.Metadata.SetAuthor(Author);
    }

    void SetPreset(UMetaSoundSource* const Source, const bool bIsPreset)
    {
        auto& Document = Source->GetDocumentChecked();
        if (bIsPreset)
        {
            Document.Template = TInstancedStruct<FMetaSoundFrontendPresetTemplate>::Make();
        }
        else
        {
            Document.Template.Reset();
        }

        PRAGMA_DISABLE_DEPRECATION_WARNINGS
        Source->GetDocumentChecked().RootGraph.PresetOptions.bIsPreset = bIsPreset;
        PRAGMA_ENABLE_DEPRECATION_WARNINGS
    }

    UMetaSoundSource* NewTransientMetaSoundSource(const TCHAR* const Name,
                                                  const bool bIsPreset = false,
                                                  const TCHAR* const Author = nullptr)
    {
        const auto Source = NewNamedTransientObject<UMetaSoundSource>(Name);
        if (Source)
        {
            SetPreset(Source, bIsPreset);
            SetAuthor(Source, Author ? FString(Author) : FString());
        }
        return Source;
    }

    UMetaSoundSource*
    NewPackagedMetaSoundSource(const FString& PackageName, const TCHAR* const ObjectName, const bool bIsPreset = false)
    {
        const auto Source = NewRegisteredPackagedAsset<UMetaSoundSource>(PackageName, ObjectName);
        if (Source)
        {
            SetPreset(Source, bIsPreset);
        }
        return Source;
    }

    bool
    SetReference(FAutomationTestBase& Test, URuleRangerMetaSoundReferenceObject* const Object, UObject* const Reference)
    {
        return SetPropertyValue(Test, Object, TEXT("Reference"), Reference);
    }

    bool SetAllowList(FAutomationTestBase& Test,
                      UEnsureNoMetaSoundSourceReferenceAction* const Action,
                      const TArray<FSoftObjectPath>& AllowList)
    {
        return SetPropertyValue(Test, Action, TEXT("AllowList"), AllowList);
    }

    URuleRangerMetaSoundReferenceObject* NewSavedReferencingObject(FAutomationTestBase& Test,
                                                                   const FString& PackageName,
                                                                   const TCHAR* const ObjectName,
                                                                   UObject* const Reference)
    {
        const auto Object = NewRegisteredPackagedAsset<URuleRangerMetaSoundReferenceObject>(PackageName, ObjectName);
        if (Test.TestNotNull(TEXT("Referencing object should be created"), Object)
            && SetReference(Test, Object, Reference) && SaveAssetAndRefreshAssetRegistry(Test, Object))
        {
            return Object;
        }
        return nullptr;
    }
} // namespace RuleRangerMetaSoundTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerMetaSoundPresetMatcherAcceptsPresetSourceTest,
                                 "RuleRanger.Matchers.MetaSound.Preset.AcceptsPresetSource",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerMetaSoundPresetMatcherAcceptsPresetSourceTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerMetaSoundTests::NewTransientObject<UMetasoundPresetMatcher>();
    const auto Source = RuleRangerMetaSoundTests::NewTransientMetaSoundSource(TEXT("PresetSource"), true);
    return TestNotNull(TEXT("MetaSound preset matcher should be created"), Matcher)
        && TestNotNull(TEXT("Preset MetaSound source should be created"), Source)
        && TestTrue(TEXT("Preset MetaSound sources should match"), Matcher->Test(Source));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerMetaSoundPresetMatcherRejectsNonPresetSourceTest,
                                 "RuleRanger.Matchers.MetaSound.Preset.RejectsNonPresetSource",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerMetaSoundPresetMatcherRejectsNonPresetSourceTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerMetaSoundTests::NewTransientObject<UMetasoundPresetMatcher>();
    const auto Source = RuleRangerMetaSoundTests::NewTransientMetaSoundSource(TEXT("NonPresetSource"), false);
    return TestNotNull(TEXT("MetaSound preset matcher should be created"), Matcher)
        && TestNotNull(TEXT("Non-preset MetaSound source should be created"), Source)
        && TestFalse(TEXT("Non-preset MetaSound sources should not match"), Matcher->Test(Source));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerMetaSoundPresetMatcherRejectsNullAndWrongTypeTest,
                                 "RuleRanger.Matchers.MetaSound.Preset.RejectsNullAndWrongType",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerMetaSoundPresetMatcherRejectsNullAndWrongTypeTest::RunTest(const FString&)
{
    const auto Matcher = RuleRangerMetaSoundTests::NewTransientObject<UMetasoundPresetMatcher>();
    const auto Object = RuleRangerMetaSoundTests::NewTransientObject<URuleRangerMetaSoundReferenceObject>();
    return TestNotNull(TEXT("MetaSound preset matcher should be created"), Matcher)
        && TestNotNull(TEXT("Wrong-type object should be created"), Object)
        && TestFalse(TEXT("Null objects should not match"), Matcher->Test(nullptr))
        && TestFalse(TEXT("Non-MetaSound objects should not match"), Matcher->Test(Object));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMetaSoundAuthorBlankAllowsBlankAuthorTest,
                                 "RuleRanger.Actions.MetaSound.EnsureAuthorBlank.AllowsBlankAuthor",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerEnsureMetaSoundAuthorBlankAllowsBlankAuthorTest::RunTest(const FString&)
{
    const auto Action = RuleRangerMetaSoundTests::NewTransientObject<UEnsureMetaSoundAuthorBlankAction>();
    const auto Source = RuleRangerMetaSoundTests::NewTransientMetaSoundSource(TEXT("BlankAuthorSource"));
    const auto Context = RuleRangerMetaSoundTests::NewActionContext(*this);
    if (TestNotNull(TEXT("EnsureMetaSoundAuthorBlankAction should be created"), Action)
        && TestNotNull(TEXT("Blank-author MetaSound source should be created"), Source)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        static_cast<URuleRangerAction*>(Action)->Apply(Context, Source);
        return TestEqual(TEXT("Blank author should not add errors"), Context->GetErrorMessages().Num(), 0)
            && TestEqual(TEXT("The action context should remain successful"),
                         Context->GetState(),
                         ERuleRangerActionState::AS_Success);
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMetaSoundAuthorBlankReportsNonBlankAuthorInDryRunTest,
                                 "RuleRanger.Actions.MetaSound.EnsureAuthorBlank.ReportsNonBlankAuthorInDryRun",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerEnsureMetaSoundAuthorBlankReportsNonBlankAuthorInDryRunTest::RunTest(const FString&)
{
    const auto Action = RuleRangerMetaSoundTests::NewTransientObject<UEnsureMetaSoundAuthorBlankAction>();
    const auto Source =
        RuleRangerMetaSoundTests::NewTransientMetaSoundSource(TEXT("DryRunAuthorSource"), false, TEXT("Composer"));
    const auto Context = RuleRangerMetaSoundTests::NewActionContext(*this);
    if (TestNotNull(TEXT("EnsureMetaSoundAuthorBlankAction should be created"), Action)
        && TestNotNull(TEXT("Non-blank-author MetaSound source should be created"), Source)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        static_cast<URuleRangerAction*>(Action)->Apply(Context, Source);
        return TestEqual(TEXT("Non-blank author should add one dry-run error"), Context->GetErrorMessages().Num(), 1)
            && RuleRangerMetaSoundTests::TestTextArrayContains(*this,
                                                               Context->GetErrorMessages(),
                                                               TEXT("The dry-run diagnostic should mention Author"),
                                                               TEXT("Author property would be cleared"))
            && TestEqual(TEXT("Dry-run should keep the source author"),
                         Source->GetConstDocument().RootGraph.Metadata.GetAuthor(),
                         FString(TEXT("Composer")));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMetaSoundAuthorBlankClearsAuthorWhenFixingTest,
                                 "RuleRanger.Actions.MetaSound.EnsureAuthorBlank.ClearsAuthorWhenFixing",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerEnsureMetaSoundAuthorBlankClearsAuthorWhenFixingTest::RunTest(const FString&)
{
    const auto Action = RuleRangerMetaSoundTests::NewTransientObject<UEnsureMetaSoundAuthorBlankAction>();
    const auto Source =
        RuleRangerMetaSoundTests::NewTransientMetaSoundSource(TEXT("FixAuthorSource"), false, TEXT("Composer"));
    const auto Context = RuleRangerMetaSoundTests::NewActionContext(*this, ERuleRangerActionTrigger::AT_Fix);
    if (TestNotNull(TEXT("EnsureMetaSoundAuthorBlankAction should be created"), Action)
        && TestNotNull(TEXT("Non-blank-author MetaSound source should be created"), Source)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        static_cast<URuleRangerAction*>(Action)->Apply(Context, Source);
        return TestEqual(TEXT("Fixing should not add errors"), Context->GetErrorMessages().Num(), 0)
            && TestEqual(TEXT("Fixing should add one info message"), Context->GetInfoMessages().Num(), 1)
            && TestEqual(TEXT("Fixing should clear the source author"),
                         Source->GetConstDocument().RootGraph.Metadata.GetAuthor(),
                         FString());
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureMetaSoundAuthorBlankIgnoresWrongTypeTest,
                                 "RuleRanger.Actions.MetaSound.EnsureAuthorBlank.IgnoresWrongType",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerEnsureMetaSoundAuthorBlankIgnoresWrongTypeTest::RunTest(const FString&)
{
    const auto Action = RuleRangerMetaSoundTests::NewTransientObject<UEnsureMetaSoundAuthorBlankAction>();
    const auto Object = RuleRangerMetaSoundTests::NewTransientObject<URuleRangerMetaSoundReferenceObject>();
    const auto Context = RuleRangerMetaSoundTests::NewActionContext(*this);
    if (TestNotNull(TEXT("EnsureMetaSoundAuthorBlankAction should be created"), Action)
        && TestNotNull(TEXT("Wrong-type object should be created"), Object)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        static_cast<URuleRangerAction*>(Action)->Apply(Context, Object);
        return TestEqual(TEXT("Wrong-type direct invocation should not add errors"),
                         Context->GetErrorMessages().Num(),
                         0);
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNoMetaSoundSourceReferenceAllowsCleanObjectTest,
                                 "RuleRanger.Actions.MetaSound.EnsureNoSourceReference.AllowsCleanObject",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerEnsureNoMetaSoundSourceReferenceAllowsCleanObjectTest::RunTest(const FString&)
{
    const auto Action = RuleRangerMetaSoundTests::NewTransientObject<UEnsureNoMetaSoundSourceReferenceAction>();
    const auto Object = RuleRangerMetaSoundTests::NewTransientObject<URuleRangerMetaSoundReferenceObject>();
    const auto Context = RuleRangerMetaSoundTests::NewActionContext(*this);
    if (TestNotNull(TEXT("EnsureNoMetaSoundSourceReferenceAction should be created"), Action)
        && TestNotNull(TEXT("Clean object should be created"), Object)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        static_cast<URuleRangerAction*>(Action)->Apply(Context, Object);
        return TestEqual(TEXT("Clean objects should not add errors"), Context->GetErrorMessages().Num(), 0)
            && TestEqual(TEXT("The action context should remain successful"),
                         Context->GetState(),
                         ERuleRangerActionState::AS_Success);
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNoMetaSoundSourceReferenceReportsForbiddenDependencyTest,
                                 "RuleRanger.Actions.MetaSound.EnsureNoSourceReference.ReportsForbiddenDependency",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerEnsureNoMetaSoundSourceReferenceReportsForbiddenDependencyTest::RunTest(const FString&)
{
    using namespace RuleRangerMetaSoundTests;

    const auto Source =
        NewPackagedMetaSoundSource(NewUniquePackageName(TEXT("ForbiddenSource")), TEXT("ForbiddenSource"), false);
    const auto Action = NewTransientObject<UEnsureNoMetaSoundSourceReferenceAction>();
    const auto Context = NewActionContext(*this);
    if (TestNotNull(TEXT("MetaSound source should be created"), Source)
        && TestNotNull(TEXT("EnsureNoMetaSoundSourceReferenceAction should be created"), Action)
        && TestNotNull(TEXT("Action context should be created"), Context)
        && SaveAssetAndRefreshAssetRegistry(*this, Source))
    {
        const auto Owner = NewSavedReferencingObject(*this,
                                                     NewUniquePackageName(TEXT("ForbiddenOwner")),
                                                     TEXT("ForbiddenOwner"),
                                                     Source);
        if (!TestNotNull(TEXT("Referencing object should be created"), Owner))
        {
            return false;
        }

        static_cast<URuleRangerAction*>(Action)->Apply(Context, Owner);
        return TestEqual(TEXT("Forbidden MetaSound source dependency should add one error"),
                         Context->GetErrorMessages().Num(),
                         1)
            && TestTextArrayContains(*this,
                                     Context->GetErrorMessages(),
                                     TEXT("The dependency diagnostic should mention direct references"),
                                     TEXT("directly references MetaSoundSource"));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNoMetaSoundSourceReferenceAllowsPresetDependencyTest,
                                 "RuleRanger.Actions.MetaSound.EnsureNoSourceReference.AllowsPresetDependency",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerEnsureNoMetaSoundSourceReferenceAllowsPresetDependencyTest::RunTest(const FString&)
{
    using namespace RuleRangerMetaSoundTests;

    const auto Source =
        NewPackagedMetaSoundSource(NewUniquePackageName(TEXT("PresetSource")), TEXT("PresetSource"), true);
    const auto Action = NewTransientObject<UEnsureNoMetaSoundSourceReferenceAction>();
    const auto Context = NewActionContext(*this);
    if (TestNotNull(TEXT("Preset MetaSound source should be created"), Source)
        && TestNotNull(TEXT("EnsureNoMetaSoundSourceReferenceAction should be created"), Action)
        && TestNotNull(TEXT("Action context should be created"), Context)
        && SaveAssetAndRefreshAssetRegistry(*this, Source))
    {
        const auto Owner =
            NewSavedReferencingObject(*this, NewUniquePackageName(TEXT("PresetOwner")), TEXT("PresetOwner"), Source);
        if (!TestNotNull(TEXT("Referencing object should be created"), Owner))
        {
            return false;
        }

        static_cast<URuleRangerAction*>(Action)->Apply(Context, Owner);
        return TestEqual(TEXT("Preset MetaSound source dependency should not add errors"),
                         Context->GetErrorMessages().Num(),
                         0);
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNoMetaSoundSourceReferenceAllowsAllowListedDependencyTest,
                                 "RuleRanger.Actions.MetaSound.EnsureNoSourceReference.AllowsAllowListedDependency",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerEnsureNoMetaSoundSourceReferenceAllowsAllowListedDependencyTest::RunTest(const FString&)
{
    using namespace RuleRangerMetaSoundTests;

    const auto Source =
        NewPackagedMetaSoundSource(NewUniquePackageName(TEXT("AllowListedSource")), TEXT("AllowListedSource"), false);
    const auto Action = NewTransientObject<UEnsureNoMetaSoundSourceReferenceAction>();
    const auto Context = NewActionContext(*this);
    if (TestNotNull(TEXT("Allow-listed MetaSound source should be created"), Source)
        && TestNotNull(TEXT("EnsureNoMetaSoundSourceReferenceAction should be created"), Action)
        && TestNotNull(TEXT("Action context should be created"), Context)
        && SaveAssetAndRefreshAssetRegistry(*this, Source))
    {
        const auto Owner = NewSavedReferencingObject(*this,
                                                     NewUniquePackageName(TEXT("AllowListedOwner")),
                                                     TEXT("AllowListedOwner"),
                                                     Source);
        if (!TestNotNull(TEXT("Referencing object should be created"), Owner)
            || !SetAllowList(*this, Action, { FSoftObjectPath(Source) }))
        {
            return false;
        }

        static_cast<URuleRangerAction*>(Action)->Apply(Context, Owner);
        return TestEqual(TEXT("Allow-listed MetaSound source dependency should not add errors"),
                         Context->GetErrorMessages().Num(),
                         0);
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNoMetaSoundSourceReferenceReportsForbiddenReferencerTest,
                                 "RuleRanger.Actions.MetaSound.EnsureNoSourceReference.ReportsForbiddenReferencer",
                                 RuleRangerMetaSoundTests::AutomationTestFlags)
bool FRuleRangerEnsureNoMetaSoundSourceReferenceReportsForbiddenReferencerTest::RunTest(const FString&)
{
    using namespace RuleRangerMetaSoundTests;

    const auto Source =
        NewPackagedMetaSoundSource(NewUniquePackageName(TEXT("ReferencerSource")), TEXT("ReferencerSource"), false);
    const auto Action = NewTransientObject<UEnsureNoMetaSoundSourceReferenceAction>();
    const auto Context = NewActionContext(*this);
    if (TestNotNull(TEXT("MetaSound source should be created"), Source)
        && TestNotNull(TEXT("EnsureNoMetaSoundSourceReferenceAction should be created"), Action)
        && TestNotNull(TEXT("Action context should be created"), Context)
        && SaveAssetAndRefreshAssetRegistry(*this, Source))
    {
        const auto Owner = NewSavedReferencingObject(*this,
                                                     NewUniquePackageName(TEXT("ReferencerOwner")),
                                                     TEXT("ReferencerOwner"),
                                                     Source);
        if (!TestNotNull(TEXT("Referencing object should be created"), Owner))
        {
            return false;
        }

        static_cast<URuleRangerAction*>(Action)->Apply(Context, Source);
        return TestEqual(TEXT("Forbidden MetaSound source referencer should add one error"),
                         Context->GetErrorMessages().Num(),
                         1)
            && TestTextArrayContains(*this,
                                     Context->GetErrorMessages(),
                                     TEXT("The referencer diagnostic should mention direct references"),
                                     TEXT("directly references MetaSoundSource"));
    }
    return false;
}

#endif
