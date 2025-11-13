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

#include "RuleRanger/UI/ToolTab/SRuleRangerToolPanel.h"
#include "AssetRegistry/AssetData.h"
#include "Editor.h"
#include "Misc/ScopedSlowTask.h"
#include "Modules/ModuleManager.h"
#include "RuleRanger/UI/RuleRangerDeveloperSettings.h"
#include "RuleRanger/UI/RuleRangerEditorSubsystem.h"
#include "RuleRanger/UI/RuleRangerStyle.h"
#include "RuleRanger/UI/RuleRangerTools.h"
#include "RuleRanger/UI/RuleRangerUIHelpers.h"
#include "RuleRanger/UI/ToolTab/RuleRangerToolProjectResultHandler.h"
#include "RuleRanger/UI/ToolTab/RuleRangerToolResultHandler.h"
#include "RuleRanger/UI/ToolTab/SRuleRangerRunView.h"
#include "RuleRangerConfig.h"
#include "RuleRangerProjectRule.h"
#include "RuleRangerRuleSet.h"
#include "Styling/AppStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/STextBlock.h"

void SRuleRangerToolPanel::Construct(const FArguments&)
{
    const auto ScanAllEnabled = [] { return FRuleRangerTools::CanRunScanAll(); };
    const auto ScanProjectEnabled = [] { return FRuleRangerTools::CanRunScanProject(); };
    const auto ScanContentEnabled = [] { return FRuleRangerTools::CanRunScanContent(); };
    const auto ScanSelectedEnabled = [] { return FRuleRangerTools::CanRunScanSelected(); };

    const auto OnScanAll = [this] {
        StartRun(NSLOCTEXT("RuleRanger", "Run_ScanAll", "Scan All"));
        const auto Run = Runs.IsValidIndex(ActiveRunIndex) ? Runs[ActiveRunIndex] : nullptr;
        RunProjectScan(Run, /*bFix*/ false);
        RunContentScan(Run, /*bFix*/ false);
        RebuildRunContents();
        RebuildRunsUI();
        return FReply::Handled();
    };
    const auto OnFixAll = [this] {
        StartRun(NSLOCTEXT("RuleRanger", "Run_FixAll", "Fix All"));
        const auto Run = Runs.IsValidIndex(ActiveRunIndex) ? Runs[ActiveRunIndex] : nullptr;
        RunProjectScan(Run, /*bFix*/ true);
        RunContentScan(Run, /*bFix*/ true);
        RebuildRunContents();
        RebuildRunsUI();
        return FReply::Handled();
    };
    const auto OnScanProject = [this] {
        StartRun(NSLOCTEXT("RuleRanger", "Run_ScanProject", "Scan Project"));
        const auto Run = Runs.IsValidIndex(ActiveRunIndex) ? Runs[ActiveRunIndex] : nullptr;
        RunProjectScan(Run, /*bFix*/ false);
        RebuildRunContents();
        RebuildRunsUI();
        return FReply::Handled();
    };
    const auto OnFixProject = [this] {
        StartRun(NSLOCTEXT("RuleRanger", "Run_FixProject", "Fix Project"));
        const auto Run = Runs.IsValidIndex(ActiveRunIndex) ? Runs[ActiveRunIndex] : nullptr;
        RunProjectScan(Run, /*bFix*/ true);
        RebuildRunContents();
        RebuildRunsUI();
        return FReply::Handled();
    };
    const auto OnScanContent = [this] {
        StartRun(NSLOCTEXT("RuleRanger", "Run_ScanContent", "Scan Content"));
        const auto Run = Runs.IsValidIndex(ActiveRunIndex) ? Runs[ActiveRunIndex] : nullptr;
        if (Run.IsValid())
        {
            RunContentScan(Run, /*bFix*/ false);
        }
        RebuildRunContents();
        RebuildRunsUI();
        return FReply::Handled();
    };
    const auto OnFixContent = [this] {
        StartRun(NSLOCTEXT("RuleRanger", "Run_FixContent", "Fix Content"));
        const auto Run = Runs.IsValidIndex(ActiveRunIndex) ? Runs[ActiveRunIndex] : nullptr;
        if (Run.IsValid())
        {
            RunContentScan(Run, /*bFix*/ true);
        }
        RebuildRunContents();
        RebuildRunsUI();
        return FReply::Handled();
    };
    const auto OnScanSelected = [this] {
        StartRun(NSLOCTEXT("RuleRanger", "Run_ScanSelected", "Scan Selected"));
        const auto Run = Runs.IsValidIndex(ActiveRunIndex) ? Runs[ActiveRunIndex] : nullptr;
        if (Run.IsValid())
        {
            RunSelectedScan(Run, /*bFix*/ false);
        }
        RebuildRunContents();
        RebuildRunsUI();
        return FReply::Handled();
    };
    const auto OnFixSelected = [this] {
        StartRun(NSLOCTEXT("RuleRanger", "Run_FixSelected", "Fix Selected"));
        const auto Run = Runs.IsValidIndex(ActiveRunIndex) ? Runs[ActiveRunIndex] : nullptr;
        if (Run.IsValid())
        {
            RunSelectedScan(Run, /*bFix*/ true);
        }
        RebuildRunContents();
        RebuildRunsUI();
        return FReply::Handled();
    };

    ChildSlot[SNew(SBorder).Padding(FMargin(
        8.f))[SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()
                    [SNew(SScrollBox)
                         .Orientation(Orient_Horizontal)
                         .AllowOverscroll(EAllowOverscroll::No)
                         .AnimateWheelScrolling(true)
                         .ScrollBarAlwaysVisible(false)
                     + SScrollBox::Slot()
                         [SNew(SHorizontalBox)
                          + SHorizontalBox::Slot().AutoWidth().Padding(
                              FMargin(0.f, 2.f, 8.f, 2.f))[SNew(SBox).WidthOverride(
                              140.f)[SNew(SButton)
                                         .ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
                                         .OnClicked_Lambda(OnScanAll)
                                         .IsEnabled_Lambda(ScanAllEnabled)
                                         .ToolTipText(NSLOCTEXT("RuleRanger",
                                                                "ToolTip_ScanAll",
                                                                "Run project and content scans"))
                                             [RuleRangerUI::MakeIconLabel(
                                                 FRuleRangerStyle::GetScanIcon(),
                                                 NSLOCTEXT("RuleRanger", "ScanAll_Label", "Scan All"))]]]

                          + SHorizontalBox::Slot().AutoWidth().Padding(
                              FMargin(0.f, 2.f, 8.f, 2.f))[SNew(SBox).WidthOverride(
                              140.f)[SNew(SButton)
                                         .OnClicked_Lambda(OnFixAll)
                                         .IsEnabled_Lambda(ScanAllEnabled)
                                         .ToolTipText(NSLOCTEXT("RuleRanger",
                                                                "ToolTip_FixAll",
                                                                "Run project and content scans, applying fixes"))
                                             [RuleRangerUI::MakeIconLabel(
                                                 FRuleRangerStyle::GetScanAndFixIcon(),
                                                 NSLOCTEXT("RuleRanger", "FixAll_Label", "Fix All"))]]]

                          + SHorizontalBox::Slot().AutoWidth().Padding(
                              FMargin(0.f, 2.f, 8.f, 2.f))[SNew(SBox).WidthOverride(
                              140.f)[SNew(SButton)
                                         .OnClicked_Lambda(OnScanProject)
                                         .IsEnabled_Lambda(ScanProjectEnabled)
                                         .ToolTipText(NSLOCTEXT("RuleRanger",
                                                                "ToolTip_ScanProject",
                                                                "Execute project-level rules"))
                                             [RuleRangerUI::MakeIconLabel(
                                                 FRuleRangerStyle::GetScanIcon(),
                                                 NSLOCTEXT("RuleRanger", "ScanProject_Label", "Scan Project"))]]]

                          + SHorizontalBox::Slot().AutoWidth().Padding(
                              FMargin(0.f, 2.f, 8.f, 2.f))[SNew(SBox).WidthOverride(
                              140.f)[SNew(SButton)
                                         .OnClicked_Lambda(OnFixProject)
                                         .IsEnabled_Lambda(ScanProjectEnabled)
                                         .ToolTipText(NSLOCTEXT("RuleRanger",
                                                                "ToolTip_FixProject",
                                                                "Execute project-level rules and apply fixes"))
                                             [RuleRangerUI::MakeIconLabel(
                                                 FRuleRangerStyle::GetScanAndFixIcon(),
                                                 NSLOCTEXT("RuleRanger", "FixProject_Label", "Fix Project"))]]]

                          + SHorizontalBox::Slot().AutoWidth().Padding(
                              FMargin(0.f, 2.f, 8.f, 2.f))[SNew(SBox).WidthOverride(
                              140.f)[SNew(SButton)
                                         .OnClicked_Lambda(OnScanContent)
                                         .IsEnabled_Lambda(ScanContentEnabled)
                                         .ToolTipText(NSLOCTEXT("RuleRanger",
                                                                "ToolTip_ScanAllContent",
                                                                "Scan all configured content"))
                                             [RuleRangerUI::MakeIconLabel(
                                                 FRuleRangerStyle::GetScanIcon(),
                                                 NSLOCTEXT("RuleRanger", "ScanAllContent_Label", "Scan Content"))]]]

                          + SHorizontalBox::Slot().AutoWidth().Padding(
                              FMargin(0.f, 2.f, 8.f, 2.f))[SNew(SBox).WidthOverride(
                              140.f)[SNew(SButton)
                                         .OnClicked_Lambda(OnFixContent)
                                         .IsEnabled_Lambda(ScanContentEnabled)
                                         .ToolTipText(NSLOCTEXT("RuleRanger",
                                                                "ToolTip_FixAllContent",
                                                                "Scan all configured content and apply fixes"))
                                             [RuleRangerUI::MakeIconLabel(
                                                 FRuleRangerStyle::GetScanAndFixIcon(),
                                                 NSLOCTEXT("RuleRanger", "FixAllContent_Label", "Fix Content"))]]]

                          + SHorizontalBox::Slot().AutoWidth().Padding(
                              FMargin(0.f, 2.f, 8.f, 2.f))[SNew(SBox).WidthOverride(
                              140.f)[SNew(SButton)
                                         .OnClicked_Lambda(OnScanSelected)
                                         .IsEnabled_Lambda(ScanSelectedEnabled)
                                         .ToolTipText(NSLOCTEXT("RuleRanger",
                                                                "ToolTip_ScanSelected",
                                                                "Scan selected assets/paths within configured content"))
                                             [RuleRangerUI::MakeIconLabel(
                                                 FRuleRangerStyle::GetScanIcon(),
                                                 NSLOCTEXT("RuleRanger", "ScanSelected_Label", "Scan Selected"))]]]

                          + SHorizontalBox::Slot().AutoWidth().Padding(
                              FMargin(0.f, 2.f, 8.f, 2.f))[SNew(SBox).WidthOverride(
                              140.f)[SNew(SButton)
                                         .OnClicked_Lambda(OnFixSelected)
                                         .IsEnabled_Lambda(ScanSelectedEnabled)
                                         .ToolTipText(
                                             NSLOCTEXT("RuleRanger",
                                                       "ToolTip_FixSelected",
                                                       "Scan and fix selected assets/paths within configured content"))
                                             [RuleRangerUI::MakeIconLabel(
                                                 FRuleRangerStyle::GetScanAndFixIcon(),
                                                 NSLOCTEXT("RuleRanger", "FixSelected_Label", "Fix Selected"))]]]]]

              + SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 4.f, 0.f, 4.f))[SNew(SSeparator)]

              + SVerticalBox::Slot().FillHeight(1.f)[SNew(SBorder).Padding(FMargin(
                  4.f))[SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()
                              [SNew(SHorizontalBox)
                               + SHorizontalBox::Slot().FillWidth(1.f)[SAssignNew(RunTabBar, SHorizontalBox)]
                               + SHorizontalBox::Slot()
                                     .AutoWidth()
                                     .VAlign(VAlign_Center)
                                     .Padding(FMargin(8.f, 0.f, 0.f, 0.f))
                                         [SNew(SButton)
                                              .ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
                                              .OnClicked_Lambda([this] {
                                                  ClearAllRuns();
                                                  return FReply::Handled();
                                              })
                                              .IsEnabled_Lambda([this] {
                                                  return Runs.Num() > 0;
                                              })[SNew(STextBlock)
                                                     .Text(NSLOCTEXT("RuleRanger", "ClearRuns", "Clear All Runs"))]]]
                        + SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 4.f, 0.f, 4.f))[SNew(SSeparator)]
                        + SVerticalBox::Slot().FillHeight(1.f)[SAssignNew(RunContentSwitcher, SWidgetSwitcher)]]]]];

    RebuildRunsUI();
    RebuildRunContents();
}

void SRuleRangerToolPanel::StartRun(const FText& Title)
{
    const auto Run = MakeShared<FRuleRangerRun>();
    Run->Title = Title;
    Runs.Add(Run);
    ActiveRunIndex = Runs.Num() - 1;
    RebuildRunsUI();
    RebuildRunContents();
}

void SRuleRangerToolPanel::RebuildRunsUI()
{
    if (RunTabBar.IsValid())
    {
        RunTabBar->ClearChildren();
        for (auto Index = 0; Index < Runs.Num(); ++Index)
        {
            const auto bActive = ActiveRunIndex == Index;
            const auto Title = Runs[Index]->Title;
            auto ErrorCount{ 0 };
            auto WarningCount{ 0 };
            auto InfoCount{ 0 };
            for (const auto& Row : Runs[Index]->Messages)
            {
                switch (Row->Severity)
                {
                    case ERuleRangerToolSeverity::Error:
                        ++ErrorCount;
                        break;
                    case ERuleRangerToolSeverity::Warning:
                        ++WarningCount;
                        break;
                    case ERuleRangerToolSeverity::Info:
                    default:
                        ++InfoCount;
                        break;
                }
            }
            auto LabelStr = Title.ToString();
            if (ErrorCount + WarningCount + InfoCount > 0)
            {
                TArray<FString> Parts;
                if (ErrorCount > 0)
                {
                    Parts.Add(FString::Printf(TEXT("E%d"), ErrorCount));
                }
                if (WarningCount > 0)
                {
                    Parts.Add(FString::Printf(TEXT("W%d"), WarningCount));
                }
                if (InfoCount > 0)
                {
                    Parts.Add(FString::Printf(TEXT("I%d"), InfoCount));
                }
                LabelStr += TEXT(" (") + FString::Join(Parts, TEXT(" ")) + TEXT(")");
            }
            const auto Label = FText::FromString(LabelStr);
            RunTabBar->AddSlot().AutoWidth().Padding(FMargin(0.f, 0.f, 8.f, 0.f))
                [SNew(SHorizontalBox)
                 + SHorizontalBox::Slot().AutoWidth()[SNew(SButton)
                                                          .ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>(
                                                              bActive ? TEXT("PrimaryButton") : TEXT("Button")))
                                                          .OnClicked_Lambda([this, Index] {
                                                              ActiveRunIndex = Index;
                                                              if (RunContentSwitcher.IsValid())
                                                              {
                                                                  RunContentSwitcher->SetActiveWidgetIndex(
                                                                      ActiveRunIndex);
                                                              }
                                                              RebuildRunsUI();
                                                              return FReply::Handled();
                                                          })[SNew(STextBlock).Text(Label)]]
                 + SHorizontalBox::Slot()
                       .AutoWidth()
                       .Padding(FMargin(4.f, 0.f, 0.f, 0.f))
                       .VAlign(VAlign_Center)[SNew(SButton)
                                                  .ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>(
                                                      TEXT("SimpleButton")))
                                                  .ContentPadding(FMargin(6.f, 2.f))
                                                  .ToolTipText(NSLOCTEXT("RuleRanger", "CloseRun", "Close this run"))
                                                  .OnClicked_Lambda([this, Index] {
                                                      CloseRunAt(Index);
                                                      return FReply::Handled();
                                                  })[SNew(STextBlock).Text(FText::FromString(TEXT("×")))]]];
        }
    }
}

void SRuleRangerToolPanel::RebuildRunContents()
{
    if (RunContentSwitcher.IsValid())
    {
        // Remove all current pages by stored refs
        for (const auto& Page : RunPageWidgets)
        {
            RunContentSwitcher->RemoveSlot(Page);
        }
        RunPageWidgets.Reset();
        for (auto Index = 0; Index < Runs.Num(); ++Index)
        {
            const TSharedRef<SWidget> Page = SNew(SRuleRangerRunView).Run(Runs[Index]);
            RunContentSwitcher->AddSlot()[Page];
            RunPageWidgets.Add(Page);
        }
        if (Runs.Num() > 0)
        {
            RunContentSwitcher->SetActiveWidgetIndex(FMath::Clamp(ActiveRunIndex, 0, Runs.Num() - 1));
        }
    }
}

void SRuleRangerToolPanel::CloseRunAt(const int32 Index)
{
    if (Runs.IsValidIndex(Index))
    {
        // Remove the run
        Runs.RemoveAt(Index);

        // Adjust active index
        if (Runs.IsEmpty())
        {
            ActiveRunIndex = INDEX_NONE;
        }
        else if (ActiveRunIndex == Index)
        {
            ActiveRunIndex = FMath::Clamp(Index, 0, Runs.Num() - 1);
        }
        else if (ActiveRunIndex > Index)
        {
            ActiveRunIndex -= 1;
        }

        // Rebuild UI
        RebuildRunContents();
        RebuildRunsUI();
        if (RunContentSwitcher.IsValid() && INDEX_NONE != ActiveRunIndex)
        {
            RunContentSwitcher->SetActiveWidgetIndex(ActiveRunIndex);
        }
    }
}

void SRuleRangerToolPanel::ClearAllRuns()
{
    Runs.Reset();
    ActiveRunIndex = INDEX_NONE;
    if (RunContentSwitcher.IsValid())
    {
        for (const auto& Page : RunPageWidgets)
        {
            RunContentSwitcher->RemoveSlot(Page);
        }
    }
    RunPageWidgets.Reset();
    RebuildRunsUI();
    RebuildRunContents();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void SRuleRangerToolPanel::RunProjectScan(const TSharedPtr<FRuleRangerRun>& Run, const bool bFix)
{
    if (Run.IsValid())
    {
        if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
        {
            // Compute total number of project rules to improve progress reporting
            int32 Total = 0;
            if (const auto DevSettings = GetDefault<URuleRangerDeveloperSettings>())
            {
                // Local counter to avoid introducing file-scope statics
                struct FCounter
                {
                    static int32 Count(const URuleRangerRuleSet* RuleSet, TSet<const URuleRangerRuleSet*>& Visited)
                    {
                        if (!IsValid(RuleSet) || Visited.Contains(RuleSet))
                        {
                            return 0;
                        }
                        else
                        {
                            Visited.Add(RuleSet);
                            int32 CountRules = 0;
                            for (const auto RulePtr : RuleSet->ProjectRules)
                            {
                                if (const auto Rule = RulePtr.Get(); IsValid(Rule))
                                {
                                    if (Rule->bApplyOnDemand)
                                    {
                                        ++CountRules;
                                    }
                                }
                            }
                            for (const auto NestedPtr : RuleSet->RuleSets)
                            {
                                CountRules += Count(NestedPtr.Get(), Visited);
                            }
                            return CountRules;
                        }
                    }
                };

                TSet<const URuleRangerRuleSet*> Visited;
                for (const auto& SoftConfig : DevSettings->Configs)
                {
                    if (const auto Config = SoftConfig.LoadSynchronous())
                    {
                        for (const auto& RuleSetPtr : Config->RuleSets)
                        {
                            Total += FCounter::Count(RuleSetPtr.Get(), Visited);
                        }
                    }
                }
            }

            FScopedSlowTask SlowTask(FMath::Max(1, Total),
                                     bFix ? NSLOCTEXT("RuleRanger", "ToolFixProject", "Rule Ranger: Scan & Fix Project")
                                          : NSLOCTEXT("RuleRanger", "ToolScanProject", "Rule Ranger: Scan Project"));
            SlowTask.MakeDialogDelayed(.5f, true);

            // Use TStrongObjectPtr to ensure handler survives GC during the run
            const TStrongObjectPtr ProjectHandler(NewObject<URuleRangerToolProjectResultHandler>(Subsystem));
            ProjectHandler->Init(Run);
            ProjectHandler->AttachProgress(&SlowTask);

            Subsystem->RunProjectScanCancellable(bFix, ProjectHandler.Get(), [&SlowTask] {
                return !SlowTask.ShouldCancel();
            });
        }
    }
}

void SRuleRangerToolPanel::RunContentScan(const TSharedPtr<FRuleRangerRun>& Run, const bool bFix)
{
    if (Run.IsValid())
    {
        TArray<FAssetData> Assets;
        FRuleRangerTools::CollectAllConfiguredAssets(Assets);

        RunAssetScan(Run, bFix, Assets);
    }
}

void SRuleRangerToolPanel::RunSelectedScan(const TSharedPtr<FRuleRangerRun>& Run, const bool bFix)
{
    if (Run.IsValid())
    {
        TArray<FAssetData> Assets;
        FRuleRangerTools::CollectAllSelectedAssets(Assets);

        RunAssetScan(Run, bFix, Assets);
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void SRuleRangerToolPanel::RunAssetScan(const TSharedPtr<FRuleRangerRun>& Run,
                                        const bool bFix,
                                        TArray<FAssetData> Assets)
{
    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        FScopedSlowTask SlowTask(Assets.Num(),
                                 bFix ? NSLOCTEXT("RuleRanger", "ToolFixAssets", "Rule Ranger: Scan & Fix Assets")
                                      : NSLOCTEXT("RuleRanger", "ToolScanAssets", "Rule Ranger: Scan Assets"));
        SlowTask.MakeDialogDelayed(.5f, true);

        // Use TStrongObjectPtr to avoid GC collecting handler during the run
        const TStrongObjectPtr Handler(NewObject<URuleRangerToolResultHandler>(Subsystem));
        Handler->Init(Run);

        for (const auto& Asset : Assets)
        {
            if (SlowTask.ShouldCancel())
            {
                break;
            }
            else
            {
                if (const auto Object = Asset.GetAsset())
                {
                    if (bFix)
                    {
                        Subsystem->ScanAndFixObject(Object, Handler.Get());
                    }
                    else
                    {
                        Subsystem->ScanObject(Object, Handler.Get());
                    }
                }
                SlowTask.EnterProgressFrame();
                SlowTask.TickProgress();
            }
        }
    }
}
