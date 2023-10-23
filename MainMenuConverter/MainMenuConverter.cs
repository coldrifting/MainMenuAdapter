using nifly;
using SharpCompress.Archives;
using SharpCompress.Common;

namespace MainMenuConverter;

public static class MainMenuConvert
{
    public static int Main(string[] args)
    {
        if (args.Length < 1)
        {
            Error("No Mod Specified.");
            return 1;
        }

        try
        {
            string archivePath = Path.GetFullPath(args[0]);
            if (!File.Exists(archivePath))
            {
                Error($"Could not find the mod archive: {args[0]}");
                return 1;
            }

            Console.WriteLine("Please enter a name for this main menu background");
            string modId = Console.ReadLine() ?? "Default";

            string tempDir;
            {
                using var archive = ArchiveFactory.Open(archivePath);

                // Extract to temp folder
                tempDir = Path.GetTempPath() + @"\" + modId;
                archive.ExtractToDirectory(tempDir);
            }

            // Search for logo.nif
            string logoPath = "";
            if (!FindFile(tempDir, "logo.nif", ref logoPath))
            {
                if (!FindFile(tempDir, "logo01ae.nif", ref logoPath))
                {
                    Error("Unable to find main menu mesh in mod.");
                    return 1;
                }
            }

            NifFile nif = new();
            if (nif.Load(logoPath) != 0)
            {
                Error($"Unable to load {logoPath}. Is it a valid nif file?");
                return 1;
            }

            // Save original texture and replace it with new path
            List<NiShape> x = nif.GetShapes().ToList();
            string oldTexPath = "";
            foreach (NiShape shape in x)
            {
                oldTexPath = nif.GetTexturePathByIndex(shape, 0);
                nif.SetTextureSlot(shape, "Interface/MainMenu/" + modId + ".dds", 0);
            }

            // Save nif
            string newMeshFolder = tempDir + @"\Data\Data\Meshes\Interface\MainMenu";
            string newMeshFile = newMeshFolder + @"\" + modId + ".nif";
            Directory.CreateDirectory(newMeshFolder);

            if (nif.Save(newMeshFile) != 0)
            {
                Error($"Unable to save mesh to path: {newMeshFile}. Exiting...");
                return 1;
            }

            // Find old texture in mod
            string oldTexFile = "";
            string oldTexFilename = Path.GetFileName(Path.GetFullPath(oldTexPath));
            if (!FindFile(tempDir, oldTexFilename, ref oldTexFile)) {
                Error("Unable to find old texture file. Exiting...");
                return 1;
            }

            // Copy old texture file to new path
            string newTextureFolder = tempDir + @"\Data\Data\Textures\Interface\MainMenu";
            Directory.CreateDirectory(newTextureFolder);

            string newTextureFile = newTextureFolder + @"\" + modId + ".dds";
            File.Copy(oldTexFile, newTextureFile, true);

            {
                using var archiveNew = ArchiveFactory.Create(ArchiveType.Zip);
                archiveNew.AddAllFromDirectory($@"{tempDir}\Data");

                string outputDir = Directory.GetParent(archivePath)?.ToString() ??
                                   Environment.SpecialFolder.Desktop.ToString();

                archiveNew.SaveTo($@"{outputDir}\{modId}.zip", CompressionType.None);
            }

            Directory.Delete(tempDir, true);
            
            Console.WriteLine("Done!");
            Thread.Sleep(500);
        }
        catch (ArgumentException)
        {
            Error("Could not find the mod archive: " + args[0]);
            return 1;
        }

        return 0;
    }

    private static void Error(string msg)
    {
        Console.Error.WriteLine("ERROR: " + msg);
        Console.WriteLine("Press any key to exit...");
        Console.ReadKey();
    }

    private static bool FindFile(string dir, string fileName, ref string path)
    {
        foreach (string file in Directory.EnumerateFiles(dir, "*.*", SearchOption.AllDirectories))
        {
            if (!file.Contains(fileName)) 
                continue;
            
            path = file.Replace(@"\\", @"\");
            return true;
        }

        return false;
    }
}