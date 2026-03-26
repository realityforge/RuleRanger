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

#include "EditorFramework/AssetImportData.h"
#include "Misc/DataValidation.h"
#include "RuleRangerAction.h"
#include "RuleRangerCommonContext.h"
#include "RuleRangerMatcher.h"
#include "RuleRangerAutomationTestTypes.generated.h"

UENUM()
enum class ERuleRangerAutomationDisplayEnum : uint8
{
    Disabled UMETA(DisplayName = "Display Disabled"),
    Enabled UMETA(DisplayName = "Display Enabled"),
};

UENUM()
enum ERuleRangerAutomationLegacyEnum
{
    LegacyDisabled UMETA(DisplayName = "Legacy Disabled"),
    LegacyEnabled UMETA(DisplayName = "Legacy Enabled"),
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationTestCommonContext final : public URuleRangerCommonContext
{
    GENERATED_BODY()

public:
    void ResetForTest(URuleRangerConfig* const InConfig, URuleRangerRuleSet* const InRuleSet)
    {
        ResetContext(InConfig, InRuleSet);
    }

    void ClearForTest() { ClearContext(); }
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationTestObject : public UObject
{
    GENERATED_BODY()
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationDerivedTestObject final : public URuleRangerAutomationTestObject
{
    GENERATED_BODY()
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationImportDataObject final : public URuleRangerAutomationTestObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Instanced)
    TObjectPtr<UAssetImportData> AssetImportData{ nullptr };
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationEditorPropertyObject : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    bool bEditorBool{ false };

    UPROPERTY(EditAnywhere)
    int32 EditorInteger{ 0 };

    UPROPERTY(EditAnywhere)
    double EditorNumber{ 0.0 };

    UPROPERTY(EditAnywhere)
    ERuleRangerAutomationDisplayEnum EditorEnum{ ERuleRangerAutomationDisplayEnum::Disabled };

    UPROPERTY(EditAnywhere)
    TEnumAsByte<ERuleRangerAutomationLegacyEnum> EditorLegacyEnum{ LegacyDisabled };

    UPROPERTY(EditAnywhere)
    TObjectPtr<UObject> EditorReference{ nullptr };

    UPROPERTY(EditAnywhere)
    TObjectPtr<UObject> EditorReferenceArray[2]{ nullptr, nullptr };

    UPROPERTY(EditAnywhere)
    FString EditorText;
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationDerivedEditorPropertyObject final : public URuleRangerAutomationEditorPropertyObject
{
    GENERATED_BODY()
};

UENUM()
enum class ERuleRangerAutomationTestActionOutcome : uint8
{
    None UMETA(DisplayName = "None"),
    Info UMETA(DisplayName = "Info"),
    Warning UMETA(DisplayName = "Warning"),
    Error UMETA(DisplayName = "Error"),
    Fatal UMETA(DisplayName = "Fatal"),
};

UCLASS(NotBlueprintable, DisplayName = "Automation Test Action")
class URuleRangerAutomationTestAction final : public URuleRangerAction
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    TSubclassOf<UObject> ExpectedType{ UObject::StaticClass() };

    UPROPERTY(EditAnywhere)
    ERuleRangerAutomationTestActionOutcome Outcome{ ERuleRangerAutomationTestActionOutcome::None };

    UPROPERTY(EditAnywhere)
    FString Message{ TEXT("Automation test action message") };

    void ResetApplyCount() { ApplyCount = 0; }

    int32 GetApplyCount() const { return ApplyCount; }

    virtual void Apply(URuleRangerActionContext* ActionContext, UObject* Object) override
    {
        ApplyCount++;

        switch (Outcome)
        {
            case ERuleRangerAutomationTestActionOutcome::Info:
                ActionContext->Info(FText::FromString(Message));
                break;
            case ERuleRangerAutomationTestActionOutcome::Warning:
                ActionContext->Warning(FText::FromString(Message));
                break;
            case ERuleRangerAutomationTestActionOutcome::Error:
                ActionContext->Error(FText::FromString(Message));
                break;
            case ERuleRangerAutomationTestActionOutcome::Fatal:
                ActionContext->Fatal(FText::FromString(Message));
                break;
            case ERuleRangerAutomationTestActionOutcome::None:
            default:
                break;
        }
    }

    virtual UClass* GetExpectedType() const override { return ExpectedType.Get(); }

private:
    int32 ApplyCount{ 0 };
};

UCLASS(NotBlueprintable, DisplayName = "Automation Test Matcher")
class URuleRangerAutomationTestMatcher final : public URuleRangerMatcher
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    bool bResult{ true };

    void ResetCallCount() const { CallCount = 0; }

    int32 GetCallCount() const { return CallCount; }

    virtual bool Test(UObject* Object) const override
    {
        CallCount++;
        return bResult;
    }

private:
    mutable int32 CallCount{ 0 };
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationTestValidationObject final : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    EDataValidationResult ValidationResult{ EDataValidationResult::Valid };

    UPROPERTY(EditAnywhere)
    bool bEmitInfo{ false };

    UPROPERTY(EditAnywhere)
    bool bEmitWarning{ false };

    UPROPERTY(EditAnywhere)
    bool bEmitPerformanceWarning{ false };

    UPROPERTY(EditAnywhere)
    bool bEmitError{ false };

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override
    {
        if (bEmitInfo)
        {
            Context.AddMessage(EMessageSeverity::Info, FText::FromString(TEXT("Validation info")));
        }
        if (bEmitWarning)
        {
            Context.AddWarning(FText::FromString(TEXT("Validation warning")));
        }
        if (bEmitPerformanceWarning)
        {
            Context.AddMessage(EMessageSeverity::PerformanceWarning, FText::FromString(TEXT("Validation performance")));
        }
        if (bEmitError)
        {
            Context.AddError(FText::FromString(TEXT("Validation error")));
        }

        return ValidationResult;
    }
};

#endif

UCLASS(NotBlueprintable)
class URuleRangerAutomationRequiredLeafObject final : public UObject
{
    GENERATED_BODY()
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationRequiredObjectOwner final : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, meta = (RuleRangerRequired))
    TObjectPtr<UObject> RequiredObject{ nullptr };
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationRequiredArrayOwner final : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, meta = (RuleRangerRequired))
    TObjectPtr<UObject> RequiredObjects[2]{ nullptr, nullptr };
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationUnsupportedRequiredOwner final : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, meta = (RuleRangerRequired))
    int32 RequiredValue{ 0 };
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationRequiredComponentObject final : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    TObjectPtr<UObject> RequiredObject{ nullptr };

    UPROPERTY(EditAnywhere)
    int32 RequiredValue{ 0 };
};

UCLASS(NotBlueprintable, meta = (RuleRangerRequired = "RequiredComponent.RequiredObject"))
class URuleRangerAutomationRequiredComponentOwner final : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    TObjectPtr<URuleRangerAutomationRequiredComponentObject> RequiredComponent{ nullptr };
};

UCLASS(NotBlueprintable, meta = (RuleRangerRequired = "MissingComponent.RequiredObject"))
class URuleRangerAutomationMissingComponentOwner final : public UObject
{
    GENERATED_BODY()
};

UCLASS(NotBlueprintable, meta = (RuleRangerRequired = "RequiredComponent.MissingProperty"))
class URuleRangerAutomationMissingComponentPropertyOwner final : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    TObjectPtr<URuleRangerAutomationRequiredComponentObject> RequiredComponent{ nullptr };
};

UCLASS(NotBlueprintable, meta = (RuleRangerRequired = "RequiredComponent.RequiredValue"))
class URuleRangerAutomationInvalidComponentPropertyOwner final : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    TObjectPtr<URuleRangerAutomationRequiredComponentObject> RequiredComponent{ nullptr };
};

UCLASS(NotBlueprintable, meta = (RuleRangerRequired = "RequiredComponent.RequiredObject"))
class URuleRangerAutomationInheritedRequiredBaseOwner : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    TObjectPtr<URuleRangerAutomationRequiredComponentObject> RequiredComponent{ nullptr };
};

UCLASS(NotBlueprintable)
class URuleRangerAutomationInheritedRequiredDerivedOwner final : public URuleRangerAutomationInheritedRequiredBaseOwner
{
    GENERATED_BODY()
};

UCLASS(Abstract, NotBlueprintable, meta = (RuleRangerRequired = "RequiredComponent.RequiredObject"))
class URuleRangerAutomationAbstractRequiredOwner : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    TObjectPtr<URuleRangerAutomationRequiredComponentObject> RequiredComponent{ nullptr };
};
