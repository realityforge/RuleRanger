﻿/*
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
#include "MetadataTagMatcher.h"
#include "Editor.h"
#include "Subsystems/EditorAssetSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MetadataTagMatcher)

bool UMetadataTagMatcher::Test_Implementation(UObject* Object) const
{
    if (Key != NAME_None && IsValid(Object))
    {
        if (const auto Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>())
        {
            return Subsystem->GetMetadataTag(Object, Key) == Value;
        }
    }
    return false;
}
