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

    #include "Editor/EditorPerProjectUserSettings.h"
    #include "EditorFramework/AssetImportData.h"
    #include "Engine/Texture2D.h"
    #include "Materials/Material.h"
    #include "Misc/AutomationTest.h"
    #include "Misc/DataValidation.h"
    #include "RuleRangerActionContext.h"
    #include "RuleRangerConfig.h"
    #include "RuleRangerRule.h"
    #include "RuleRangerRuleSet.h"
    #include "Sound/SoundWave.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestTypes.h"
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

    inline UPackage* NewTransientPackage(const TCHAR* const PackageName)
    {
        const auto Package = CreatePackage(PackageName);
        if (Package)
        {
            Package->SetFlags(RF_Transient);
        }

        return Package;
    }

    inline UPackage* NewTestPackage(const TCHAR* const PackageName)
    {
        return CreatePackage(PackageName);
    }

    template <typename TObject>
    TObject* NewPackagedObject(const TCHAR* const PackageName, const TCHAR* const ObjectName)
    {
        const auto Package = NewTransientPackage(PackageName);
        return Package ? NewTransientObject<TObject>(Package, FName(ObjectName)) : nullptr;
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
        const auto Package = NewTestPackage(PackageName);
        const auto Material = Package ? NewTransientObject<UMaterial>(Package, FName(ObjectName)) : nullptr;
        if (Material)
        {
            ClearPackageDirtyFlag(Material);
        }
        return Material;
    }

    inline USoundWave* NewPackagedSoundWave(const TCHAR* const PackageName, const TCHAR* const ObjectName)
    {
        const auto Package = NewTestPackage(PackageName);
        const auto SoundWave = Package ? NewTransientObject<USoundWave>(Package, FName(ObjectName)) : nullptr;
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
} // namespace RuleRangerTests

#endif
