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

namespace RuleRanger::SortUtils
{
    /**
     * Remove nullptr entries and sort by UObject::GetName() (case-sensitive).
     */
    template <typename T>
    void RemoveNullsAndSortByName(TArray<TObjectPtr<T>>& Array)
    {
        Array.RemoveAll([](const TObjectPtr<T>& Value) { return Value == nullptr; });
        // Use comparator on the underlying object references to avoid deprecated TObjectPtr-by-ref path
        Array.Sort([](const T& A, const T& B) { return A.GetName() < B.GetName(); });
    }

    /**
     * Remove entries with empty Path and sort remaining by Path (case-sensitive).
     */
    inline void SortDirsByPath(TArray<FDirectoryPath>& Dirs)
    {
        Dirs.RemoveAll([](const FDirectoryPath& Dir) { return Dir.Path.IsEmpty(); });
        Dirs.Sort([](const FDirectoryPath& A, const FDirectoryPath& B) { return A.Path < B.Path; });
    }
} // namespace RuleRanger::SortUtils
