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
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "Misc/AutomationTest.h"
    #include "RuleRanger/UI/RuleRangerStyle.h"
    #include "RuleRanger/UI/ToolTab/SRuleRangerRunRow.h"
    #include "RuleRanger/UI/ToolTab/SRuleRangerRunView.h"
    #include "RuleRanger/UI/ToolTab/SRuleRangerToolPanel.h"
    #include "RuleRangerProjectRule.h"
    #include "RuleRangerRule.h"
    #include "RuleRangerRuleSet.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"
    #include "Widgets/Views/SHeaderRow.h"
    #include "Widgets/Views/SListView.h"
    #include "Widgets/Views/STableRow.h"

namespace RuleRangerToolTabSlateWidgetTests
{
    TSharedPtr<FRuleRangerMessageRow> MakeMessageRow(const ERuleRangerToolSeverity Severity, const TCHAR* const Message)
    {
        auto Row = MakeShared<FRuleRangerMessageRow>();
        Row->Severity = Severity;
        Row->Text = FText::FromString(Message);
        return Row;
    }

    bool TestWidgetVisible(FAutomationTestBase& Test, const TSharedRef<SWidget>& Widget, const TCHAR* const Description)
    {
        return Test.TestTrue(Description, Widget->GetVisibility().IsVisible());
    }

    TSharedRef<SListView<TSharedPtr<FRuleRangerMessageRow>>>
    MakeOwnerListView(TArray<TSharedPtr<FRuleRangerMessageRow>>& Items)
    {
        return SNew(SListView<TSharedPtr<FRuleRangerMessageRow>>)
            .ListItemsSource(&Items)
            .HeaderRow(SNew(SHeaderRow) + SHeaderRow::Column(TEXT("Severity")).FixedWidth(28.f)
                       + SHeaderRow::Column(TEXT("Asset")).FixedWidth(180.f) + SHeaderRow::Column(TEXT("Message"))
                       + SHeaderRow::Column(TEXT("RuleSet")).FixedWidth(180.f)
                       + SHeaderRow::Column(TEXT("Rule")).FixedWidth(180.f))
            .OnGenerateRow_Lambda(
                [](TSharedPtr<FRuleRangerMessageRow> InItem, const TSharedRef<STableViewBase>& OwnerTable) {
                    return SNew(STableRow<TSharedPtr<FRuleRangerMessageRow>>, OwnerTable);
                });
    }
} // namespace RuleRangerToolTabSlateWidgetTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRunRowConstructsSeverityAndColumnWidgetsTest,
                                 "RuleRanger.UI.ToolTab.SlateWidgets.RunRow.ConstructsSeverityAndColumnWidgets",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRunRowConstructsSeverityAndColumnWidgetsTest::RunTest(const FString&)
{
    const auto Asset = RuleRangerTests::NewNamedTransientObject<URuleRangerAutomationTestObject>(TEXT("RunRowAsset"));
    const auto RuleSet = RuleRangerTests::NewNamedTransientObject<URuleRangerRuleSet>(TEXT("RunRowRuleSet"));
    const auto Rule = RuleRangerTests::NewNamedTransientObject<URuleRangerRule>(TEXT("RunRowRule"));
    const auto ProjectRule =
        RuleRangerTests::NewNamedTransientObject<URuleRangerProjectRule>(TEXT("RunRowProjectRule"));
    if (!TestNotNull(TEXT("Asset should be created"), Asset) || !TestNotNull(TEXT("RuleSet should be created"), RuleSet)
        || !TestNotNull(TEXT("Rule should be created"), Rule)
        || !TestNotNull(TEXT("Project rule should be created"), ProjectRule))
    {
        return false;
    }

    TArray<TSharedPtr<FRuleRangerMessageRow>> Items;
    const auto Row =
        RuleRangerToolTabSlateWidgetTests::MakeMessageRow(ERuleRangerToolSeverity::Error, TEXT("Error row"));
    Row->Asset = Asset;
    Row->RuleSet = RuleSet;
    Row->Rule = Rule;
    Items.Add(Row);

    const auto ListView = RuleRangerToolTabSlateWidgetTests::MakeOwnerListView(Items);
    const auto RowWidget = SNew(SRuleRangerRunRow, ListView).Item(Row).ListView(ListView);

    return TestEqual(TEXT("Error severity text should be stable"),
                     SRuleRangerRunRow::GetSeverityTextForTest(ERuleRangerToolSeverity::Error).ToString(),
                     FString(TEXT("Error")))
        && TestEqual(TEXT("Warning severity text should be stable"),
                     SRuleRangerRunRow::GetSeverityTextForTest(ERuleRangerToolSeverity::Warning).ToString(),
                     FString(TEXT("Warning")))
        && TestEqual(TEXT("Info severity text should be stable"),
                     SRuleRangerRunRow::GetSeverityTextForTest(ERuleRangerToolSeverity::Info).ToString(),
                     FString(TEXT("Info")))
        && TestNotNull(TEXT("Error severity brush should resolve"),
                       SRuleRangerRunRow::GetSeverityBrushForTest(ERuleRangerToolSeverity::Error))
        && TestNotNull(TEXT("Warning severity brush should resolve"),
                       SRuleRangerRunRow::GetSeverityBrushForTest(ERuleRangerToolSeverity::Warning))
        && TestNotNull(TEXT("Info severity brush should resolve"),
                       SRuleRangerRunRow::GetSeverityBrushForTest(ERuleRangerToolSeverity::Info))
        && TestTrue(TEXT("Error severity color should be specified"),
                    SRuleRangerRunRow::GetSeverityTextColorForTest(ERuleRangerToolSeverity::Error).IsColorSpecified())
        && RuleRangerToolTabSlateWidgetTests::TestWidgetVisible(
               *this,
               RowWidget->GenerateWidgetForColumnForTest(TEXT("Severity")),
               TEXT("Severity column widget should be visible"))
        && RuleRangerToolTabSlateWidgetTests::TestWidgetVisible(
               *this,
               RowWidget->GenerateWidgetForColumnForTest(TEXT("Message")),
               TEXT("Message column widget should be visible"))
        && RuleRangerToolTabSlateWidgetTests::TestWidgetVisible(
               *this,
               RowWidget->GenerateWidgetForColumnForTest(TEXT("Asset")),
               TEXT("Asset column widget should be visible"))
        && RuleRangerToolTabSlateWidgetTests::TestWidgetVisible(*this,
                                                                RowWidget->GenerateWidgetForColumnForTest(TEXT("Rule")),
                                                                TEXT("Rule column widget should be visible"))
        && RuleRangerToolTabSlateWidgetTests::TestWidgetVisible(
               *this,
               RowWidget->GenerateWidgetForColumnForTest(TEXT("RuleSet")),
               TEXT("RuleSet column widget should be visible"))
        && RuleRangerToolTabSlateWidgetTests::TestWidgetVisible(
               *this,
               RowWidget->GenerateWidgetForColumnForTest(TEXT("Unknown")),
               TEXT("Unknown column fallback widget should be visible"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerRunViewFiltersSortsAndRefreshesRowsTest,
                                 "RuleRanger.UI.ToolTab.SlateWidgets.RunView.FiltersSortsAndRefreshesRows",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerRunViewFiltersSortsAndRefreshesRowsTest::RunTest(const FString&)
{
    const auto Run = MakeShared<FRuleRangerRun>();
    Run->Title = FText::FromString(TEXT("Smoke Run"));
    Run->Messages.Add(
        RuleRangerToolTabSlateWidgetTests::MakeMessageRow(ERuleRangerToolSeverity::Info, TEXT("Gamma passing detail")));
    Run->Messages.Add(RuleRangerToolTabSlateWidgetTests::MakeMessageRow(ERuleRangerToolSeverity::Warning,
                                                                        TEXT("Beta warning detail")));
    Run->Messages.Add(RuleRangerToolTabSlateWidgetTests::MakeMessageRow(ERuleRangerToolSeverity::Error,
                                                                        TEXT("Alpha failing detail")));

    const auto View = SNew(SRuleRangerRunView).Run(Run);
    View->SetSeverityFiltersForTest(true, true, true);

    const bool bConstructedWithAllRows =
        TestEqual(TEXT("Run view should include all severities when all filters are enabled"),
                  View->GetFilteredItemCountForTest(),
                  3);

    View->SetSearchQueryForTest(TEXT("warning"));
    const bool bSearchFiltersRows = TestEqual(TEXT("Search should reduce the filtered rows to matching messages"),
                                              View->GetFilteredItemCountForTest(),
                                              1)
        && TestEqual(TEXT("The matching row should be the warning message"),
                     View->GetFilteredItemForTest(0)->Text.ToString(),
                     FString(TEXT("Beta warning detail")));

    View->SetSearchQueryForTest(TEXT(""));
    View->SetSeverityFiltersForTest(false, true, false);
    const bool bSeverityFiltersRows =
        TestEqual(TEXT("Severity filters should hide non-warning rows"), View->GetFilteredItemCountForTest(), 1)
        && TestEqual(TEXT("The remaining row should be a warning"),
                     View->GetFilteredItemForTest(0)->Severity,
                     ERuleRangerToolSeverity::Warning);

    Run->Messages.Add(RuleRangerToolTabSlateWidgetTests::MakeMessageRow(ERuleRangerToolSeverity::Error,
                                                                        TEXT("Delta failing detail")));
    View->SetSeverityFiltersForTest(true, true, true);
    View->RebuildFilteredForTest();
    const bool bRepeatedRefreshIncludesNewRows =
        TestEqual(TEXT("Repeated refresh should include messages added after construction"),
                  View->GetFilteredItemCountForTest(),
                  4);

    View->SetSortForTest(TEXT("Message"), EColumnSortMode::Ascending);
    const bool bSortsByMessage = TestEqual(TEXT("Ascending message sort should place Alpha first"),
                                           View->GetFilteredItemForTest(0)->Text.ToString(),
                                           FString(TEXT("Alpha failing detail")))
        && TestEqual(TEXT("Ascending message sort should place Gamma last"),
                     View->GetFilteredItemForTest(3)->Text.ToString(),
                     FString(TEXT("Gamma passing detail")));

    TArray<TSharedPtr<FRuleRangerMessageRow>> OwnerItems;
    const auto OwnerListView = RuleRangerToolTabSlateWidgetTests::MakeOwnerListView(OwnerItems);
    const auto GeneratedRow = View->GenerateRowForTest(View->GetFilteredItemForTest(0), OwnerListView);
    const auto ColumnsMenu = View->BuildColumnsMenuForTest();
    const auto ContextMenu = View->OpenContextMenuForTest();

    return bConstructedWithAllRows && bSearchFiltersRows && bSeverityFiltersRows && bRepeatedRefreshIncludesNewRows
        && bSortsByMessage
        && TestTrue(TEXT("Run view should generate table rows for filtered items"),
                    GeneratedRow->AsWidget()->GetVisibility().IsVisible())
        && TestTrue(TEXT("Column menu should construct without rendering assertions"),
                    ColumnsMenu->GetVisibility().IsVisible())
        && TestTrue(TEXT("Context menu should construct for empty selection"),
                    ContextMenu.IsValid() && ContextMenu->GetVisibility().IsVisible());
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerToolPanelManagesRunsAndRepeatedRefreshesTest,
                                 "RuleRanger.UI.ToolTab.SlateWidgets.ToolPanel.ManagesRunsAndRepeatedRefreshes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerToolPanelManagesRunsAndRepeatedRefreshesTest::RunTest(const FString&)
{
    const auto Panel = SNew(SRuleRangerToolPanel);

    const bool bStartsEmpty = TestEqual(TEXT("Tool panel should start with no runs"), Panel->GetRunCountForTest(), 0)
        && TestEqual(TEXT("Tool panel should have no active run while empty"),
                     Panel->GetActiveRunIndexForTest(),
                     INDEX_NONE);

    Panel->RebuildRunsUIForTest();
    Panel->RebuildRunContentsForTest();
    Panel->StartRunForTest(FText::FromString(TEXT("Empty Smoke Run")));
    const auto FirstRun = Panel->GetRunForTest(0);
    if (!TestNotNull(TEXT("First run should be created"), FirstRun.Get()))
    {
        return false;
    }

    const bool bCreatesEmptyRun =
        TestEqual(TEXT("Starting a run should add a tab model"), Panel->GetRunCountForTest(), 1)
        && TestEqual(TEXT("The first run should become active"), Panel->GetActiveRunIndexForTest(), 0)
        && TestEqual(TEXT("New runs should start without messages"), FirstRun->Messages.Num(), 0);

    FirstRun->Messages.Add(
        RuleRangerToolTabSlateWidgetTests::MakeMessageRow(ERuleRangerToolSeverity::Warning, TEXT("Panel warning")));
    FirstRun->Messages.Add(
        RuleRangerToolTabSlateWidgetTests::MakeMessageRow(ERuleRangerToolSeverity::Error, TEXT("Panel error")));
    Panel->RebuildRunsUIForTest();
    Panel->RebuildRunContentsForTest();
    Panel->RebuildRunsUIForTest();
    Panel->RebuildRunContentsForTest();
    const bool bRepeatedRefreshPreservesRun =
        TestEqual(TEXT("Repeated panel refreshes should preserve the run model"), Panel->GetRunCountForTest(), 1)
        && TestEqual(TEXT("Repeated panel refreshes should preserve messages"), FirstRun->Messages.Num(), 2);

    Panel->StartRunForTest(FText::FromString(TEXT("Second Smoke Run")));
    const bool bSecondRunBecomesActive = TestEqual(TEXT("Second run should be added"), Panel->GetRunCountForTest(), 2)
        && TestEqual(TEXT("Second run should become active"), Panel->GetActiveRunIndexForTest(), 1);

    Panel->CloseRunAtForTest(0);
    const bool bCloseAdjustsActiveRun =
        TestEqual(TEXT("Closing a run should remove it"), Panel->GetRunCountForTest(), 1)
        && TestEqual(TEXT("Active run index should adjust after closing an earlier run"),
                     Panel->GetActiveRunIndexForTest(),
                     0);

    Panel->ClearAllRunsForTest();
    Panel->ClearAllRunsForTest();
    const bool bRepeatedClearReturnsToEmpty =
        TestEqual(TEXT("Repeated clear should leave no runs"), Panel->GetRunCountForTest(), 0)
        && TestEqual(TEXT("Repeated clear should leave no active run"), Panel->GetActiveRunIndexForTest(), INDEX_NONE);

    return bStartsEmpty && bCreatesEmptyRun && bRepeatedRefreshPreservesRun && bSecondRunBecomesActive
        && bCloseAdjustsActiveRun && bRepeatedClearReturnsToEmpty;
}

#endif
