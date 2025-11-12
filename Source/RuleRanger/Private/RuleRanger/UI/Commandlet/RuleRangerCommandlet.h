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

#include "Commandlets/Commandlet.h"
#include "RuleRangerResultHandler.h"
#include "RuleRangerCommandlet.generated.h"

class URuleRangerConfig;
class URuleRangerRuleSet;
class URuleRangerProjectRule;
class URuleRangerProjectActionContext;

UCLASS()
class URuleRangerCommandlet final : public UCommandlet, public IRuleRangerResultHandler
{
    GENERATED_BODY()

    void CollectAssetsFromAllowlist(const TArray<FString>& AllowlistPaths, TArray<FAssetData>& Assets);
    void DeriveAllowlistPaths(const FString& Params, TArray<FString>& AllowlistPaths);
    void ResetState();
    void ExecuteProjectRules(bool bFix);
    bool ProcessProjectRuleSet(URuleRangerConfig* Config,
                               URuleRangerRuleSet* RuleSet,
                               URuleRangerProjectActionContext* ProjectContext,
                               bool bFix,
                               TSet<const URuleRangerRuleSet*>& Visited);

    FAssetData CurrentAsset;
    int32 NumErrors{ 0 };
    int32 NumWarnings{ 0 };
    int32 NumFatals{ 0 };
    int32 NumAssetsScanned{ 0 };
    int32 NumProjectRulesScanned{ 0 };

    TArray<TSharedPtr<FJsonValue>> AssetRuleResults;
    TArray<TSharedPtr<FJsonValue>> ProjectRuleResults;

public:
    URuleRangerCommandlet();

    virtual int32 Main(const FString& Params) override;

    virtual void OnRuleApplied(URuleRangerActionContext* ActionContext) override;
};
