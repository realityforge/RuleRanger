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
#include "RuleRanger/RuleRangerEditorSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "Logging/StructuredLog.h"
#include "RuleRangerActionContext.h"
#include "RuleRangerConfig.h"
#include "RuleRangerDefaultResultHandler.h"
#include "RuleRangerDeveloperSettings.h"
#include "RuleRangerLogging.h"
#include "RuleRangerMessageLog.h"
#include "RuleRangerRule.h"
#include "RuleRangerRuleExclusion.h"
#include "RuleRangerRuleSet.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Subsystems/ImportSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RuleRangerEditorSubsystem)

const static FString ImportMarkerValue = FString(TEXT("True"));

void URuleRangerEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    // Register delegate for OnAssetPostImport callback
    if (const auto Subsystem = GEditor->GetEditorSubsystem<UImportSubsystem>())
    {
        OnAssetPostImportDelegateHandle = Subsystem->OnAssetPostImport.AddUObject(this, &ThisClass::OnAssetPostImport);
    }
    DefaultResultHandler =
        NewObject<URuleRangerDefaultResultHandler>(this, URuleRangerDefaultResultHandler::StaticClass());
}

void URuleRangerEditorSubsystem::Deinitialize()
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<UImportSubsystem>())
    {
        // Deregister delegate for OnAssetPostImport callback
        if (OnAssetPostImportDelegateHandle.IsValid())
        {
            Subsystem->OnAssetPostImport.Remove(OnAssetPostImportDelegateHandle);
        }
    }
    OnAssetPostImportDelegateHandle.Reset();
}

void URuleRangerEditorSubsystem::ScanObject(UObject* InObject, IRuleRangerResultHandler* InResultHandler)
{
    const auto Handler = InResultHandler ? InResultHandler : DefaultResultHandler.GetInterface();
    ProcessRule(InObject, [this, Handler](auto Config, auto RuleSet, auto Rule, auto InnerInObject) mutable {
        return ProcessDemandScan(Config, RuleSet, Rule, InnerInObject, Handler);
    });
}

void URuleRangerEditorSubsystem::ScanAndFixObject(UObject* InObject, IRuleRangerResultHandler* InResultHandler)
{
    const auto Handler = InResultHandler ? InResultHandler : DefaultResultHandler.GetInterface();
    ProcessRule(InObject, [this, Handler](auto Config, auto RuleSet, auto Rule, auto InnerInObject) mutable {
        return ProcessDemandScanAndFix(Config, RuleSet, Rule, InnerInObject, Handler);
    });
}

void URuleRangerEditorSubsystem::ValidateObject(UObject* InObject,
                                                bool bIsSave,
                                                IRuleRangerResultHandler* InResultHandler)
{
    const auto Handler = InResultHandler ? InResultHandler : DefaultResultHandler.GetInterface();
    ProcessRule(InObject, [this, bIsSave, Handler](auto Config, auto RuleSet, auto Rule, auto InnerInObject) mutable {
        return ProcessOnAssetValidateRule(Config, RuleSet, Rule, InnerInObject, bIsSave, Handler);
    });
    // Ensure any per-object validation caches are cleared even if validator is not in play
    ClearValidationMatchCache();
}

bool URuleRangerEditorSubsystem::CanValidateObject(UObject* InObject, bool bIsSave)
{
    bool Result = false;
    // Begin caching matches for this object across CanValidate and Validate
    ResetValidationMatchCache(InObject);
    ProcessRule(InObject, [this, &Result, bIsSave](auto, auto, auto Rule, auto InnerObject) {
        if (CanValidateObject(Rule, InnerObject, bIsSave))
        {
            bool bMatches;
            if (ValidationMatchCacheObject == InnerObject)
            {
                const TWeakObjectPtr Key(Rule);
                if (const auto Cached = ValidationMatchCache.Find(Key))
                {
                    bMatches = *Cached;
                }
                else
                {
                    bMatches = Rule->Match(ActionContext, InnerObject);
                    ValidationMatchCache.Add(Key, bMatches);
                }
            }
            else
            {
                bMatches = Rule->Match(ActionContext, InnerObject);
            }

            if (bMatches)
            {
                Result = true;
            }
        }
        return true;
    });
    return Result;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool URuleRangerEditorSubsystem::CanValidateObject(const URuleRangerRule* Rule,
                                                   const UObject* InObject,
                                                   const bool bIsSave) const
{
    // ReSharper disable once CppTooWideScopeInitStatement
    if (!bIsSave && Rule->bApplyOnValidate)
    {
        UE_LOGFMT(LogRuleRanger,
                  VeryVerbose,
                  "CanValidateObject({Object}) detected applicable rule {Rule} for non-save validate.",
                  InObject->GetName(),
                  Rule->GetName());
        return true;
    }
    else if (bIsSave && Rule->bApplyOnSave)
    {
        UE_LOGFMT(LogRuleRanger,
                  VeryVerbose,
                  "CanValidateAsset({Object}) detected applicable rule {Rule} for save validate.",
                  InObject->GetName(),
                  Rule->GetName());
        return true;
    }
    else
    {
        return false;
    }
}

TConstArrayView<TWeakObjectPtr<URuleRangerConfig>> URuleRangerEditorSubsystem::GetCachedRuleSetConfigs()
{
    if (bRuleSetConfigCacheDirty)
    {
        CachedRuleSetConfigs.Reset();

        const auto DevSettings = GetDefault<URuleRangerDeveloperSettings>();
        if (IsValid(DevSettings))
        {
            for (const auto& SoftConfig : DevSettings->Configs)
            {
                if (const auto Config = SoftConfig.LoadSynchronous())
                {
                    CachedRuleSetConfigs.Add(Config);
                }
            }
            UE_LOGFMT(LogRuleRanger,
                      VeryVerbose,
                      "RuleSetConfigCache populated with {Count} entries.",
                      CachedRuleSetConfigs.Num());

            bRuleSetConfigCacheDirty = false;
        }
    }
    return CachedRuleSetConfigs;
}

void URuleRangerEditorSubsystem::MarkRuleSetConfigCacheDirty()
{
    if (!bRuleSetConfigCacheDirty)
    {
        CachedRuleSetConfigs.Reset();
        bRuleSetConfigCacheDirty = true;
        UE_LOGFMT(LogRuleRanger, Verbose, "Clearing the RuleSetConfig cache");
    }
}

IRuleRangerResultHandler* URuleRangerEditorSubsystem::GetDefaultResultHandler() const
{
    return DefaultResultHandler.GetInterface();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerEditorSubsystem::OnAssetPostImport([[maybe_unused]] UFactory* Factory, UObject* Object)
{
    if (Object->IsA<URuleRangerConfig>())
    {
        MarkRuleSetConfigCacheDirty();
    }

    const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();

    const static FName NAME_ImportMarkerKey = FName(TEXT("RuleRanger.ImportProcessed"));
    // Use a metadata tag when we have imported an asset so that when we try to reimport asset, we can
    // identify this through the presence of tag.
    const bool bIsReimport = Subsystem && Subsystem->GetMetadataTag(Object, NAME_ImportMarkerKey) == ImportMarkerValue;

    ProcessRule(Object, [this, bIsReimport](auto Config, auto RuleSet, auto Rule, auto InObject) {
        return ProcessOnAssetPostImportRule(Config,
                                            RuleSet,
                                            Rule,
                                            bIsReimport,
                                            InObject,
                                            DefaultResultHandler.GetInterface());
    });

    // Mark asset as having been processed by RuleRanger during import so we can detect reimports later
    if (Subsystem && !bIsReimport)
    {
        Subsystem->SetMetadataTag(Object, NAME_ImportMarkerKey, ImportMarkerValue);
    }
}

bool URuleRangerEditorSubsystem::ProcessRuleSetForObject(URuleRangerConfig* const Config,
                                                         URuleRangerRuleSet* const RuleSet,
                                                         const TArray<FRuleRangerRuleExclusion>& Exclusions,
                                                         UObject* Object,
                                                         const FRuleRangerRuleFn& ProcessRuleFunction,
                                                         TSet<const URuleRangerRuleSet*>& Visited)
{
    UE_LOGFMT(LogRuleRanger,
              VeryVerbose,
              "ProcessRule: Processing Rule Set {RuleSet} for object {Object}",
              RuleSet->GetName(),
              Object->GetName());

    if (Visited.Contains(RuleSet))
    {
        UE_LOGFMT(LogRuleRanger,
                  Error,
                  "ProcessRule: Detected cyclic reference involving Rule Set {RuleSet}. Skipping nested traversal.",
                  RuleSet->GetName());
        return true;
    }
    else
    {
        Visited.Add(RuleSet);

        for (const auto& Exclusion : Exclusions)
        {
            if (Exclusion.RuleSets.Contains(RuleSet))
            {
                UE_LOGFMT(LogRuleRanger,
                          VeryVerbose,
                          "ProcessRule: Rule Set {RuleSet} excluded for object "
                          "{Object} due to exclusion rule. Reason: {Reason}",
                          RuleSet->GetName(),
                          Object->GetName(),
                          Exclusion.Description.ToString());
                return true;
            }
        }

        for (const auto& NestedRuleSetPtr : RuleSet->RuleSets)
        {
            if (const auto NestedRuleSet = NestedRuleSetPtr.Get())
            {
                UE_LOGFMT(LogRuleRanger,
                          VeryVerbose,
                          "ProcessRule: Processing Nested Rule Set {RuleSet} for object {Object}",
                          NestedRuleSet->GetName(),
                          Object->GetName());
                if (!ProcessRuleSetForObject(Config, NestedRuleSet, Exclusions, Object, ProcessRuleFunction, Visited))
                {
                    return false;
                }
                UE_LOGFMT(LogRuleRanger,
                          VeryVerbose,
                          "ProcessRule: Completed processing of Nested Rule Set {RuleSet} for object {Object}",
                          NestedRuleSet->GetName(),
                          Object->GetName());
            }
            else
            {
                UE_LOGFMT(LogRuleRanger,
                          Error,
                          "ProcessRule: Invalid RuleSet skipped when processing rules for {Object} in config {Config}",
                          Object->GetName(),
                          Config->GetName());
            }
        }

        int RuleIndex = 0;
        for (const auto RulePtr : RuleSet->Rules)
        {
            // ReSharper disable once CppTooWideScopeInitStatement
            if (const auto Rule = RulePtr.Get(); IsValid(Rule))
            {
                bool bSkipRule = false;

                for (const auto& Exclusion : Exclusions)
                {
                    if (Exclusion.Rules.Contains(Rule))
                    {
                        UE_LOGFMT(LogRuleRanger,
                                  VeryVerbose,
                                  "ProcessRule: Rule {Rule} from RuleSet {RuleSet} was excluded for "
                                  "object {Object} due to exclusion rule. Reason: {Reason}",
                                  Rule->GetName(),
                                  RuleSet->GetName(),
                                  Object->GetName(),
                                  Exclusion.Description.ToString());
                        bSkipRule = true;
                    }
                }

                if (!bSkipRule && !ProcessRuleFunction(Config, RuleSet, Rule, Object))
                {
                    UE_LOGFMT(LogRuleRanger,
                              VeryVerbose,
                              "ProcessRule: Rule {Rule} from RuleSet {RuleSet} indicated that following "
                              "rules should be skipped for {Object}",
                              Rule->GetName(),
                              RuleSet->GetName(),
                              Object->GetName());
                    ActionContext->ClearContext();
                    return false;
                }
            }
            else
            {
                UE_LOGFMT(LogRuleRanger,
                          Error,
                          "ProcessRule: Invalid Rule skipped at index {RuleIndex} "
                          "in rule set '{RuleSet}' from config '{Config}' when analyzing "
                          "object '{Object}'",
                          RuleIndex,
                          RuleSet->GetName(),
                          Config->GetName(),
                          Object->GetName());
            }
            RuleIndex++;
        }
        return true;
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerEditorSubsystem::ProcessRule(UObject* Object, const FRuleRangerRuleFn& ProcessRuleFunction)
{
    if (IsValid(Object))
    {
        if (!ActionContext)
        {
            UE_LOGFMT(LogRuleRanger, VeryVerbose, "RuleRangerEditorSubsystem: Creating the initial ActionContext");
            ActionContext = NewObject<URuleRangerActionContext>(this, URuleRangerActionContext::StaticClass());
        }

        const auto Configs = GetCachedRuleSetConfigs();
        const auto Path = Object->GetPathName();
        UE_LOGFMT(LogRuleRanger,
                  VeryVerbose,
                  "ProcessRule: Located {Count} Rule Set Config(s) when "
                  "discovering rules for object {Object} at {Path}",
                  Configs.Num(),
                  Object->GetName(),
                  Path);
        TSet<const URuleRangerRuleSet*> Visited;
        for (const auto ConfigPtr : Configs)
        {
            if (const auto Config = ConfigPtr.Get())
            {
                if (Config->ConfigMatches(Path))
                {
                    TArray<FRuleRangerRuleExclusion> Exclusions;
                    Exclusions.Reserve(Config->Exclusions.Num());
                    for (auto& Exclusion : Config->Exclusions)
                    {
                        if (Exclusion.ExclusionMatches(*Object, Path))
                        {
                            Exclusions.Add(Exclusion);
                        }
                    }

                    for (const auto& RuleSetPtr : Config->RuleSets)
                    {
                        if (const auto RuleSet = RuleSetPtr.Get())
                        {
                            if (!ProcessRuleSetForObject(Config,
                                                         RuleSet,
                                                         Exclusions,
                                                         Object,
                                                         ProcessRuleFunction,
                                                         Visited))
                            {
                                return;
                            }
                        }
                        else
                        {
                            UE_LOGFMT(LogRuleRanger,
                                      Error,
                                      "ProcessRule: Invalid RuleSet skipped when processing rules "
                                      "for {Object} in config {Config}",
                                      Object->GetName(),
                                      Config->GetName());
                        }
                    }
                }
            }
            else
            {
                UE_LOGFMT(LogRuleRanger,
                          Error,
                          "Invalid RuleSetConfig skipped when processing rules for {Object}",
                          Object->GetName());
            }
        }
    }

    // We need to check that the ActionContext is valid as it may not have been initialized.
    // This happens when Object is not valid, gets renamed or removed by an action or we are importing
    // from an FBX that has no animation or mesh data.
    if (IsValid(ActionContext))
    {
        ActionContext->ClearContext();
    }
}

bool URuleRangerEditorSubsystem::ProcessOnAssetValidateRule(URuleRangerConfig* const Config,
                                                            URuleRangerRuleSet* const RuleSet,
                                                            URuleRangerRule* Rule,
                                                            UObject* InObject,
                                                            const bool bIsSave,
                                                            IRuleRangerResultHandler* InResultHandler) const
{
    if ((!bIsSave && Rule->bApplyOnValidate) || (bIsSave && Rule->bApplyOnSave))
    {
        // If we previously determined during CanValidate that this rule does not match this object,
        // skip invoking Apply/Match a second time.
        if (ValidationMatchCacheObject == InObject)
        {
            const TWeakObjectPtr Key(Rule);
            if (const auto Cached = ValidationMatchCache.Find(Key))
            {
                if (!*Cached)
                {
                    UE_LOGFMT(LogRuleRanger,
                              VeryVerbose,
                              "OnAssetValidate({Object}) skipped rule {Rule} due to cached non-match.",
                              InObject->GetName(),
                              Rule->GetName());

                    return true;
                }
            }
        }
        UE_LOGFMT(LogRuleRanger,
                  VeryVerbose,
                  "OnAssetValidate({Object}) detected applicable rule {Rule}.",
                  InObject->GetName(),
                  Rule->GetName());

        ActionContext->ResetContext(Config,
                                    RuleSet,
                                    Rule,
                                    InObject,
                                    bIsSave ? ERuleRangerActionTrigger::AT_Save
                                            : ERuleRangerActionTrigger::AT_Validate);

        Rule->Apply(ActionContext, InObject);

        InResultHandler->OnRuleApplied(ActionContext);

        const auto State = ActionContext->GetState();
        ActionContext->ClearContext();

        if (ERuleRangerActionState::AS_Fatal == State)
        {
            UE_LOGFMT(LogRuleRanger,
                      VeryVerbose,
                      "OnAssetValidate({Object}) applied rule {Rule} which resulted in fatal error. "
                      "Processing rules will not continue.",
                      InObject->GetName(),
                      Rule->GetName());
            return false;
        }
        else if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
        {
            UE_LOGFMT(LogRuleRanger,
                      VeryVerbose,
                      "OnAssetValidate({Object}) applied rule {Rule} which resulted in error. "
                      "Processing rules will not continue as ContinueOnError=False.",
                      InObject->GetName(),
                      Rule->GetName());
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return true;
    }
}

void URuleRangerEditorSubsystem::ResetValidationMatchCache(UObject* InObject)
{
    ValidationMatchCacheObject = InObject;
    ValidationMatchCache.Reset();
}

void URuleRangerEditorSubsystem::ClearValidationMatchCache()
{
    ValidationMatchCacheObject.Reset();
    ValidationMatchCache.Reset();
}

bool URuleRangerEditorSubsystem::ProcessOnAssetPostImportRule(URuleRangerConfig* const Config,
                                                              URuleRangerRuleSet* const RuleSet,
                                                              URuleRangerRule* Rule,
                                                              const bool bIsReimport,
                                                              UObject* InObject,
                                                              IRuleRangerResultHandler* ResultHandler) const
{
    check(ActionContext);

    if ((!bIsReimport && Rule->bApplyOnImport) || (bIsReimport && Rule->bApplyOnReimport))
    {
        UE_LOGFMT(LogRuleRanger,
                  VeryVerbose,
                  "OnAssetPostImport({Object}) applying rule {Rule} during {Action}.",
                  InObject->GetName(),
                  Rule->GetName(),
                  bIsReimport ? TEXT("reimport") : TEXT("import"));
        const auto Trigger = bIsReimport ? ERuleRangerActionTrigger::AT_Reimport : ERuleRangerActionTrigger::AT_Import;
        ActionContext->ResetContext(Config, RuleSet, Rule, InObject, Trigger);

        Rule->Apply(ActionContext, InObject);

        ResultHandler->OnRuleApplied(ActionContext);

        const auto State = ActionContext->GetState();
        ActionContext->ClearContext();

        if (ERuleRangerActionState::AS_Fatal == State)
        {
            UE_LOGFMT(LogRuleRanger,
                      VeryVerbose,
                      "OnAssetPostImport({Object}) applied rule {Rule} which resulted in fatal error. "
                      "Processing rules will not continue.",
                      InObject->GetName(),
                      Rule->GetName());
            return false;
        }
        else if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
        {
            UE_LOGFMT(LogRuleRanger,
                      VeryVerbose,
                      "OnAssetPostImport({Object}) applied rule {Rule} which resulted in error. "
                      "Processing rules will not continue as ContinueOnError=False.",
                      InObject->GetName(),
                      Rule->GetName());
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        UE_LOGFMT(LogRuleRanger,
                  VeryVerbose,
                  "OnAssetPostImport({Object}) skipped rule {Rule} as flag on "
                  "rule does not enable rule during {Phase}.",
                  InObject->GetName(),
                  Rule->GetName(),
                  bIsReimport ? TEXT("reimport") : TEXT("import"));
        return true;
    }
}

bool URuleRangerEditorSubsystem::ProcessDemandScan(URuleRangerConfig* const Config,
                                                   URuleRangerRuleSet* const RuleSet,
                                                   URuleRangerRule* Rule,
                                                   UObject* InObject,
                                                   IRuleRangerResultHandler* ResultHandler) const
{
    check(ActionContext);

    if (Rule->bApplyOnDemand)
    {
        UE_LOGFMT(LogRuleRanger,
                  VeryVerbose,
                  "ProcessDemandScan({Object}) applying rule {Rule}.",
                  InObject->GetName(),
                  Rule->GetName());
        ActionContext->ResetContext(Config, RuleSet, Rule, InObject, ERuleRangerActionTrigger::AT_Report);

        Rule->Apply(ActionContext, InObject);

        ResultHandler->OnRuleApplied(ActionContext);

        const auto State = ActionContext->GetState();
        ActionContext->ClearContext();

        if (ERuleRangerActionState::AS_Fatal == State)
        {
            UE_LOGFMT(LogRuleRanger,
                      VeryVerbose,
                      "ProcessDemandScan({Object}) applied rule {Rule} which resulted in fatal error. "
                      "Processing rules will not continue.",
                      InObject->GetName(),
                      Rule->GetName());
            return false;
        }
        else if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
        {
            UE_LOGFMT(LogRuleRanger,
                      VeryVerbose,
                      "ProcessDemandScan({Object}) applied rule {Rule} which resulted in error. "
                      "Processing rules will not continue as ContinueOnError=False.",
                      InObject->GetName(),
                      Rule->GetName());
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        UE_LOGFMT(LogRuleRanger,
                  VeryVerbose,
                  "ProcessDemandScan({Object}) skipped rule {Rule} as flag on "
                  "rule does not enable rule on demand.",
                  InObject->GetName(),
                  Rule->GetName());
        return true;
    }
}

bool URuleRangerEditorSubsystem::ProcessDemandScanAndFix(URuleRangerConfig* const Config,
                                                         URuleRangerRuleSet* const RuleSet,
                                                         URuleRangerRule* Rule,
                                                         UObject* InObject,
                                                         IRuleRangerResultHandler* ResultHandler) const
{
    check(ActionContext);

    if (Rule->bApplyOnDemand)
    {
        UE_LOGFMT(LogRuleRanger,
                  VeryVerbose,
                  "ProcessDemandScanAndFix({Object}) applying rule {Rule}.",
                  InObject->GetName(),
                  Rule->GetName());
        ActionContext->ResetContext(Config, RuleSet, Rule, InObject, ERuleRangerActionTrigger::AT_Fix);

        Rule->Apply(ActionContext, InObject);

        ResultHandler->OnRuleApplied(ActionContext);

        // ReSharper disable once CppTooWideScopeInitStatement
        const auto State = ActionContext->GetState();
        ActionContext->ClearContext();

        if (ERuleRangerActionState::AS_Fatal == State)
        {
            UE_LOGFMT(LogRuleRanger,
                      VeryVerbose,
                      "ProcessDemandScanAndFix({Object}) applied rule {Rule} which resulted in fatal error. "
                      "Processing rules will not continue.",
                      InObject->GetName(),
                      Rule->GetName());
            return false;
        }
        else if (!Rule->bContinueOnError && ERuleRangerActionState::AS_Error == State)
        {
            UE_LOGFMT(LogRuleRanger,
                      VeryVerbose,
                      "ProcessDemandScanAndFix({Object}) applied rule {Rule} which resulted in error. "
                      "Processing rules will not continue as ContinueOnError=False.",
                      InObject->GetName(),
                      Rule->GetName());
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        UE_LOGFMT(LogRuleRanger,
                  VeryVerbose,
                  "ProcessDemandScanAndFix({Object}) skipped rule {Rule} as flag on "
                  "rule does not enable rule on demand.",
                  InObject->GetName(),
                  Rule->GetName());
        return true;
    }
}

static void MaybeOpenMessageLog(FMessageLog& MessageLog)
{
    const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
    check(IsValid(DeveloperSettings));
    MessageLog.Open(DeveloperSettings->bAlwaysShowMessageLog ? EMessageSeverity::Info : EMessageSeverity::Warning);
}

static void TickTask(FScopedSlowTask& SlowTask)
{
    SlowTask.EnterProgressFrame();
    SlowTask.TickProgress();
}

void URuleRangerEditorSubsystem::OnScanSelectedAssets(const TArray<FAssetData>& Assets)
{
    if (Assets.Num() > 0)
    {
        FScopedSlowTask SlowTask(
            Assets.Num(),
            NSLOCTEXT("RuleRanger", "ScanAssetsStarting", "Rule Ranger: Scan the selected assets"));
        SlowTask.MakeDialogDelayed(.5f, true);
        FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
        MessageLog.Info()->AddToken(
            FTextToken::Create(FText::Format(NSLOCTEXT("RuleRanger",
                                                       "ScanSelectedAssetsStartingAt",
                                                       "Rule Ranger has started scanning the selected assets at {0}"),
                                             FText::AsDateTime(FDateTime::UtcNow()))));

        for (const auto& Asset : Assets)
        {
            if (SlowTask.ShouldCancel())
            {
                MessageLog.Info()->AddToken(FTextToken::Create(FText::Format(
                    NSLOCTEXT("RuleRanger",
                              "CancelScanSelectedAssets",
                              "User requested that Rule Ranger cancel the scanning of the selected assets at {0}"),
                    FText::AsDateTime(FDateTime::UtcNow()))));
                MaybeOpenMessageLog(MessageLog);
                return;
            }
            // Object can be null if it is a redirect
            if (const auto Object = Asset.GetAsset())
            {
                ScanObject(Object);
            }
            TickTask(SlowTask);
        }
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanSelectedAssetsCompleted",
                                    "Rule Ranger has completed scanning of the selected assets at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));
        MaybeOpenMessageLog(MessageLog);
    }
}

void URuleRangerEditorSubsystem::OnFixSelectedAssets(const TArray<FAssetData>& Assets)
{
    if (Assets.Num() > 0)
    {
        FScopedSlowTask SlowTask(
            Assets.Num(),
            NSLOCTEXT("RuleRanger", "ScanAndFixAssetsStarting", "Rule Ranger: Scan and fix the selected assets"));
        SlowTask.MakeDialogDelayed(.5f, true);
        FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanAndFixSelectedAssetsStartingAt",
                                    "Rule Ranger has started scanning and fixing of the selected assets at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));

        for (const auto& Asset : Assets)
        {
            if (SlowTask.ShouldCancel())
            {
                MessageLog.Info()->AddToken(FTextToken::Create(FText::Format(
                    NSLOCTEXT(
                        "RuleRanger",
                        "CancelScanAndFixSelectedAssets",
                        "User requested that Rule Ranger cancel the scanning and fixing of the selected assets at {0}"),
                    FText::AsDateTime(FDateTime::UtcNow()))));
                MaybeOpenMessageLog(MessageLog);
                return;
            }
            // Object can be null if it is a redirect
            if (const auto Object = Asset.GetAsset())
            {
                ScanAndFixObject(Object);
            }
            TickTask(SlowTask);
        }
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanAndFixSelectedAssetsCompleted",
                                    "Rule Ranger has completed scanning and fixing of the selected assets at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));
        MaybeOpenMessageLog(MessageLog);
    }
}

static void CollectAssetsFromPaths(const TArray<FString>& AssetPaths, TArray<FAssetData>& Assets)
{
    const auto& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
    for (const auto& AssetPath : AssetPaths)
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto& AssetData = AssetRegistry.GetAssetByObjectPath(AssetPath);
        if (AssetData.IsRedirector())
        {
            // Ignore
        }
        else if (EName::None != AssetData.AssetName)
        {
            Assets.Add(AssetData);
        }
        else
        {
            TArray<FAssetData> OutAssets;
            AssetRegistry.GetAssetsByPath(*AssetPath, OutAssets, true);
            for (auto& OutAsset : OutAssets)
            {
                if (OutAsset.IsTopLevelAsset())
                {
                    Assets.Add(OutAsset);
                }
            }
        }
    }
}

void URuleRangerEditorSubsystem::OnScanSelectedPaths(const TArray<FString>& AssetPaths)
{
    TArray<FAssetData> Assets;
    CollectAssetsFromPaths(AssetPaths, Assets);

    if (Assets.Num() > 0)
    {
        FScopedSlowTask SlowTask(
            Assets.Num(),
            NSLOCTEXT("RuleRanger", "ScanSelectedPathsStarting", "Rule Ranger: Scan the selected paths"));
        SlowTask.MakeDialogDelayed(.5f, true);
        FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
        MessageLog.Info()->AddToken(
            FTextToken::Create(FText::Format(NSLOCTEXT("RuleRanger",
                                                       "ScanSelectedPathsStartingAt",
                                                       "Rule Ranger has started scanning the selected paths at {0}"),
                                             FText::AsDateTime(FDateTime::UtcNow()))));

        for (const auto& AssetData : Assets)
        {
            if (SlowTask.ShouldCancel())
            {
                MessageLog.Info()->AddToken(FTextToken::Create(FText::Format(
                    NSLOCTEXT("RuleRanger",
                              "CancelScanSelectedPaths",
                              "User requested that Rule Ranger cancel the scanning of the selected paths at {0}"),
                    FText::AsDateTime(FDateTime::UtcNow()))));
                MaybeOpenMessageLog(MessageLog);
                return;
            }

            if (const auto Object = AssetData.GetAsset())
            {
                ScanObject(Object);
            }
            TickTask(SlowTask);
        }
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanSelectedPathsCompleted",
                                    "Rule Ranger has completed scanning of the selected paths at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));
        MaybeOpenMessageLog(MessageLog);
    }
}

void URuleRangerEditorSubsystem::OnFixSelectedPaths(const TArray<FString>& AssetPaths)
{
    TArray<FAssetData> Assets;
    CollectAssetsFromPaths(AssetPaths, Assets);

    if (Assets.Num() > 0)
    {
        FScopedSlowTask SlowTask(
            Assets.Num(),
            NSLOCTEXT("RuleRanger", "ScanAndFixSelectedPathsStarting", "Rule Ranger: Scan and fix the selected paths"));
        SlowTask.MakeDialogDelayed(.5f, true);
        FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanAndFixSelectedPathsStartingAt",
                                    "Rule Ranger has started scanning and fixing of the selected paths at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));

        for (const auto& AssetData : Assets)
        {
            if (SlowTask.ShouldCancel())
            {
                MessageLog.Info()->AddToken(FTextToken::Create(FText::Format(
                    NSLOCTEXT(
                        "RuleRanger",
                        "CancelScanAndFixSelectedPaths",
                        "User requested that Rule Ranger cancel the scanning and fixing of the selected paths at {0}"),
                    FText::AsDateTime(FDateTime::UtcNow()))));
                MaybeOpenMessageLog(MessageLog);
                return;
            }

            if (const auto Object = AssetData.GetAsset())
            {
                ScanAndFixObject(Object);
            }
            TickTask(SlowTask);
        }

        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanAndFixSelectedPathsCompleted",
                                    "Rule Ranger has completed scanning and fixing of the selected paths at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));
        MaybeOpenMessageLog(MessageLog);
    }
}

static void CollectConfiguredPaths(TArray<FString>& OutPaths)
{
    TSet<FString> UniquePaths;
    const auto DevSettings = GetDefault<URuleRangerDeveloperSettings>();
    if (IsValid(DevSettings))
    {
        for (const auto& SoftConfig : DevSettings->Configs)
        {
            if (const auto Config = SoftConfig.LoadSynchronous())
            {
                for (const auto& Dir : Config->Dirs)
                {
                    if (!Dir.Path.IsEmpty())
                    {
                        UniquePaths.Add(Dir.Path);
                    }
                }
            }
        }
    }
    OutPaths.Reserve(UniquePaths.Num());
    for (const auto& Path : UniquePaths)
    {
        OutPaths.Add(Path);
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerEditorSubsystem::OnScanConfiguredContent()
{
    TArray<FString> AssetPaths;
    CollectConfiguredPaths(AssetPaths);

    TArray<FAssetData> Assets;
    CollectAssetsFromPaths(AssetPaths, Assets);

    if (Assets.Num() > 0)
    {
        FScopedSlowTask SlowTask(
            Assets.Num(),
            NSLOCTEXT("RuleRanger", "ScanConfiguredPathsStarting", "Rule Ranger: Scan configured content"));
        SlowTask.MakeDialogDelayed(.5f, true);
        FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanConfiguredContentStartingAt",
                                    "Rule Ranger has started scanning the configured content at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));

        TSet<FSoftObjectPath> Seen;
        for (const auto& AssetData : Assets)
        {
            if (SlowTask.ShouldCancel())
            {
                MessageLog.Info()->AddToken(FTextToken::Create(FText::Format(
                    NSLOCTEXT("RuleRanger",
                              "CancelScanConfiguredContent",
                              "User requested that Rule Ranger cancel the scanning of the configured content at {0}"),
                    FText::AsDateTime(FDateTime::UtcNow()))));
                MaybeOpenMessageLog(MessageLog);
                return;
            }

            if (const auto Object = AssetData.GetAsset())
            {
                const auto ObjPath = AssetData.GetSoftObjectPath();
                if (!Seen.Contains(ObjPath))
                {
                    Seen.Add(ObjPath);
                    ScanObject(Object);
                }
            }
            SlowTask.EnterProgressFrame();
            SlowTask.TickProgress();
        }
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanConfiguredContentCompleted",
                                    "Rule Ranger has completed scanning of the configured content at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));
        MaybeOpenMessageLog(MessageLog);
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void URuleRangerEditorSubsystem::OnFixConfiguredContent()
{
    TArray<FString> AssetPaths;
    CollectConfiguredPaths(AssetPaths);

    TArray<FAssetData> Assets;
    CollectAssetsFromPaths(AssetPaths, Assets);

    if (Assets.Num() > 0)
    {
        FScopedSlowTask SlowTask(Assets.Num(),
                                 NSLOCTEXT("RuleRanger",
                                           "ScanAndFixConfiguredPathsStarting",
                                           "Rule Ranger: Scan and fix configured content"));
        SlowTask.MakeDialogDelayed(.5f, true);
        FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanAndFixConfiguredContentStartingAt",
                                    "Rule Ranger has started scanning and fixing of the configured content at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));

        TSet<FSoftObjectPath> Seen;
        for (const auto& AssetData : Assets)
        {
            if (SlowTask.ShouldCancel())
            {
                MessageLog.Info()->AddToken(FTextToken::Create(FText::Format(
                    NSLOCTEXT(
                        "RuleRanger",
                        "CancelScanAndFixConfiguredContent",
                        "User requested that Rule Ranger cancel the scanning and fixing of the configured content at {0}"),
                    FText::AsDateTime(FDateTime::UtcNow()))));
                MaybeOpenMessageLog(MessageLog);
                return;
            }

            if (const auto Object = AssetData.GetAsset())
            {
                const auto ObjPath = AssetData.GetSoftObjectPath();
                if (!Seen.Contains(ObjPath))
                {
                    Seen.Add(ObjPath);
                    ScanAndFixObject(Object);
                }
            }
            SlowTask.EnterProgressFrame();
            SlowTask.TickProgress();
        }
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanAndFixConfiguredContentCompleted",
                                    "Rule Ranger has completed scanning and fixing of the configured content at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));
        MaybeOpenMessageLog(MessageLog);
    }
}
