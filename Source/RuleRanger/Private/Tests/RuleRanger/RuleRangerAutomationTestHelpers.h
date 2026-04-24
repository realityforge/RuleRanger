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
#pragma once

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "AssetRegistry/AssetRegistryModule.h"
    #include "EdGraph/EdGraph.h"
    #include "EdGraphSchema_K2.h"
    #include "Editor.h"
    #include "Editor/EditorPerProjectUserSettings.h"
    #include "EditorFramework/AssetImportData.h"
    #include "Engine/BlueprintGeneratedClass.h"
    #include "Engine/DataTable.h"
    #include "Engine/Texture2D.h"
    #include "HAL/FileManager.h"
    #include "K2Node_Event.h"
    #include "K2Node_FunctionEntry.h"
    #include "K2Node_FunctionResult.h"
    #include "Kismet2/BlueprintEditorUtils.h"
    #include "Kismet2/KismetEditorUtilities.h"
    #include "Materials/Material.h"
    #include "Misc/AutomationTest.h"
    #include "Misc/DataValidation.h"
    #include "Misc/Paths.h"
    #include "RuleRanger/Actions/Blueprint/EnsureDataOnlyBlueprintAction.h"
    #include "RuleRanger/RuleRangerUtilities.h"
    #include "RuleRangerActionContext.h"
    #include "RuleRangerConfig.h"
    #include "RuleRangerProjectActionContext.h"
    #include "RuleRangerProjectRule.h"
    #include "RuleRangerRule.h"
    #include "RuleRangerRuleSet.h"
    #include "Sound/SoundWave.h"
    #include "Subsystems/EditorAssetSubsystem.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"
    #include "UObject/MetaData.h"
    #include "UObject/ObjectSaveContext.h"
    #include "UObject/Package.h"
    #include "UObject/UnrealType.h"

class FRuleRangerActionContextTestAccessor
{
public:
    static void ResetContext(URuleRangerActionContext* const Context,
                             URuleRangerConfig* const Config,
                             URuleRangerRuleSet* const RuleSet,
                             URuleRangerRule* const Rule,
                             UObject* const Object,
                             const ERuleRangerActionTrigger Trigger)
    {
        Context->ResetContext(Config, RuleSet, Rule, Object, Trigger);
    }

    static void ClearContext(URuleRangerActionContext* const Context) { Context->ClearContext(); }
};

class FRuleRangerProjectActionContextTestAccessor
{
public:
    static void ResetContext(URuleRangerProjectActionContext* const Context,
                             URuleRangerConfig* const Config,
                             URuleRangerRuleSet* const RuleSet,
                             URuleRangerProjectRule* const Rule,
                             const ERuleRangerProjectActionTrigger Trigger)
    {
        Context->ResetContext(Config, RuleSet, Rule, Trigger);
    }

    static void ClearContext(URuleRangerProjectActionContext* const Context) { Context->ClearContext(); }
};

namespace RuleRangerTests
{
    constexpr auto AutomationTestFlags =
        EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

    template <typename TObject>
    TObject* NewTransientObject(UObject* Outer = GetTransientPackage(), const FName Name = NAME_None)
    {
        return NewObject<TObject>(Outer, Name, RF_Transient);
    }

    template <typename TObject>
    TObject* NewNamedTransientObject(const TCHAR* const Name, UObject* Outer = GetTransientPackage())
    {
        return NewTransientObject<TObject>(Outer, FName(Name));
    }

    template <typename TObject, typename TValue>
    bool SetPropertyValue(FAutomationTestBase& Test,
                          TObject* Object,
                          const TCHAR* PropertyName,
                          const TValue& Value,
                          const int32 ArrayIndex)
    {
        const auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
        if (Test.TestNotNull(FString::Printf(TEXT("Property %s should exist"), PropertyName), Property))
        {
            auto ValuePtr = Property->template ContainerPtrToValuePtr<TValue>(Object, ArrayIndex);
            if (Test.TestNotNull(FString::Printf(TEXT("Property %s should be writable"), PropertyName), ValuePtr))
            {
                *ValuePtr = Value;
                return true;
            }
        }

        return false;
    }

    template <typename TObject, typename TValue>
    bool SetPropertyValue(FAutomationTestBase& Test, TObject* Object, const TCHAR* PropertyName, const TValue& Value)
    {
        return SetPropertyValue(Test, Object, PropertyName, Value, 0);
    }

    template <typename TObject>
    bool SetBoolPropertyValue(FAutomationTestBase& Test,
                              TObject* Object,
                              const TCHAR* PropertyName,
                              const bool Value,
                              const int32 ArrayIndex = 0)
    {
        const auto Property = FindFProperty<FBoolProperty>(Object->GetClass(), PropertyName);
        if (Test.TestNotNull(FString::Printf(TEXT("Bool property %s should exist"), PropertyName), Property))
        {
            Property->SetPropertyValue_InContainer(Object, Value, ArrayIndex);
            return true;
        }
        else
        {
            return false;
        }
    }

    inline FDataValidationContext CreateValidationContext()
    {
        return FDataValidationContext(true, EDataValidationUsecase::Manual, TConstArrayView<FAssetData>{});
    }

    inline bool TextArrayContainsFragment(const TArray<FText>& Messages, const FString& ExpectedFragment)
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

    inline bool ValidationContextContainsIssue(const FDataValidationContext& Context, const FString& ExpectedFragment)
    {
        for (const auto& Issue : Context.GetIssues())
        {
            if (Issue.Message.ToString().Contains(ExpectedFragment))
            {
                return true;
            }
        }

        return false;
    }

    inline bool TestTextArrayContains(FAutomationTestBase& Test,
                                      const TArray<FText>& Messages,
                                      const TCHAR* Description,
                                      const TCHAR* ExpectedFragment)
    {
        return Test.TestTrue(Description, TextArrayContainsFragment(Messages, ExpectedFragment));
    }

    inline bool TestValidation(FAutomationTestBase& Test,
                               const UObject* Object,
                               const EDataValidationResult ExpectedResult,
                               const TCHAR* ExpectedIssueFragment = nullptr)
    {
        auto Context = CreateValidationContext();
        const auto ActualResult = Object->IsDataValid(Context);
        const auto bResultMatches =
            Test.TestEqual(TEXT("Validation result should match expectation"), ActualResult, ExpectedResult);

        if (ExpectedIssueFragment)
        {
            return Test.TestTrue(FString::Printf(TEXT("Validation issues should contain '%s'"), ExpectedIssueFragment),
                                 ValidationContextContainsIssue(Context, ExpectedIssueFragment))
                && bResultMatches;
        }
        else
        {
            return bResultMatches;
        }
    }

    template <typename TObject>
    TObject* GetClassDefaultObject()
    {
        return TObject::StaticClass()->template GetDefaultObject<TObject>();
    }

    inline FString GetRuleRangerTestContentRoot()
    {
        return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Developers/Tests/RuleRanger"));
    }

    inline const TCHAR* GetRuleRangerTestMountRoot()
    {
        return TEXT("/Game/Developers/Tests/RuleRanger");
    }

    inline bool IsRuleRangerTestPackagePath(const FString& PackagePath)
    {
        return PackagePath.StartsWith(GetRuleRangerTestMountRoot(), ESearchCase::CaseSensitive);
    }

    struct FRuleRangerCreatedTestDirectory
    {
        FString MountPath;
        FString FileSystemPath;
    };

    class FRuleRangerTestAssetCleanupRegistrar
    {
    public:
        FRuleRangerTestAssetCleanupRegistrar()
        {
            auto& Framework = FAutomationTestFramework::Get();
            OnTestEndHandle =
                Framework.OnTestEndEvent.AddRaw(this, &FRuleRangerTestAssetCleanupRegistrar::HandleTestEnd);
        }

        ~FRuleRangerTestAssetCleanupRegistrar()
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
            CreatedDirectories.Sort(
                [](const FRuleRangerCreatedTestDirectory& Left, const FRuleRangerCreatedTestDirectory& Right) {
                    return Left.FileSystemPath.Len() > Right.FileSystemPath.Len();
                });

            if (nullptr != GEditor)
            {
                if (auto* const Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>())
                {
                    const auto& AssetRegistry =
                        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
                    for (const auto& Directory : CreatedDirectories)
                    {
                        TArray<FAssetData> Assets;
                        if (AssetRegistry.GetAssetsByPath(FName(*Directory.MountPath), Assets, true, false))
                        {
                            TArray<FAssetData> RepresentativeAssets;
                            FRuleRangerUtilities::AddPackageRepresentativeAssets(Assets, RepresentativeAssets);
                            for (const auto& Asset : RepresentativeAssets)
                            {
                                Subsystem->DeleteAsset(Asset.GetSoftObjectPath().ToString());
                            }
                        }
                    }
                }
            }

            for (const auto& Directory : CreatedDirectories)
            {
                IFileManager::Get().DeleteDirectory(*Directory.FileSystemPath, false, true);
            }

            CreatedDirectories.Reset();
        }

        void HandleTestEnd(FAutomationTestBase*) { CleanupRegisteredDirectories(); }

        FDelegateHandle OnTestEndHandle;
        TArray<FRuleRangerCreatedTestDirectory> CreatedDirectories;
    };

    inline FRuleRangerTestAssetCleanupRegistrar& EnsureTestAssetCleanupRegistered()
    {
        static FRuleRangerTestAssetCleanupRegistrar Registrar;
        return Registrar;
    }

    inline void EnsurePackageDirectoryExists(const TCHAR* const PackageName)
    {
        if (nullptr != PackageName)
        {
            const FString PackageNameString(PackageName);
            if (IsRuleRangerTestPackagePath(PackageNameString))
            {
                constexpr TCHAR GameRoot[] = TEXT("/Game/");
                const auto RelativePath = PackageNameString.RightChop(UE_ARRAY_COUNT(GameRoot) - 1);
                const auto DirectoryPath = FPaths::GetPath(FPaths::Combine(FPaths::ProjectContentDir(), RelativePath));
                if (!DirectoryPath.IsEmpty())
                {
                    IFileManager::Get().MakeDirectory(*DirectoryPath, true);
                    EnsureTestAssetCleanupRegistered().RegisterDirectory(FPaths::GetPath(PackageNameString),
                                                                         DirectoryPath);
                }
            }
        }
    }

    inline UPackage* NewTransientPackage(const TCHAR* const PackageName)
    {
        EnsurePackageDirectoryExists(PackageName);
        const auto Package = CreatePackage(PackageName);
        if (Package)
        {
            Package->SetFlags(RF_Transient);
        }

        return Package;
    }

    inline UPackage* NewTestPackage(const TCHAR* const PackageName)
    {
        EnsurePackageDirectoryExists(PackageName);
        return CreatePackage(PackageName);
    }

    inline void RegisterAsset(UObject* const Object)
    {
        if (Object)
        {
            FAssetRegistryModule::AssetCreated(Object);
        }
    }

    inline bool ClearPackageDirtyFlag(UObject* const Object)
    {
        if (const auto Package = Object ? Object->GetPackage() : nullptr)
        {
            Package->ClearDirtyFlag();
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename TObject>
    TObject* NewPackagedObject(const TCHAR* const PackageName, const TCHAR* const ObjectName)
    {
        const auto Package = NewTransientPackage(PackageName);
        return Package ? NewTransientObject<TObject>(Package, FName(ObjectName)) : nullptr;
    }

    template <typename TObject>
    TObject* NewRegisteredPackagedAsset(const TCHAR* const PackageName, const TCHAR* const ObjectName)
    {
        const auto Package = NewTestPackage(PackageName);
        const auto Object =
            Package ? NewObject<TObject>(Package, FName(ObjectName), RF_Public | RF_Standalone) : nullptr;
        if (Object)
        {
            RegisterAsset(Object);
            ClearPackageDirtyFlag(Object);
        }
        return Object;
    }

    inline bool TestPackageDirtyFlag(FAutomationTestBase& Test,
                                     UObject* const Object,
                                     const bool bExpectedDirty,
                                     const TCHAR* const Description)
    {
        const auto Package = Object ? Object->GetPackage() : nullptr;
        return Test.TestNotNull(TEXT("Object package should exist"), Package)
            && Test.TestEqual(Description, Package->IsDirty(), bExpectedDirty);
    }

    inline UTexture2D* NewTransientTexture2D(const int32 SizeX,
                                             const int32 SizeY,
                                             const TCHAR* const ObjectName = TEXT("RuleRangerTexture"))
    {
        return UTexture2D::CreateTransient(SizeX, SizeY, PF_B8G8R8A8, FName(ObjectName));
    }

    inline UTexture2D* NewPackagedTexture2D(const TCHAR* const PackageName,
                                            const TCHAR* const ObjectName,
                                            const int32 SizeX,
                                            const int32 SizeY)
    {
        const auto Package = NewTestPackage(PackageName);
        const auto Texture = NewTransientTexture2D(SizeX, SizeY, ObjectName);
        if (Texture && Package)
        {
            Texture->Rename(ObjectName, Package, REN_DontCreateRedirectors);
            ClearPackageDirtyFlag(Texture);
        }
        return Texture;
    }

    inline UMaterial* NewPackagedMaterial(const TCHAR* const PackageName, const TCHAR* const ObjectName)
    {
        return NewRegisteredPackagedAsset<UMaterial>(PackageName, ObjectName);
    }

    inline USoundWave* NewPackagedSoundWave(const TCHAR* const PackageName, const TCHAR* const ObjectName)
    {
        const auto SoundWave = NewRegisteredPackagedAsset<USoundWave>(PackageName, ObjectName);
        if (SoundWave)
        {
            ClearPackageDirtyFlag(SoundWave);
        }
        return SoundWave;
    }

    inline void SetSoundWaveSampleRate(USoundWave* const SoundWave, const int32 SampleRate)
    {
        SoundWave->SetSampleRate(static_cast<uint32>(SampleRate), true);
        SoundWave->SetImportedSampleRate(static_cast<uint32>(SampleRate));
    }

    inline bool SetImportFilename(FAutomationTestBase& Test,
                                  URuleRangerAutomationImportDataObject* Object,
                                  const TCHAR* const Filename)
    {
        const auto ImportData = NewTransientObject<UAssetImportData>(Object, TEXT("AssetImportData"));
        if (Test.TestNotNull(TEXT("AssetImportData should be created"), ImportData))
        {
            ImportData->UpdateFilenameOnly(FString(Filename));
            return SetPropertyValue(Test, Object, TEXT("AssetImportData"), ImportData);
        }
        else
        {
            return false;
        }
    }

    inline UBlueprint* NewBlueprint(UClass* const ParentClass,
                                    const TCHAR* const PackageName,
                                    const TCHAR* const ObjectName,
                                    const EBlueprintType BlueprintType = BPTYPE_Normal)
    {
        const auto Package = NewTestPackage(PackageName);
        return Package ? FKismetEditorUtilities::CreateBlueprint(ParentClass,
                                                                 Package,
                                                                 FName(ObjectName),
                                                                 BlueprintType,
                                                                 UBlueprint::StaticClass(),
                                                                 UBlueprintGeneratedClass::StaticClass(),
                                                                 NAME_None)
                       : nullptr;
    }

    inline void CompileBlueprint(UBlueprint* const Blueprint)
    {
        FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipGarbageCollection);
    }

    inline void MarkBlueprintModified(UBlueprint* const Blueprint)
    {
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
    }

    inline UEdGraph* FindEventGraph(UBlueprint* const Blueprint)
    {
        return FBlueprintEditorUtils::FindEventGraph(Blueprint);
    }

    inline UEdGraph* CreateFunctionGraph(UBlueprint* const Blueprint, const FName FunctionName)
    {
        const auto Graph = FBlueprintEditorUtils::CreateNewGraph(Blueprint,
                                                                 FunctionName,
                                                                 UEdGraph::StaticClass(),
                                                                 UEdGraphSchema_K2::StaticClass());
        FBlueprintEditorUtils::AddFunctionGraph<UClass>(Blueprint, Graph, true, nullptr);
        return Graph;
    }

    template <typename TNode>
    TNode* AddGraphNode(UEdGraph* const Graph, const FName Name = NAME_None)
    {
        const auto Node = NewObject<TNode>(Graph, Name);
        Graph->AddNode(Node);
        Node->CreateNewGuid();
        Node->PostPlacedNewNode();
        Node->AllocateDefaultPins();
        return Node;
    }

    inline UK2Node_FunctionEntry* GetFunctionEntry(UEdGraph* const Graph)
    {
        TArray<UK2Node_FunctionEntry*> EntryNodes;
        Graph->GetNodesOfClass(EntryNodes);
        return EntryNodes.Num() > 0 ? EntryNodes[0] : nullptr;
    }

    inline UK2Node_FunctionResult* AddFunctionResultNode(UEdGraph* const Graph, const FName Name = NAME_None)
    {
        return AddGraphNode<UK2Node_FunctionResult>(Graph, Name);
    }

    inline UK2Node_Event* AddEventNode(UEdGraph* const Graph,
                                       const FName EventName,
                                       UClass* const OwnerClass = AActor::StaticClass(),
                                       const bool bMakeGhost = false)
    {
        const auto EventNode = AddGraphNode<UK2Node_Event>(Graph, EventName);
        EventNode->EventReference.SetExternalMember(EventName, OwnerClass);
        if (bMakeGhost)
        {
            EventNode->MakeAutomaticallyPlacedGhostNode();
        }
        return EventNode;
    }

    inline void LinkPins(UEdGraphPin* const SourcePin, UEdGraphPin* const TargetPin)
    {
        if (SourcePin && TargetPin)
        {
            SourcePin->MakeLinkTo(TargetPin);
        }
    }

    inline bool AddBlueprintVariable(UBlueprint* const Blueprint,
                                     const FName VariableName,
                                     const FName PinCategory,
                                     const FText Category = FText::FromString(TEXT("Default")),
                                     const TCHAR* const Tooltip = nullptr,
                                     const uint64 PropertyFlags = CPF_Edit)
    {
        FEdGraphPinType PinType;
        PinType.PinCategory = PinCategory;
        if (!FBlueprintEditorUtils::AddMemberVariable(Blueprint, VariableName, PinType))
        {
            return false;
        }

        for (auto& Variable : Blueprint->NewVariables)
        {
            if (Variable.VarName == VariableName)
            {
                Variable.Category = Category;
                Variable.PropertyFlags = PropertyFlags;
                if (Tooltip)
                {
                    Variable.SetMetaData(FBlueprintMetadata::MD_Tooltip, Tooltip);
                }
                return true;
            }
        }

        return false;
    }

    inline bool SetBlueprintVariableMetaData(UBlueprint* const Blueprint,
                                             const FName VariableName,
                                             const FName Key,
                                             const TCHAR* const Value)
    {
        FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, VariableName, nullptr, Key, FString(Value));
        return true;
    }

    inline bool AddLocalVariable(UK2Node_FunctionEntry* const FunctionEntry,
                                 const FName VariableName,
                                 const FName PinCategory,
                                 const FText Category = FText::FromString(TEXT("Default")),
                                 const TCHAR* const Tooltip = nullptr,
                                 const uint64 PropertyFlags = CPF_Edit)
    {
        if (!FunctionEntry)
        {
            return false;
        }

        FBPVariableDescription Variable;
        Variable.VarName = VariableName;
        Variable.VarGuid = FGuid::NewGuid();
        Variable.VarType.PinCategory = PinCategory;
        Variable.FriendlyName = VariableName.ToString();
        Variable.Category = Category;
        Variable.PropertyFlags = PropertyFlags;
        if (Tooltip)
        {
            Variable.SetMetaData(FBlueprintMetadata::MD_Tooltip, Tooltip);
        }
        FunctionEntry->LocalVariables.Add(Variable);
        return true;
    }

    inline bool SetFunctionCategory(UK2Node_FunctionEntry* const FunctionEntry, const TCHAR* const Category)
    {
        if (!FunctionEntry)
        {
            return false;
        }

        FunctionEntry->MetaData.Category = FText::FromString(Category);
        return true;
    }

    inline bool SetFunctionTooltip(UK2Node_FunctionEntry* const FunctionEntry, const TCHAR* const Tooltip)
    {
        if (!FunctionEntry)
        {
            return false;
        }

        FunctionEntry->MetaData.ToolTip = FText::FromString(Tooltip);
        return true;
    }

    inline bool SetFunctionFlags(UK2Node_FunctionEntry* const FunctionEntry, const int32 Flags)
    {
        if (!FunctionEntry)
        {
            return false;
        }

        FunctionEntry->SetExtraFlags(Flags);
        return true;
    }

    inline UDataTable*
    NewDataTable(const TCHAR* const PackageName, const TCHAR* const ObjectName, UScriptStruct* RowStruct)
    {
        const auto DataTable = NewRegisteredPackagedAsset<UDataTable>(PackageName, ObjectName);
        if (DataTable)
        {
            DataTable->RowStruct = RowStruct;
        }
        return DataTable;
    }

    template <typename TRow>
    bool AddDataTableRow(UDataTable* const DataTable, const FName RowName, const TRow& Row)
    {
        if (!DataTable)
        {
            return false;
        }

        DataTable->AddRow(RowName, Row);
        return true;
    }

    inline bool SetPackageMetaData(UObject* const Object, const FName Key, const TCHAR* const Value)
    {
        if (!Object || Key.IsNone() || Value == nullptr)
        {
            return false;
        }

        if (const auto Package = Object->GetPackage())
        {
            Package->GetMetaData().SetValue(Object, Key, Value);
            return true;
        }
        else
        {
            return false;
        }
    }

    inline bool SetAssetMetaData(UObject* const Object, const FName Key, const TCHAR* const Value)
    {
        if (!Object || Key.IsNone() || Value == nullptr || GEditor == nullptr)
        {
            return false;
        }

        if (auto* Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>())
        {
            Subsystem->SetMetadataTag(Object, Key, Value);
            return true;
        }
        else
        {
            return false;
        }
    }

    inline FString GetAssetMetaData(UObject* const Object, const FName Key)
    {
        if (!Object || Key.IsNone() || GEditor == nullptr)
        {
            return FString();
        }

        if (auto* Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>())
        {
            return Subsystem->GetMetadataTag(Object, Key);
        }
        else
        {
            return FString();
        }
    }

    inline bool DeleteAssetIfExists(const TCHAR* const AssetPath, const TCHAR* const ObjectName = nullptr)
    {
        if (AssetPath == nullptr || GEditor == nullptr)
        {
            return false;
        }

        auto* const Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
        if (Subsystem == nullptr)
        {
            return false;
        }

        const FString AssetPathString(AssetPath);
        TArray<FString> CandidateObjectPaths;
        if (ObjectName != nullptr)
        {
            CandidateObjectPaths.Add(FString::Printf(TEXT("%s.%s"), AssetPath, ObjectName));
        }
        CandidateObjectPaths.Add(AssetPathString);

        const auto& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
        for (const auto& CandidateObjectPath : CandidateObjectPaths)
        {
            if (CandidateObjectPath.Contains(TEXT(".")))
            {
                if (UObject* const LoadedAsset = FindObject<UObject>(nullptr, *CandidateObjectPath))
                {
                    return Subsystem->DeleteLoadedAsset(LoadedAsset);
                }
            }

            const auto AssetData = AssetRegistry.GetAssetByObjectPath(CandidateObjectPath);
            if (AssetData.IsValid())
            {
                if (UObject* const Asset = Subsystem->LoadAsset(CandidateObjectPath))
                {
                    return Subsystem->DeleteLoadedAsset(Asset);
                }
                else
                {
                    return false;
                }
            }
        }

        return true;
    }

    inline bool AddDataOnlyBlueprintRow(UDataTable* const DataTable, const FName RowName, UClass* const ObjectType)
    {
        if (!DataTable || !ObjectType)
        {
            return false;
        }

        FDataOnlyBlueprintEntry Entry;
        Entry.ObjectType = ObjectType;
        DataTable->AddRow(RowName, Entry);
        return true;
    }

    struct FScopedDataSourceFolderOverride
    {
        UEditorPerProjectUserSettings* Settings{ nullptr };
        FString OriginalPath;

        explicit FScopedDataSourceFolderOverride(const TCHAR* const NewPath)
            : Settings(GetMutableDefault<UEditorPerProjectUserSettings>())
        {
            if (Settings)
            {
                OriginalPath = Settings->DataSourceFolder.Path;
                Settings->DataSourceFolder.Path = NewPath;
            }
        }

        ~FScopedDataSourceFolderOverride()
        {
            if (Settings)
            {
                Settings->DataSourceFolder.Path = OriginalPath;
            }
        }
    };

    struct FPreSaveContextHolder
    {
        FObjectSaveContextData Data;
        FObjectPreSaveContext Context;

        FPreSaveContextHolder() : Context(Data) {}
    };

    struct FRuleFixture
    {
        TObjectPtr<URuleRangerConfig> Config{ nullptr };
        TObjectPtr<URuleRangerRuleSet> RuleSet{ nullptr };
        TObjectPtr<URuleRangerRule> Rule{ nullptr };
        TObjectPtr<UObject> Object{ nullptr };
        TObjectPtr<URuleRangerActionContext> ActionContext{ nullptr };
    };

    inline bool CreateRuleFixture(FAutomationTestBase& Test,
                                  FRuleFixture& OutFixture,
                                  const TCHAR* const ObjectName = TEXT("RuleRangerTestObject"),
                                  const ERuleRangerActionTrigger Trigger = ERuleRangerActionTrigger::AT_Report)
    {
        OutFixture.Config = NewTransientObject<URuleRangerConfig>();
        OutFixture.RuleSet = NewTransientObject<URuleRangerRuleSet>();
        OutFixture.Rule = NewTransientObject<URuleRangerRule>();
        OutFixture.Object = NewNamedTransientObject<URuleRangerAutomationTestObject>(ObjectName);
        OutFixture.ActionContext = NewTransientObject<URuleRangerActionContext>();

        const auto bCreated = Test.TestNotNull(TEXT("Config should be created"), OutFixture.Config.Get())
            && Test.TestNotNull(TEXT("RuleSet should be created"), OutFixture.RuleSet.Get())
            && Test.TestNotNull(TEXT("Rule should be created"), OutFixture.Rule.Get())
            && Test.TestNotNull(TEXT("Object should be created"), OutFixture.Object.Get())
            && Test.TestNotNull(TEXT("ActionContext should be created"), OutFixture.ActionContext.Get());

        if (bCreated)
        {
            FRuleRangerActionContextTestAccessor::ResetContext(OutFixture.ActionContext,
                                                               OutFixture.Config,
                                                               OutFixture.RuleSet,
                                                               OutFixture.Rule,
                                                               OutFixture.Object,
                                                               Trigger);
            return true;
        }
        else
        {
            return false;
        }
    }

    inline void ResetRuleFixtureObject(FRuleFixture& Fixture,
                                       UObject* const Object,
                                       const ERuleRangerActionTrigger Trigger = ERuleRangerActionTrigger::AT_Report)
    {
        FRuleRangerActionContextTestAccessor::ResetContext(Fixture.ActionContext,
                                                           Fixture.Config,
                                                           Fixture.RuleSet,
                                                           Fixture.Rule,
                                                           Object,
                                                           Trigger);
    }

    struct FProjectRuleFixture
    {
        TObjectPtr<URuleRangerConfig> Config{ nullptr };
        TObjectPtr<URuleRangerRuleSet> RuleSet{ nullptr };
        TObjectPtr<URuleRangerProjectRule> Rule{ nullptr };
        TObjectPtr<URuleRangerProjectActionContext> ActionContext{ nullptr };
    };

    inline bool
    CreateProjectRuleFixture(FAutomationTestBase& Test,
                             FProjectRuleFixture& OutFixture,
                             const ERuleRangerProjectActionTrigger Trigger = ERuleRangerProjectActionTrigger::AT_Report)
    {
        OutFixture.Config = NewTransientObject<URuleRangerConfig>();
        OutFixture.RuleSet = NewTransientObject<URuleRangerRuleSet>();
        OutFixture.Rule = NewTransientObject<URuleRangerProjectRule>();
        OutFixture.ActionContext = NewTransientObject<URuleRangerProjectActionContext>();

        const auto bCreated = Test.TestNotNull(TEXT("Config should be created"), OutFixture.Config.Get())
            && Test.TestNotNull(TEXT("RuleSet should be created"), OutFixture.RuleSet.Get())
            && Test.TestNotNull(TEXT("Project rule should be created"), OutFixture.Rule.Get())
            && Test.TestNotNull(TEXT("Project action context should be created"), OutFixture.ActionContext.Get());

        if (bCreated)
        {
            FRuleRangerProjectActionContextTestAccessor::ResetContext(OutFixture.ActionContext,
                                                                      OutFixture.Config,
                                                                      OutFixture.RuleSet,
                                                                      OutFixture.Rule,
                                                                      Trigger);
            return true;
        }
        else
        {
            return false;
        }
    }

    inline void ResetProjectRuleFixtureContext(
        FProjectRuleFixture& Fixture,
        const ERuleRangerProjectActionTrigger Trigger = ERuleRangerProjectActionTrigger::AT_Report)
    {
        FRuleRangerProjectActionContextTestAccessor::ResetContext(Fixture.ActionContext,
                                                                  Fixture.Config,
                                                                  Fixture.RuleSet,
                                                                  Fixture.Rule,
                                                                  Trigger);
    }
} // namespace RuleRangerTests

#endif
