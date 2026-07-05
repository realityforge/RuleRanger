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
#include "RuleRanger/MetaSound/MetaSoundPresetUtilities.h"
#include "AssetRegistry/AssetData.h"
#include "MetasoundAssetManager.h"
#include "MetasoundSource.h"

namespace RuleRanger::MetaSound
{
    bool IsPreset(const UMetaSoundSource* Source)
    {
        if (!Source)
        {
            return false;
        }

#if WITH_EDITORONLY_DATA
        bool bAssetDataIsValid = false;
        const FMetaSoundDocumentInfo AssetDocumentInfo(FAssetData(Source), bAssetDataIsValid);
        if (bAssetDataIsValid)
        {
            return AssetDocumentInfo.bIsPreset != 0;
        }

        const FMetaSoundDocumentInfo DocumentInfo(*Source);
        if (DocumentInfo.bIsPreset != 0)
        {
            return true;
        }
#endif

        PRAGMA_DISABLE_DEPRECATION_WARNINGS
        return Source->GetConstDocument().RootGraph.PresetOptions.bIsPreset;
        PRAGMA_ENABLE_DEPRECATION_WARNINGS
    }
} // namespace RuleRanger::MetaSound
