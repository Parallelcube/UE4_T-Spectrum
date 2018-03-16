// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class Tutorial_spectrum : ModuleRules
{
	public Tutorial_spectrum(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
		PrivateDependencyModuleNames.AddRange(new string[] {  });

		var basePath = Path.GetDirectoryName(RulesCompiler.GetFileNameFromType(GetType()));
        string thirdPartyPath = Path.Combine(basePath, "..", "..", "Thirdparty");

        
        //FMOD
        PublicIncludePaths.Add(Path.Combine(thirdPartyPath, "FMOD", "Includes"));
   
        switch (Target.Platform)
        {
            case UnrealTargetPlatform.Win64:
                PublicLibraryPaths.Add(Path.Combine(thirdPartyPath, "FMOD", "Libraries", "Win64"));
                PublicAdditionalLibraries.Add("fmod64_vc.lib");
                string fmodDllPath = Path.Combine(thirdPartyPath, "FMOD", "Libraries", "Win64", "fmod64.dll");
                RuntimeDependencies.Add(new RuntimeDependency(fmodDllPath));

                string binariesDir = Path.Combine(basePath,"..","..", "Binaries", "Win64");
                if (!Directory.Exists(binariesDir))
                    System.IO.Directory.CreateDirectory(binariesDir);

                string fmodDllDest = System.IO.Path.Combine(binariesDir, "fmod64.dll");
                CopyFile(fmodDllPath, fmodDllDest);
                PublicDelayLoadDLLs.AddRange(new string[] { "fmod64.dll" });

                break;
            case UnrealTargetPlatform.Android:
                PublicLibraryPaths.Add(Path.Combine(thirdPartyPath, "FMOD", "Libraries", "Android", "armeabi-v7a"));
                PublicLibraryPaths.Add(Path.Combine(thirdPartyPath, "FMOD", "Libraries", "Android", "arm64-v8a"));
                PublicAdditionalLibraries.Add("fmod");
                string RelAPLPath = Utils.MakePathRelativeTo(System.IO.Path.Combine(thirdPartyPath, "FMOD", "FMOD_APL.xml"), Target.RelativeEnginePath);
                AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin", RelAPLPath));
                break;
            default:
                throw new System.Exception(System.String.Format("Unsupported platform {0}", Target.Platform.ToString()));
        }
    }


    private void CopyFile(string source, string dest)
    {
        System.Console.WriteLine("Copying {0} to {1}", source, dest);
        if (System.IO.File.Exists(dest))
        {
            System.IO.File.SetAttributes(dest, System.IO.File.GetAttributes(dest) & ~System.IO.FileAttributes.ReadOnly);
        }
        try
        {
            System.IO.File.Copy(source, dest, true);
        }
        catch (System.Exception ex)
        {
            System.Console.WriteLine("Failed to copy file: {0}", ex.Message);
        }
    }
}
