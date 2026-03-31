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
    #include "RuleRanger/Actions/Blueprint/EnsureBlueprintContainsNoUnlinkedNodesAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionTests
{
    UBlueprint*
    CreateBlueprintFixture(UClass* const ParentClass, const TCHAR* const PackageName, const TCHAR* const ObjectName)
    {
        const auto Blueprint = RuleRangerTests::NewBlueprint(ParentClass, PackageName, ObjectName);
        if (Blueprint)
        {
            RuleRangerTests::CompileBlueprint(Blueprint);
        }
        return Blueprint;
    }

    bool SetErrorOnLooseDefaultEvents(FAutomationTestBase& Test,
                                      UEnsureBlueprintContainsNoUnlinkedNodesAction* const Action,
                                      const bool bErrorOnLooseDefaultEvents)
    {
        return RuleRangerTests::SetBoolPropertyValue(Test,
                                                     Action,
                                                     TEXT("bErrorOnLooseDefaultEvents"),
                                                     bErrorOnLooseDefaultEvents);
    }
} // namespace RuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionErrorsForUnlinkedExecNodesTest,
                                 "RuleRanger.Actions.Blueprint.EnsureNoUnlinkedNodes.ErrorsForUnlinkedExecNodes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionErrorsForUnlinkedExecNodesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureBlueprintContainsNoUnlinkedNodesAction>();
    const auto Blueprint = RuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionTests::CreateBlueprintFixture(
        URuleRangerAutomationBlueprintParentObject::StaticClass(),
        TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/LooseNodes/Error"),
        TEXT("BP_LooseNodesError"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("LooseFunction"));
        const auto Node = Graph ? RuleRangerTests::AddGraphNode<URuleRangerAutomationExecGraphNode>(Graph) : nullptr;
        if (TestNotNull(TEXT("Function graph should be created"), Graph)
            && TestNotNull(TEXT("Exec node should be created"), Node))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestEqual(TEXT("Unlinked exec nodes should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should mention the function graph"),
                                                          TEXT("LooseFunction"));
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionSkipsGhostEventsWhenDisabledTest,
    "RuleRanger.Actions.Blueprint.EnsureNoUnlinkedNodes.SkipsGhostEventsWhenDisabled",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionSkipsGhostEventsWhenDisabledTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureBlueprintContainsNoUnlinkedNodesAction>();
    const auto Blueprint = RuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionTests::CreateBlueprintFixture(
        AActor::StaticClass(),
        TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/LooseNodes/GhostSkipped"),
        TEXT("BP_GhostSkipped"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        const auto Graph = RuleRangerTests::FindEventGraph(Blueprint);
        const auto EventNode =
            Graph ? RuleRangerTests::AddEventNode(Graph, TEXT("ReceiveTick"), AActor::StaticClass(), true) : nullptr;
        if (TestNotNull(TEXT("Event graph should exist"), Graph)
            && TestNotNull(TEXT("Ghost event should be created"), EventNode))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestTrue(TEXT("Ghost default events should be skipped when disabled"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty());
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionErrorsForGhostEventsWhenEnabledTest,
    "RuleRanger.Actions.Blueprint.EnsureNoUnlinkedNodes.ErrorsForGhostEventsWhenEnabled",
    RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionErrorsForGhostEventsWhenEnabledTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureBlueprintContainsNoUnlinkedNodesAction>();
    const auto Blueprint = RuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionTests::CreateBlueprintFixture(
        AActor::StaticClass(),
        TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/LooseNodes/GhostError"),
        TEXT("BP_GhostError"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint)
        && RuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionTests::SetErrorOnLooseDefaultEvents(*this,
                                                                                                     Action,
                                                                                                     true))
    {
        const auto Graph = RuleRangerTests::FindEventGraph(Blueprint);
        const auto EventNode =
            Graph ? RuleRangerTests::AddEventNode(Graph, TEXT("ReceiveTick"), AActor::StaticClass(), true) : nullptr;
        if (TestNotNull(TEXT("Event graph should exist"), Graph)
            && TestNotNull(TEXT("Ghost event should be created"), EventNode))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestTrue(TEXT("Ghost default events should add at least one error when enabled"),
                            Fixture.ActionContext->GetErrorMessages().Num() > 0)
                && RuleRangerTests::TestTextArrayContains(*this,
                                                          Fixture.ActionContext->GetErrorMessages(),
                                                          TEXT("The error should mention the default event"),
                                                          TEXT("default event node"));
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionAcceptsPureNodesTest,
                                 "RuleRanger.Actions.Blueprint.EnsureNoUnlinkedNodes.AcceptsPureNodes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionAcceptsPureNodesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureBlueprintContainsNoUnlinkedNodesAction>();
    const auto Blueprint = RuleRangerEnsureBlueprintContainsNoUnlinkedNodesActionTests::CreateBlueprintFixture(
        URuleRangerAutomationBlueprintParentObject::StaticClass(),
        TEXT("/Game/Developers/Tests/RuleRanger/Blueprint/LooseNodes/Pure"),
        TEXT("BP_PureNode"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        const auto Graph = RuleRangerTests::CreateFunctionGraph(Blueprint, TEXT("PureFunction"));
        const auto Node = Graph ? RuleRangerTests::AddGraphNode<URuleRangerAutomationPureGraphNode>(Graph) : nullptr;
        if (TestNotNull(TEXT("Function graph should be created"), Graph)
            && TestNotNull(TEXT("Pure node should be created"), Node))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestTrue(TEXT("Pure nodes should not produce unlinked-node errors"),
                            Fixture.ActionContext->GetErrorMessages().IsEmpty());
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

#endif
