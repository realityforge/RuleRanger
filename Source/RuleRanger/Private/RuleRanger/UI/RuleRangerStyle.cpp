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
#include "RuleRangerStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Logging/StructuredLog.h"
#include "RuleRanger.h"
#include "RuleRangerLogging.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FRuleRangerStyle::StyleInstance = nullptr;

void FRuleRangerStyle::Initialize()
{
    if (!StyleInstance.IsValid())
    {
        UE_LOGFMT(LogRuleRanger, VeryVerbose, "FRuleRangerStyle::Initialize(): Creating StyleSet.");
        StyleInstance = Create();
        UE_LOGFMT(LogRuleRanger, VeryVerbose, "FRuleRangerStyle::Initialize(): Registering StyleSet.");
        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
    else
    {
        UE_LOGFMT(LogRuleRanger,
                  VeryVerbose,
                  "FRuleRangerStyle::Initialize(): Skipping "
                  "creation and registration of StyleSheet as already valid.");
    }
}

void FRuleRangerStyle::Shutdown()
{
    UE_LOGFMT(LogRuleRanger, VeryVerbose, "FRuleRangerStyle::Shutdown: Deregistering StyleSet.");
    FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
    ensure(StyleInstance.IsUnique());
    StyleInstance.Reset();
}

FName FRuleRangerStyle::GetStyleSetName()
{
    static FName StyleSetName(TEXT("RuleRangerStyle"));
    return StyleSetName;
}

FSlateIcon FRuleRangerStyle::GetSettingsIcon()
{
    return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Toolbar.Settings");
}

FSlateIcon FRuleRangerStyle::GetScanIcon()
{
    return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Symbols.SearchGlass");
}

FSlateIcon FRuleRangerStyle::GetScanAndFixIcon()
{
    return FSlateIcon(FAppStyle::GetAppStyleSetName(), "MessageLog.Fix");
}

FSlateIcon FRuleRangerStyle::GetEditAssetIcon()
{
    return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.Edit");
}

FSlateIcon FRuleRangerStyle::GetFindInContentBrowserIcon()
{
    return FSlateIcon(FAppStyle::GetAppStyleSetName(), "SystemWideCommands.FindInContentBrowser");
}

FSlateIcon FRuleRangerStyle::GetCopyMessageIcon()
{
    return FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCommands.Copy");
}

const FSlateBrush* FRuleRangerStyle::GetNoteMessageBrush()
{
    return FAppStyle::Get().GetBrush("MessageLog.Note");
}

const FSlateBrush* FRuleRangerStyle::GetWarningMessageBrush()
{
    return FAppStyle::Get().GetBrush("MessageLog.Warning");
}

const FSlateBrush* FRuleRangerStyle::GetErrorMessageBrush()
{
    return FAppStyle::Get().GetBrush("MessageLog.Error");
}

TSharedRef<FSlateStyleSet> FRuleRangerStyle::Create()
{
    const auto RuleRangerModuleName = FRuleRangerModule::GetModuleName();

    // The resources directory of the RuleRanger Plugin
    const auto ResourcesDir =
        IPluginManager::Get().FindPlugin(RuleRangerModuleName.ToString())->GetBaseDir() / TEXT("Resources");

    TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
    Style->SetContentRoot(ResourcesDir);

    return Style;
}

void FRuleRangerStyle::ReloadTextures()
{
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
    }
}

const ISlateStyle& FRuleRangerStyle::Get()
{
    return *StyleInstance;
}
