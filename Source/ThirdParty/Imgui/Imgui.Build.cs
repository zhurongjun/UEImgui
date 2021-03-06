// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class Imgui : ModuleRules
{
	public Imgui(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] 
			{
                Path.Combine(ModuleDirectory, "Public")
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			});
		
		PublicDefinitions.Add("IMGUI_DISABLE_OBSOLETE_FUNCTIONS");
		PublicDefinitions.Add("IMGUI_DEFINE_MATH_OPERATORS");
	}
}
