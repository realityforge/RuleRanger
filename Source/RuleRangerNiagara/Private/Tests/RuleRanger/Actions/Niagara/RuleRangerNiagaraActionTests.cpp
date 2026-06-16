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

    #include "Misc/AutomationTest.h"
    #include "NiagaraEmitter.h"
    #include "NiagaraEmitterHandle.h"
    #include "NiagaraScript.h"
    #include "NiagaraSystem.h"
    #include "RuleRanger/Actions/Niagara/CheckNestedEmitterNameMatchesPatternAction.h"
    #include "RuleRanger/Actions/Niagara/CheckNiagaraEmitterCompileStatusAction.h"
    #include "RuleRanger/Actions/Niagara/CheckNiagaraSystemCompileStatusAction.h"
    #include "RuleRanger/Actions/Niagara/EnsureNoDisabledEmittersAction.h"
    #include "RuleRangerAction.h"
    #include "RuleRangerActionContext.h"
    #include "UObject/UnrealType.h"

namespace RuleRangerNiagaraActionTests
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
        const auto Property = Object ? FindFProperty<FProperty>(Object->GetClass(), PropertyName) : nullptr;
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
    SetBoolPropertyValue(FAutomationTestBase& Test, UObject* const Object, const TCHAR* PropertyName, const bool Value)
    {
        const auto Property = Object ? FindFProperty<FBoolProperty>(Object->GetClass(), PropertyName) : nullptr;
        if (Test.TestNotNull(FString::Printf(TEXT("Bool property %s should exist"), PropertyName), Property))
        {
            Property->SetPropertyValue_InContainer(Object, Value);
            return true;
        }
        return false;
    }

    bool SetEnumPropertyValue(UObject* const Object, const TCHAR* PropertyName, const int64 Value)
    {
        const auto Property = Object ? FindFProperty<FProperty>(Object->GetClass(), PropertyName) : nullptr;
        if (!Property)
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
                               const TCHAR* const Description,
                               const TCHAR* const ExpectedFragment)
    {
        return Test.TestTrue(Description, TextArrayContainsFragment(Messages, ExpectedFragment));
    }

    URuleRangerActionContext* NewActionContext()
    {
        const auto Context = NewTransientObject<URuleRangerActionContext>();
        if (Context)
        {
            SetEnumPropertyValue(Context, TEXT("ActionState"), static_cast<int64>(ERuleRangerActionState::AS_Success));
            SetEnumPropertyValue(Context,
                                 TEXT("ActionTrigger"),
                                 static_cast<int64>(ERuleRangerActionTrigger::AT_Report));
        }
        return Context;
    }

    void SetCompileStatus(UNiagaraScript* const Script, const ENiagaraScriptCompileStatus Status)
    {
        if (Script)
        {
            Script->GetVMExecutableData().LastCompileStatus = Status;
        }
    }

    void SetAllEmitterScriptCompileStatuses(UNiagaraEmitter* const Emitter, const ENiagaraScriptCompileStatus Status)
    {
        if (const auto EmitterData = Emitter ? Emitter->GetLatestEmitterData() : nullptr)
        {
            TArray<UNiagaraScript*> Scripts;
            EmitterData->GetScripts(Scripts, true, true);
            for (const auto Script : Scripts)
            {
                SetCompileStatus(Script, Status);
            }
        }
    }

    UNiagaraEmitter*
    NewTransientEmitter(const TCHAR* const Name,
                        const ENiagaraScriptCompileStatus Status = ENiagaraScriptCompileStatus::NCS_UpToDate)
    {
        const auto Emitter = NewNamedTransientObject<UNiagaraEmitter>(Name);
        if (Emitter)
        {
            Emitter->SetUniqueEmitterName(Name);
            SetAllEmitterScriptCompileStatuses(Emitter, Status);
        }
        return Emitter;
    }

    FNiagaraEmitterHandle AddEmitter(UNiagaraSystem* const System,
                                     UNiagaraEmitter* const Emitter,
                                     const FName Name,
                                     const bool bEnabled = true)
    {
        FNiagaraEmitterHandle Handle(*Emitter, Emitter->GetExposedVersion().VersionGuid);
        Handle.SetName(Name, *System);
        Handle.SetIsEnabled(bEnabled, *System, false);
        System->AddEmitterHandleDirect(Handle);
        return Handle;
    }

    UNiagaraSystem* NewTransientSystem(const TCHAR* const Name)
    {
        const auto System = NewNamedTransientObject<UNiagaraSystem>(Name);
        if (System)
        {
            SetCompileStatus(System->GetSystemSpawnScript(), ENiagaraScriptCompileStatus::NCS_UpToDate);
            SetCompileStatus(System->GetSystemUpdateScript(), ENiagaraScriptCompileStatus::NCS_UpToDate);
        }
        return System;
    }

    UNiagaraSystem*
    NewSystemWithEmitter(const TCHAR* const SystemName,
                         const TCHAR* const EmitterObjectName,
                         const FName EmitterDisplayName,
                         const bool bEmitterEnabled = true,
                         const ENiagaraScriptCompileStatus EmitterStatus = ENiagaraScriptCompileStatus::NCS_UpToDate)
    {
        const auto System = NewTransientSystem(SystemName);
        const auto Emitter = NewTransientEmitter(EmitterObjectName, EmitterStatus);
        if (System && Emitter)
        {
            AddEmitter(System, Emitter, EmitterDisplayName, bEmitterEnabled);
        }
        return System;
    }

    bool CanActionRunForObject(const URuleRangerAction* const Action, const UObject* const Object)
    {
        return Action && Object && Object->IsA(Action->GetExpectedType());
    }
} // namespace RuleRangerNiagaraActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraEmitterCompileStatusAcceptsCleanEmitterTest,
                                 "RuleRanger.Actions.Niagara.EmitterCompileStatus.AcceptsCleanEmitter",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraEmitterCompileStatusAcceptsCleanEmitterTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNiagaraEmitterCompileStatusAction>();
    const auto Emitter = NewTransientEmitter(TEXT("CleanEmitter"));
    const auto Context = NewActionContext();
    if (TestNotNull(TEXT("Action should be created"), Action) && TestNotNull(TEXT("Emitter should be created"), Emitter)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        Action->Apply(Context, Emitter);
        return TestTrue(TEXT("Clean emitter scripts should not add errors"), Context->GetErrorMessages().IsEmpty());
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraEmitterCompileStatusReportsErrorStatusTest,
                                 "RuleRanger.Actions.Niagara.EmitterCompileStatus.ReportsErrorStatus",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraEmitterCompileStatusReportsErrorStatusTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNiagaraEmitterCompileStatusAction>();
    const auto Emitter = NewTransientEmitter(TEXT("ErrorEmitter"), ENiagaraScriptCompileStatus::NCS_Error);
    const auto Context = NewActionContext();
    if (TestNotNull(TEXT("Action should be created"), Action) && TestNotNull(TEXT("Emitter should be created"), Emitter)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        Action->Apply(Context, Emitter);
        return TestEqual(TEXT("Error status should add one error"), Context->GetErrorMessages().Num(), 1)
            && TestTextArrayContains(*this,
                                     Context->GetErrorMessages(),
                                     TEXT("Error diagnostic should mention error status"),
                                     TEXT("error status"));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraEmitterCompileStatusHonorsWarningAndUnknownPoliciesTest,
                                 "RuleRanger.Actions.Niagara.EmitterCompileStatus.HonorsWarningAndUnknownPolicies",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraEmitterCompileStatusHonorsWarningAndUnknownPoliciesTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto WarningAction = NewTransientObject<UCheckNiagaraEmitterCompileStatusAction>();
    const auto WarningEmitter =
        NewTransientEmitter(TEXT("WarningEmitter"), ENiagaraScriptCompileStatus::NCS_UpToDateWithWarnings);
    const auto WarningContext = NewActionContext();
    const auto UnknownAction = NewTransientObject<UCheckNiagaraEmitterCompileStatusAction>();
    const auto UnknownEmitter = NewTransientEmitter(TEXT("UnknownEmitter"), ENiagaraScriptCompileStatus::NCS_Unknown);
    const auto UnknownContext = NewActionContext();
    if (TestNotNull(TEXT("Warning action should be created"), WarningAction)
        && TestNotNull(TEXT("Warning emitter should be created"), WarningEmitter)
        && TestNotNull(TEXT("Warning context should be created"), WarningContext)
        && TestNotNull(TEXT("Unknown action should be created"), UnknownAction)
        && TestNotNull(TEXT("Unknown emitter should be created"), UnknownEmitter)
        && TestNotNull(TEXT("Unknown context should be created"), UnknownContext)
        && SetBoolPropertyValue(*this, WarningAction, TEXT("bErrorOnUpToDateWithWarnings"), false)
        && SetBoolPropertyValue(*this, UnknownAction, TEXT("bErrorOnUnknown"), true))
    {
        WarningAction->Apply(WarningContext, WarningEmitter);
        UnknownAction->Apply(UnknownContext, UnknownEmitter);

        return TestTrue(TEXT("Warnings should be accepted when configured"),
                        WarningContext->GetErrorMessages().IsEmpty())
            && TestEqual(TEXT("Unknown status should add an error when configured"),
                         UnknownContext->GetErrorMessages().Num(),
                         1);
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerNiagaraEmitterCompileStatusRejectsWrongTypeAndNullThroughExpectedTypeTest,
    "RuleRanger.Actions.Niagara.EmitterCompileStatus.RejectsWrongTypeAndNullThroughExpectedType",
    RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraEmitterCompileStatusRejectsWrongTypeAndNullThroughExpectedTypeTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNiagaraEmitterCompileStatusAction>();
    const auto System = NewTransientSystem(TEXT("WrongTypeSystem"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Wrong-type system should be created"), System))
    {
        return TestEqual(TEXT("Emitter compile status should target Niagara emitters"),
                         Action->GetExpectedType(),
                         UNiagaraEmitter::StaticClass())
            && TestFalse(TEXT("Wrong-type objects should be rejected by the action type gate"),
                         CanActionRunForObject(Action, System))
            && TestFalse(TEXT("Null objects should be rejected by the action type gate"),
                         CanActionRunForObject(Action, nullptr));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraSystemCompileStatusAcceptsCleanSystemTest,
                                 "RuleRanger.Actions.Niagara.SystemCompileStatus.AcceptsCleanSystem",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraSystemCompileStatusAcceptsCleanSystemTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNiagaraSystemCompileStatusAction>();
    const auto System = NewSystemWithEmitter(TEXT("CleanSystem"), TEXT("CleanSystemEmitter"), TEXT("CleanEmitter"));
    const auto Context = NewActionContext();
    if (TestNotNull(TEXT("Action should be created"), Action) && TestNotNull(TEXT("System should be created"), System)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        Action->Apply(Context, System);
        return TestTrue(TEXT("Clean system and emitter scripts should not add errors"),
                        Context->GetErrorMessages().IsEmpty());
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraSystemCompileStatusReportsSystemScriptErrorsTest,
                                 "RuleRanger.Actions.Niagara.SystemCompileStatus.ReportsSystemScriptErrors",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraSystemCompileStatusReportsSystemScriptErrorsTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNiagaraSystemCompileStatusAction>();
    const auto System = NewSystemWithEmitter(TEXT("DirtySystem"), TEXT("DirtySystemEmitter"), TEXT("DirtyEmitter"));
    const auto Context = NewActionContext();
    if (TestNotNull(TEXT("Action should be created"), Action) && TestNotNull(TEXT("System should be created"), System)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        SetCompileStatus(System->GetSystemUpdateScript(), ENiagaraScriptCompileStatus::NCS_Dirty);
        Action->Apply(Context, System);
        return TestEqual(TEXT("Dirty system script should add one error"), Context->GetErrorMessages().Num(), 1)
            && TestTextArrayContains(*this,
                                     Context->GetErrorMessages(),
                                     TEXT("System diagnostic should mention recompilation"),
                                     TEXT("needs to be recompiled"));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraSystemCompileStatusReportsEnabledEmitterErrorsTest,
                                 "RuleRanger.Actions.Niagara.SystemCompileStatus.ReportsEnabledEmitterErrors",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraSystemCompileStatusReportsEnabledEmitterErrorsTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNiagaraSystemCompileStatusAction>();
    const auto System = NewSystemWithEmitter(TEXT("EmitterErrorSystem"),
                                             TEXT("EmitterErrorEmitter"),
                                             TEXT("EmitterError"),
                                             true,
                                             ENiagaraScriptCompileStatus::NCS_Error);
    const auto Context = NewActionContext();
    if (TestNotNull(TEXT("Action should be created"), Action) && TestNotNull(TEXT("System should be created"), System)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        Action->Apply(Context, System);
        return TestEqual(TEXT("Enabled emitter error status should add one error"),
                         Context->GetErrorMessages().Num(),
                         1)
            && TestTextArrayContains(*this,
                                     Context->GetErrorMessages(),
                                     TEXT("Emitter diagnostic should mention the emitter context"),
                                     TEXT("Emitter named EmitterError"));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraSystemCompileStatusSkipsDisabledEmitterCompileErrorsTest,
                                 "RuleRanger.Actions.Niagara.SystemCompileStatus.SkipsDisabledEmitterCompileErrors",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraSystemCompileStatusSkipsDisabledEmitterCompileErrorsTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNiagaraSystemCompileStatusAction>();
    const auto System = NewSystemWithEmitter(TEXT("DisabledEmitterErrorSystem"),
                                             TEXT("DisabledEmitterErrorEmitter"),
                                             TEXT("DisabledEmitter"),
                                             false,
                                             ENiagaraScriptCompileStatus::NCS_Error);
    const auto Context = NewActionContext();
    if (TestNotNull(TEXT("Action should be created"), Action) && TestNotNull(TEXT("System should be created"), System)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        Action->Apply(Context, System);
        return TestTrue(TEXT("Disabled emitters should not be checked for compile status"),
                        Context->GetErrorMessages().IsEmpty());
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerNiagaraSystemCompileStatusRejectsWrongTypeAndNullThroughExpectedTypeTest,
    "RuleRanger.Actions.Niagara.SystemCompileStatus.RejectsWrongTypeAndNullThroughExpectedType",
    RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraSystemCompileStatusRejectsWrongTypeAndNullThroughExpectedTypeTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNiagaraSystemCompileStatusAction>();
    const auto Emitter = NewTransientEmitter(TEXT("WrongTypeEmitter"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Wrong-type emitter should be created"), Emitter))
    {
        return TestEqual(TEXT("System compile status should target Niagara systems"),
                         Action->GetExpectedType(),
                         UNiagaraSystem::StaticClass())
            && TestFalse(TEXT("Wrong-type objects should be rejected by the action type gate"),
                         CanActionRunForObject(Action, Emitter))
            && TestFalse(TEXT("Null objects should be rejected by the action type gate"),
                         CanActionRunForObject(Action, nullptr));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraEnsureNoDisabledEmittersAcceptsEnabledEmittersTest,
                                 "RuleRanger.Actions.Niagara.EnsureNoDisabledEmitters.AcceptsEnabledEmitters",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraEnsureNoDisabledEmittersAcceptsEnabledEmittersTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UEnsureNoDisabledEmittersAction>();
    const auto System =
        NewSystemWithEmitter(TEXT("EnabledSystem"), TEXT("EnabledEmitterObject"), TEXT("EnabledEmitter"));
    const auto Context = NewActionContext();
    if (TestNotNull(TEXT("Action should be created"), Action) && TestNotNull(TEXT("System should be created"), System)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        Action->Apply(Context, System);
        return TestTrue(TEXT("Enabled emitters should not add errors"), Context->GetErrorMessages().IsEmpty());
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraEnsureNoDisabledEmittersReportsDisabledEmittersTest,
                                 "RuleRanger.Actions.Niagara.EnsureNoDisabledEmitters.ReportsDisabledEmitters",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraEnsureNoDisabledEmittersReportsDisabledEmittersTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UEnsureNoDisabledEmittersAction>();
    const auto System =
        NewSystemWithEmitter(TEXT("DisabledSystem"), TEXT("DisabledEmitterObject"), TEXT("DisabledEmitter"), false);
    const auto Context = NewActionContext();
    if (TestNotNull(TEXT("Action should be created"), Action) && TestNotNull(TEXT("System should be created"), System)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        Action->Apply(Context, System);
        return TestEqual(TEXT("Disabled emitters should add one error"), Context->GetErrorMessages().Num(), 1)
            && TestTextArrayContains(*this,
                                     Context->GetErrorMessages(),
                                     TEXT("Disabled-emitter diagnostic should mention the emitter"),
                                     TEXT("DisabledEmitter"));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerNiagaraEnsureNoDisabledEmittersRejectsWrongTypeAndNullThroughExpectedTypeTest,
    "RuleRanger.Actions.Niagara.EnsureNoDisabledEmitters.RejectsWrongTypeAndNullThroughExpectedType",
    RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraEnsureNoDisabledEmittersRejectsWrongTypeAndNullThroughExpectedTypeTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UEnsureNoDisabledEmittersAction>();
    const auto Emitter = NewTransientEmitter(TEXT("WrongDisabledTypeEmitter"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Wrong-type emitter should be created"), Emitter))
    {
        return TestEqual(TEXT("Disabled-emitter action should target Niagara systems"),
                         Action->GetExpectedType(),
                         UNiagaraSystem::StaticClass())
            && TestFalse(TEXT("Wrong-type objects should be rejected by the action type gate"),
                         CanActionRunForObject(Action, Emitter))
            && TestFalse(TEXT("Null objects should be rejected by the action type gate"),
                         CanActionRunForObject(Action, nullptr));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraNestedEmitterNamePatternAcceptsMatchingNamesTest,
                                 "RuleRanger.Actions.Niagara.NestedEmitterNamePattern.AcceptsMatchingNames",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraNestedEmitterNamePatternAcceptsMatchingNamesTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNestedEmitterNameMatchesPatternAction>();
    const auto System =
        NewSystemWithEmitter(TEXT("MatchingNameSystem"), TEXT("MatchingEmitterObject"), TEXT("ValidEmitter_01"));
    const auto Context = NewActionContext();
    if (TestNotNull(TEXT("Action should be created"), Action) && TestNotNull(TEXT("System should be created"), System)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        Action->Apply(Context, System);
        return TestTrue(TEXT("Matching emitter names should not add errors"), Context->GetErrorMessages().IsEmpty());
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraNestedEmitterNamePatternReportsMismatchesTest,
                                 "RuleRanger.Actions.Niagara.NestedEmitterNamePattern.ReportsMismatches",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraNestedEmitterNamePatternReportsMismatchesTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNestedEmitterNameMatchesPatternAction>();
    const auto System =
        NewSystemWithEmitter(TEXT("MismatchNameSystem"), TEXT("MismatchEmitterObject"), TEXT("invalid-emitter"));
    const auto Context = NewActionContext();
    if (TestNotNull(TEXT("Action should be created"), Action) && TestNotNull(TEXT("System should be created"), System)
        && TestNotNull(TEXT("Action context should be created"), Context))
    {
        Action->Apply(Context, System);
        return TestEqual(TEXT("Mismatched emitter names should add one error"), Context->GetErrorMessages().Num(), 1)
            && TestTextArrayContains(*this,
                                     Context->GetErrorMessages(),
                                     TEXT("Name diagnostic should mention the mismatched emitter"),
                                     TEXT("invalid-emitter"));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerNiagaraNestedEmitterNamePatternHonorsEmptyAndCaseInsensitivePatternsTest,
    "RuleRanger.Actions.Niagara.NestedEmitterNamePattern.HonorsEmptyAndCaseInsensitivePatterns",
    RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraNestedEmitterNamePatternHonorsEmptyAndCaseInsensitivePatternsTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto EmptyPatternAction = NewTransientObject<UCheckNestedEmitterNameMatchesPatternAction>();
    const auto EmptyPatternSystem =
        NewSystemWithEmitter(TEXT("EmptyPatternSystem"), TEXT("EmptyPatternEmitterObject"), TEXT("anything-goes"));
    const auto EmptyPatternContext = NewActionContext();
    const auto CaseInsensitiveAction = NewTransientObject<UCheckNestedEmitterNameMatchesPatternAction>();
    const auto CaseInsensitiveSystem =
        NewSystemWithEmitter(TEXT("CasePatternSystem"), TEXT("CasePatternEmitterObject"), TEXT("lowername"));
    const auto CaseInsensitiveContext = NewActionContext();
    if (TestNotNull(TEXT("Empty-pattern action should be created"), EmptyPatternAction)
        && TestNotNull(TEXT("Empty-pattern system should be created"), EmptyPatternSystem)
        && TestNotNull(TEXT("Empty-pattern context should be created"), EmptyPatternContext)
        && TestNotNull(TEXT("Case-insensitive action should be created"), CaseInsensitiveAction)
        && TestNotNull(TEXT("Case-insensitive system should be created"), CaseInsensitiveSystem)
        && TestNotNull(TEXT("Case-insensitive context should be created"), CaseInsensitiveContext)
        && SetPropertyValue(*this, EmptyPatternAction, TEXT("Pattern"), FString())
        && SetPropertyValue(*this, CaseInsensitiveAction, TEXT("Pattern"), FString(TEXT("^LOWERNAME$")))
        && SetBoolPropertyValue(*this, CaseInsensitiveAction, TEXT("bCaseSensitive"), false))
    {
        EmptyPatternAction->Apply(EmptyPatternContext, EmptyPatternSystem);
        CaseInsensitiveAction->Apply(CaseInsensitiveContext, CaseInsensitiveSystem);

        return TestTrue(TEXT("Empty regex pattern should match every emitter name"),
                        EmptyPatternContext->GetErrorMessages().IsEmpty())
            && TestTrue(TEXT("Case-insensitive regex should match lowercase emitter name"),
                        CaseInsensitiveContext->GetErrorMessages().IsEmpty());
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerNiagaraNestedEmitterNamePatternReportsInvalidPatternAsMismatchTest,
                                 "RuleRanger.Actions.Niagara.NestedEmitterNamePattern.ReportsInvalidPatternAsMismatch",
                                 RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraNestedEmitterNamePatternReportsInvalidPatternAsMismatchTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNestedEmitterNameMatchesPatternAction>();
    const auto System =
        NewSystemWithEmitter(TEXT("InvalidPatternSystem"), TEXT("InvalidPatternEmitterObject"), TEXT("ValidEmitter"));
    const auto Context = NewActionContext();
    if (TestNotNull(TEXT("Action should be created"), Action) && TestNotNull(TEXT("System should be created"), System)
        && TestNotNull(TEXT("Action context should be created"), Context)
        && SetPropertyValue(*this, Action, TEXT("Pattern"), FString(TEXT("["))))
    {
        Action->Apply(Context, System);
        return TestEqual(TEXT("Invalid regex patterns should fail closed as a mismatch"),
                         Context->GetErrorMessages().Num(),
                         1)
            && TestTextArrayContains(*this,
                                     Context->GetErrorMessages(),
                                     TEXT("Invalid-pattern diagnostic should include the authored pattern"),
                                     TEXT("["));
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerNiagaraNestedEmitterNamePatternRejectsWrongTypeAndNullThroughExpectedTypeTest,
    "RuleRanger.Actions.Niagara.NestedEmitterNamePattern.RejectsWrongTypeAndNullThroughExpectedType",
    RuleRangerNiagaraActionTests::AutomationTestFlags)
bool FRuleRangerNiagaraNestedEmitterNamePatternRejectsWrongTypeAndNullThroughExpectedTypeTest::RunTest(const FString&)
{
    using namespace RuleRangerNiagaraActionTests;

    const auto Action = NewTransientObject<UCheckNestedEmitterNameMatchesPatternAction>();
    const auto Emitter = NewTransientEmitter(TEXT("WrongNamePatternTypeEmitter"));
    if (TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Wrong-type emitter should be created"), Emitter))
    {
        return TestEqual(TEXT("Nested-emitter name action should target Niagara systems"),
                         Action->GetExpectedType(),
                         UNiagaraSystem::StaticClass())
            && TestFalse(TEXT("Wrong-type objects should be rejected by the action type gate"),
                         CanActionRunForObject(Action, Emitter))
            && TestFalse(TEXT("Null objects should be rejected by the action type gate"),
                         CanActionRunForObject(Action, nullptr));
    }
    return false;
}

#endif
