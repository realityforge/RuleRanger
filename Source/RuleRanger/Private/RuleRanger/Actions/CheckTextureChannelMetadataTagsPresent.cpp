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

#include "CheckTextureChannelMetadataTagsPresent.h"
#include "Editor.h"
#include "RuleRangerLogging.h"

void UCheckTextureChannelMetadataTagsPresent::Apply_Implementation(
    TScriptInterface<IRuleRangerActionContext>& ActionContext,
    UObject* Object)
{
    if (IsValid(Object))
    {
        if (const auto Texture = Cast<UTexture2D>(Object); !Texture)
        {
            UE_LOG(RuleRanger,
                   Error,
                   TEXT("CheckTextureChannelMetadataTagsPresent: Attempt to run on Object %s "
                        "that is not a Texture2D instance."),
                   *Object->GetName());
        }
        else
        {
            static const FName Channel1Name("Channel.R");
            static const FName Channel2Name("Channel.G");
            static const FName Channel3Name("Channel.B");
            static const FName Channel4Name("Channel.A");

            const float ChannelCount = GPixelFormats[Texture->GetPixelFormat()].NumComponents;

            constexpr bool Channel1MetadataTagExpected = true;
            const bool Channel2MetadataTagExpected = ChannelCount > 1;
            const bool Channel3MetadataTagExpected = ChannelCount > 2;
            const bool Channel4MetadataTagExpected = ChannelCount > 3;

            const bool Channel1MetadataTagPresent =
                Texture->GetPackage()->GetMetaData()->HasValue(Object, Channel1Name);
            const bool Channel2MetadataTagPresent =
                Texture->GetPackage()->GetMetaData()->HasValue(Object, Channel2Name);
            const bool Channel3MetadataTagPresent =
                Texture->GetPackage()->GetMetaData()->HasValue(Object, Channel3Name);
            // ReSharper disable once CppTooWideScopeInitStatement
            const bool Channel4MetadataTagPresent =
                Texture->GetPackage()->GetMetaData()->HasValue(Object, Channel4Name);

            if ((Channel1MetadataTagExpected != Channel1MetadataTagPresent)
                || (Channel2MetadataTagExpected != Channel2MetadataTagPresent)
                || (Channel3MetadataTagExpected != Channel3MetadataTagPresent)
                || (Channel4MetadataTagExpected != Channel4MetadataTagPresent))
            {
                const FText Expected =
                    NSLOCTEXT("RuleRanger", "CheckTextureChannelMetadataTagsPresent_Expected", "expected");
                const FText NotExpected =
                    NSLOCTEXT("RuleRanger", "CheckTextureChannelMetadataTagsPresent_NotExpected", "not expected");
                const FText Present =
                    NSLOCTEXT("RuleRanger", "CheckTextureChannelMetadataTagsPresent_Present", "present");
                const FText Missing =
                    NSLOCTEXT("RuleRanger", "CheckTextureChannelMetadataTagsPresent_Missing", "missing");

                FFormatNamedArguments Arguments;
                Arguments.Add(TEXT("ChannelCount"), FText::FromString(FString::FromInt(ChannelCount)));

                Arguments.Add(TEXT("Channel1Name"), FText::FromName(Channel1Name));
                Arguments.Add(TEXT("Channel2Name"), FText::FromName(Channel2Name));
                Arguments.Add(TEXT("Channel3Name"), FText::FromName(Channel3Name));
                Arguments.Add(TEXT("Channel4Name"), FText::FromName(Channel4Name));

                Arguments.Add(TEXT("Channel1Actual"), Channel1MetadataTagPresent ? Present : Missing);
                Arguments.Add(TEXT("Channel2Actual"), Channel2MetadataTagPresent ? Present : Missing);
                Arguments.Add(TEXT("Channel3Actual"), Channel3MetadataTagPresent ? Present : Missing);
                Arguments.Add(TEXT("Channel4Actual"), Channel4MetadataTagPresent ? Present : Missing);

                Arguments.Add(TEXT("Channel1Expected"), Expected);
                Arguments.Add(TEXT("Channel2Expected"), Channel2MetadataTagExpected ? Expected : NotExpected);
                Arguments.Add(TEXT("Channel3Expected"), Channel3MetadataTagExpected ? Expected : NotExpected);
                Arguments.Add(TEXT("Channel4Expected"), Channel4MetadataTagExpected ? Expected : NotExpected);

                const FText Message =
                    FText::Format(NSLOCTEXT("RuleRanger",
                                            "CheckTextureChannelMetadataTagsPresent_Missing_ChannelMetaData",
                                            "Texture is missing expected metadata about texture channels. "
                                            "The texture contains {ChannelCount} channels."
                                            "\n{Channel1Name} tag is {Channel1Expected} and is {Channel1Actual}"
                                            "\n{Channel2Name} tag is {Channel2Expected} and is {Channel2Actual}"
                                            "\n{Channel3Name} tag is {Channel3Expected} and is {Channel3Actual}"
                                            "\n{Channel4Name} tag is {Channel4Expected} and is {Channel4Actual}"),
                                  Arguments);
                ActionContext->Error(Message);
            }
        }
    }
}
