// Made by Jenn.zip

using UnrealBuildTool;

public class UnfocusedHotkeys : ModuleRules
{
	public UnfocusedHotkeys(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore"
		});
	}
}