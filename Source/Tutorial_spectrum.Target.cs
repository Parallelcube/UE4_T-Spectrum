// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class Tutorial_spectrumTarget : TargetRules
{
	public Tutorial_spectrumTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "Tutorial_spectrum" } );
	}
}
