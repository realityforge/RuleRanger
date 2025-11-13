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
#include "RuleRangerProjectResultHandler.h"
#include "RuleRangerToolProjectResultHandler.generated.h"

struct FRuleRangerRun;

UCLASS()
class URuleRangerToolProjectResultHandler final : public UObject, public IRuleRangerProjectResultHandler
{
    GENERATED_BODY()

public:
    void Init(const TWeakPtr<FRuleRangerRun>& InRun);

    virtual void OnProjectRuleApplied(URuleRangerProjectActionContext* Context) override;

    // Attach a progress tracker so we can tick progress per rule applied
    void AttachProgress(struct FScopedSlowTask* InProgress) { Progress = InProgress; }

private:
    TWeakPtr<FRuleRangerRun> Run{ nullptr };
    FScopedSlowTask* Progress{ nullptr };
};
