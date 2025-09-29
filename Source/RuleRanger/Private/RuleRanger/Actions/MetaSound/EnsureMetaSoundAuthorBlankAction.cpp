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
#include "EnsureMetaSoundAuthorBlankAction.h"
#if WITH_RULERANGER_METASOUND_RULES
    #include "MetasoundSource.h"
#else
    #include "Sound/SoundWaveProcedural.h"
#endif
#include "RuleRangerActionContext.h"

UClass* UEnsureMetaSoundAuthorBlankAction::GetExpectedType()
{
#if WITH_RULERANGER_METASOUND_RULES
    return UMetaSoundSource::StaticClass();
#else
    return USoundWaveProcedural::StaticClass();
#endif
}

void UEnsureMetaSoundAuthorBlankAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
#if WITH_RULERANGER_METASOUND_RULES
    if (const auto MetaSound = Cast<UMetaSoundSource>(Object))
    {
        auto& Metadata = MetaSound->GetDocumentChecked().RootGraph.Metadata;
        if (!Metadata.GetAuthor().IsEmpty())
        {
            if (ActionContext->IsDryRun())
            {
                const auto Message = NSLOCTEXT(
                    "RuleRanger",
                    "MetaSoundHasNonBlankAuthor",
                    "MetaSoundSource has a non-blank Author property. This Author property would be cleared if RuleRanger was not in DryRun mode");
                ActionContext->Error(Message);
            }
            else
            {
                const auto Message =
                    NSLOCTEXT("RuleRanger",
                              "MetaSoundAuthorCleared",
                              "MetaSoundSource has a non-blank Author property. Clearing the Author property.");
                ActionContext->Info(Message);
                MetaSound->Modify();
                Metadata.SetAuthor("");
            }
        }
    }
#endif
}
