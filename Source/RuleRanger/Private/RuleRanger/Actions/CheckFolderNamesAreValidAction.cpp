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

#include "CheckFolderNamesAreValidAction.h"

void UCheckFolderNamesAreValidAction::Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object)
{
    if (IsValid(Object))
    {
        TArray<FString> Folders;
        Object->GetPathName().ParseIntoArray(Folders, TEXT("/"), true);

        const FRegexPattern Pattern(InvalidFolderRegexPattern,
                                    bCaseSensitive ? ERegexPatternFlags::None : ERegexPatternFlags::CaseInsensitive);

        for (const auto& Folder : Folders)
        {
            if (InvalidFolderNames.Contains(Folder)
                || (!InvalidFolderRegexPattern.IsEmpty() && FRegexMatcher(Pattern, Folder).FindNext()))
            {
                if (Message.IsEmpty())
                {
                    FFormatNamedArguments Arguments;
                    Arguments.Add(TEXT("Folder"), FText::FromString(Folder));
                    ActionContext->Error(
                        FText::Format(NSLOCTEXT("RuleRanger",
                                                "CheckFolderNamesAreValidAction_InvalidFolderName",
                                                "Object is contained in an folder with an invalid name {Folder}. "
                                                "Move the asset to a different folder."),
                                      Arguments));
                }
            }
            else
            {
                ActionContext->Error(FText::FromString(Message));
            }
        }
    }
}
