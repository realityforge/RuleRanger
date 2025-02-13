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
#include "OrMatcher.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OrMatcher)

bool UOrMatcher::Test_Implementation(UObject* Object) const
{
    if (IsValid(Object))
    {
        for (int32 i = 0; i < Matchers.Num(); i++)
        {
            if (auto Matcher = Matchers[i]; IsValid(Matcher))
            {
                if (Matcher->Test(Object))
                {
                    return true;
                }
            }
        }
    }
    return false;
}
