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
    #include "RuleRanger/Actions/Blueprint/EnsureNoEmptyTickAction.h"
    #include "Tests/RuleRanger/RuleRangerAutomationTestHelpers.h"

namespace RuleRangerEnsureNoEmptyTickActionTests
{
    UBlueprint* CreateActorBlueprint(const TCHAR* const PackageName, const TCHAR* const ObjectName)
    {
        const auto Blueprint = RuleRangerTests::NewBlueprint(AActor::StaticClass(), PackageName, ObjectName);
        if (Blueprint)
        {
            RuleRangerTests::CompileBlueprint(Blueprint);
        }
        return Blueprint;
    }
} // namespace RuleRangerEnsureNoEmptyTickActionTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNoEmptyTickActionErrorsForEmptyTickNodesTest,
                                 "RuleRanger.Actions.Blueprint.EnsureNoEmptyTick.ErrorsForEmptyTickNodes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNoEmptyTickActionErrorsForEmptyTickNodesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNoEmptyTickAction>();
    const auto Blueprint = RuleRangerEnsureNoEmptyTickActionTests::CreateActorBlueprint(
        TEXT("/Game/Developers/RuleRangerTests/Blueprint/EmptyTick/Error"),
        TEXT("BP_EmptyTickError"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        const auto Graph = RuleRangerTests::FindEventGraph(Blueprint);
        const auto EventNode = Graph ? RuleRangerTests::AddEventNode(Graph, TEXT("ReceiveTick")) : nullptr;
        if (TestNotNull(TEXT("Event graph should exist"), Graph)
            && TestNotNull(TEXT("Tick event should be created"), EventNode))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestEqual(TEXT("Empty tick nodes should add one error"),
                             Fixture.ActionContext->GetErrorMessages().Num(),
                             1);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNoEmptyTickActionAcceptsLinkedTickNodesTest,
                                 "RuleRanger.Actions.Blueprint.EnsureNoEmptyTick.AcceptsLinkedTickNodes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNoEmptyTickActionAcceptsLinkedTickNodesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNoEmptyTickAction>();
    const auto Blueprint = RuleRangerEnsureNoEmptyTickActionTests::CreateActorBlueprint(
        TEXT("/Game/Developers/RuleRangerTests/Blueprint/EmptyTick/Linked"),
        TEXT("BP_EmptyTickLinked"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        const auto Graph = RuleRangerTests::FindEventGraph(Blueprint);
        const auto EventNode = Graph ? RuleRangerTests::AddEventNode(Graph, TEXT("ReceiveTick")) : nullptr;
        const auto ExecNode =
            Graph ? RuleRangerTests::AddGraphNode<URuleRangerAutomationExecGraphNode>(Graph) : nullptr;
        if (TestNotNull(TEXT("Event graph should exist"), Graph)
            && TestNotNull(TEXT("Tick event should be created"), EventNode)
            && TestNotNull(TEXT("Exec node should be created"), ExecNode))
        {
            RuleRangerTests::LinkPins(EventNode->FindPin(UEdGraphSchema_K2::PN_Then),
                                      ExecNode->FindPin(UEdGraphSchema_K2::PN_Execute));
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestTrue(TEXT("Linked tick nodes should not add errors"),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRuleRangerEnsureNoEmptyTickActionSkipsGhostTickNodesTest,
                                 "RuleRanger.Actions.Blueprint.EnsureNoEmptyTick.SkipsGhostTickNodes",
                                 RuleRangerTests::AutomationTestFlags)
bool FRuleRangerEnsureNoEmptyTickActionSkipsGhostTickNodesTest::RunTest(const FString&)
{
    const auto Action = RuleRangerTests::NewTransientObject<UEnsureNoEmptyTickAction>();
    const auto Blueprint = RuleRangerEnsureNoEmptyTickActionTests::CreateActorBlueprint(
        TEXT("/Game/Developers/RuleRangerTests/Blueprint/EmptyTick/Ghost"),
        TEXT("BP_EmptyTickGhost"));
    RuleRangerTests::FRuleFixture Fixture;
    if (RuleRangerTests::CreateRuleFixture(*this, Fixture) && TestNotNull(TEXT("Action should be created"), Action)
        && TestNotNull(TEXT("Blueprint should be created"), Blueprint))
    {
        const auto Graph = RuleRangerTests::FindEventGraph(Blueprint);
        const auto EventNode =
            Graph ? RuleRangerTests::AddEventNode(Graph, TEXT("ReceiveTick"), AActor::StaticClass(), true) : nullptr;
        if (TestNotNull(TEXT("Event graph should exist"), Graph)
            && TestNotNull(TEXT("Ghost tick event should be created"), EventNode))
        {
            RuleRangerTests::ResetRuleFixtureObject(Fixture, Blueprint);
            Action->Apply(Fixture.ActionContext, Blueprint);
            return TestTrue(TEXT("Automatically placed ghost tick nodes should be ignored"),
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
