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

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "UObject/Object.h"
#include <functional>
#include "RuleRangerEditorSubsystem.generated.h"

struct FRuleRangerRuleExclusion;
struct FAssetData;
class IRuleRangerResultHandler;
class UFactory;
class URuleRangerRule;
class URuleRangerActionContext;
class URuleRangerEditorValidator;
class URuleRangerConfig;
class URuleRangerContentBrowserExtensions;
class URuleRangerRuleSet;
class URuleRangerProjectRule;
class URuleRangerProjectActionContext;
class IRuleRangerProjectResultHandler;

// Shape of function called to check whether rule will run or actually execute rule.
// The actual function is determined by where it is used.
using FRuleRangerRuleFn = std::function<
    bool(URuleRangerConfig* const Config, URuleRangerRuleSet* const RuleSet, URuleRangerRule* Rule, UObject* InObject)>;

// Shape of function called to check whether project rule will run or actually execute project rule.
using FRuleRangerProjectRuleFn = std::function<
    bool(URuleRangerConfig* const Config, URuleRangerRuleSet* const RuleSet, URuleRangerProjectRule* Rule)>;

/**
 * The subsystem responsible for managing callbacks to other subsystems such as ImportSubsystem callbacks.
 */
UCLASS(CollapseCategories)
class URuleRangerEditorSubsystem final : public UEditorSubsystem
{
    GENERATED_BODY()

    friend URuleRangerContentBrowserExtensions;
    friend URuleRangerEditorValidator;

public:
    /** Implement this for initialization of instances of the system */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** Implement this for deinitialization of instances of the system */
    virtual void Deinitialize() override;

    void ScanObject(UObject* InObject, IRuleRangerResultHandler* InResultHandler = nullptr);

    void ScanAndFixObject(UObject* InObject, IRuleRangerResultHandler* InResultHandler = nullptr);

    void ValidateObject(UObject* InObject, bool bIsSave, IRuleRangerResultHandler* InResultHandler = nullptr);

    bool CanValidateObject(UObject* InObject, bool bIsSave);

    void MarkRuleSetConfigCacheDirty();

    IRuleRangerResultHandler* GetDefaultResultHandler() const;

    void OnScanSelectedAssets(const TArray<FAssetData>& Assets);

    void OnFixSelectedAssets(const TArray<FAssetData>& Assets);

    void OnScanSelectedPaths(const TArray<FString>& Paths);

    void CollectConfiguredPaths(TArray<FString>& OutPaths) const;

    void CollectAssetsFromPaths(const TArray<FString>& AssetPaths, TArray<FAssetData>& Assets);

    void OnFixSelectedPaths(const TArray<FString>& Paths);

    void OnScanContent();

    void OnFixContent();

    // Execute project-level rules across all configured RuleRangerConfigs
    void OnScanProject();

    // Execute and fix project-level rules across all configured RuleRangerConfigs
    void OnFixProject();

    // Run project-level rules and report to a custom handler (tool-only path)
    void RunProjectScan(bool bFix, IRuleRangerProjectResultHandler* ProjectHandler);

    // Returns true if any project rules are discoverable via configured RuleSets
    bool HasAnyProjectRules() const;

    /**
     * Returns true if any RuleRangerConfig in developer settings has at least one non-empty directory
     * configured.
     */
    bool HasAnyConfiguredDirs() const;

private:
    UPROPERTY(Transient)
    TScriptInterface<IRuleRangerResultHandler> DefaultResultHandler{ nullptr };

    UPROPERTY(Transient)
    mutable TArray<TWeakObjectPtr<URuleRangerConfig>> CachedRuleSetConfigs;

    mutable bool bRuleSetConfigCacheDirty{ true };

    TConstArrayView<TWeakObjectPtr<URuleRangerConfig>> GetCachedRuleSetConfigs() const;

    UPROPERTY(Transient)
    URuleRangerActionContext* ActionContext{ nullptr };

    // Project-level action context used when applying URuleRangerProjectRule
    UPROPERTY(Transient)
    URuleRangerProjectActionContext* ProjectActionContext{ nullptr };

    // Cached results of Rule->Match(Object) calculated during CanValidate stage
    UPROPERTY(Transient)
    TWeakObjectPtr<UObject> ValidationMatchCacheObject{ nullptr };
    TMap<TWeakObjectPtr<URuleRangerRule>, bool> ValidationMatchCache;

    FDelegateHandle OnAssetPostImportDelegateHandle;

    void ClearValidationMatchCache();

    void ResetValidationMatchCache(UObject* InObject);

    void ProcessRule(UObject* Object, const FRuleRangerRuleFn& ProcessRuleFunction);

    void OnAssetPostImport(UFactory* Factory, UObject* Object);

    bool ProcessRuleSetForObject(URuleRangerConfig* const Config,
                                 URuleRangerRuleSet* const RuleSet,
                                 const TArray<FRuleRangerRuleExclusion>& Exclusions,
                                 UObject* Object,
                                 const FRuleRangerRuleFn& ProcessRuleFunction,
                                 TSet<const URuleRangerRuleSet*>& Visited);

    bool CanValidateObject(const URuleRangerRule* Rule, const UObject* InObject, const bool bIsSave) const;

    /**
     * Function invoked when each rule is applied to an object.
     *
     * @param Config The config that transitively contained the Rule.
     * @param RuleSet The RuleSet that directly contained the Rule.
     * @param Rule The rule to apply.
     * @param InObject the object to apply rule to.
     * @param bIsSave flag indicating whether the validation is a result of a save.
     * @param InResultHandler the ResultHandler to use for reporting rule results.
     * @return true to keep processing, false if no more rules should be applied to object.
     */
    bool ProcessOnAssetValidateRule(URuleRangerConfig* const Config,
                                    URuleRangerRuleSet* const RuleSet,
                                    URuleRangerRule* Rule,
                                    UObject* InObject,
                                    const bool bIsSave,
                                    IRuleRangerResultHandler* InResultHandler) const;

    /**
     * Function invoked when each rule is applied to an object during import.
     *
     * @param Config The Config that transitively included this rule.
     * @param RuleSet The RuleSet that directly included this rule.
     * @param Rule The rule to apply.
     * @param bIsReimport A flag indicating whether it is an import or re-import action.
     * @param InObject the object to apply rule to.
     * @param ResultHandler the ResultHandler to forward results to.
     * @return true to keep processing, false if no more rules should be applied to object.
     */
    bool ProcessOnAssetPostImportRule(URuleRangerConfig* const Config,
                                      URuleRangerRuleSet* const RuleSet,
                                      URuleRangerRule* Rule,
                                      const bool bIsReimport,
                                      UObject* InObject,
                                      IRuleRangerResultHandler* ResultHandler) const;

    /**
     * Function invoked when each rule is applied to an object when user requested an explicit scan.
     *
     * @param Config The RuleRangerConfig context in which to execute rules.
     * @param RuleSet The RuleSet that contains the Rule.
     * @param Rule The rule to apply.
     * @param InObject the object to apply rule to.
     * @param ResultHandler the ResultHandler to forward results to.
     * @return true to keep processing, false if no more rules should be applied to object.
     */
    bool ProcessDemandScan(URuleRangerConfig* const Config,
                           URuleRangerRuleSet* const RuleSet,
                           URuleRangerRule* Rule,
                           UObject* InObject,
                           IRuleRangerResultHandler* ResultHandler) const;

    /**
     * Function invoked when each rule is applied to an object when user requested an explicit scan and autofix.
     *
     * @param Config The RuleRangerConfig context in which to execute rules.
     * @param RuleSet The RuleSet that contains the Rule.
     * @param Rule The rule to apply.
     * @param InObject the object to apply rule to.
     * @param ResultHandler the ResultHandler to forward results to.
     * @return true to keep processing, false if no more rules should be applied to object.
     */
    bool ProcessDemandScanAndFix(URuleRangerConfig* const Config,
                                 URuleRangerRuleSet* const RuleSet,
                                 URuleRangerRule* Rule,
                                 UObject* InObject,
                                 IRuleRangerResultHandler* ResultHandler) const;

    // Project rule traversal and execution helpers
    void ProcessProjectRules(const FRuleRangerProjectRuleFn& ProcessRuleFunction);

    bool ProcessProjectRuleSet(URuleRangerConfig* const Config,
                               URuleRangerRuleSet* const RuleSet,
                               const FRuleRangerProjectRuleFn& ProcessRuleFunction,
                               TSet<const URuleRangerRuleSet*>& Visited);

    bool ProcessProjectDemandScan(URuleRangerConfig* const Config,
                                  URuleRangerRuleSet* const RuleSet,
                                  URuleRangerProjectRule* Rule,
                                  IRuleRangerProjectResultHandler* Handler = nullptr) const;

    bool ProcessProjectDemandScanAndFix(URuleRangerConfig* const Config,
                                        URuleRangerRuleSet* const RuleSet,
                                        URuleRangerProjectRule* Rule,
                                        IRuleRangerProjectResultHandler* Handler = nullptr) const;

    // Default project result handler used when no explicit handler supplied
    UPROPERTY(Transient)
    TScriptInterface<IRuleRangerProjectResultHandler> DefaultProjectResultHandler{ nullptr };

    // Utility used by Tools menu actions to count rules for progress UI
    static int32 CountProjectRulesInRuleSet(const URuleRangerRuleSet* RuleSet,
                                            TSet<const URuleRangerRuleSet*>& Visited);

    void ProcessAssetsCommon(const TArray<FAssetData>& Assets,
                             const FText& SlowTaskText,
                             const FText& StartAtText,
                             const FText& CancelText,
                             const FText& CompletedText,
                             const bool bFix);
};
