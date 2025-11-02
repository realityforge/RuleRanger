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

using UnrealBuildTool;

public class RuleRanger : ModuleRules
{
    public RuleRanger(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "ContentBrowser", "Core" });

        PrivateDependencyModuleNames.AddRange(new[] {
            "AssetRegistry",  "AnimGraph",         "AssetTools",      "CoreUObject",   "BlueprintGraph",
            "DataValidation", "DeveloperSettings", "EditorSubsystem", "Engine",        "Json",
            "Kismet",         "Projects",          "MessageLog",      "NiagaraEditor", "Niagara",
            "Slate",          "SlateCore",         "Settings",        "ToolMenus",     "UnrealEd",
        });

        PrivateIncludePathModuleNames.Add("MessageLog");

        if (IsPluginEnabled("MetaSound"))
        {
            PublicDefinitions.Add("WITH_RULERANGER_METASOUND_RULES=1");
            PrivateDependencyModuleNames.Add("MetasoundEngine");
            PrivateDependencyModuleNames.Add("MetasoundFrontend");
        }
        else
        {
            PublicDefinitions.Add("WITH_RULERANGER_METASOUND_RULES=0");
        }
    }

    private static bool IsPluginEnabled(string PluginName)
    {
        return Plugins.GetPlugin(PluginName) is not null;
    }
}
