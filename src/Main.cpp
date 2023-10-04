#pragma once

#include <iostream>
#include <filesystem>
#include <NifFile.hpp>
#include <bit7z/bitarchivewriter.hpp>
#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitfilecompressor.hpp>

using std::string;
using std::vector;
using std::cout;
using std::cin;
using std::endl;

namespace stdfs = std::filesystem;

using namespace nifly;
using namespace bit7z;

string zipDllPath = "7zip.dll";

void error(string msg) {
    cout << msg << endl;
    system("pause");
}

bool compareCaseInsensitive(string s1, string s2) {
    transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
    transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
    if (s1.compare(s2) == 0)
        return true;

    return false;
}

bool findFile(string dir, string fileName, string& path ) {
    bool found = false;
    string filePath;
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file() && compareCaseInsensitive(entry.path().filename().string(), fileName)) {
            path = entry.path().string();
            return true;
        }
    }

    return false;
}

bool extractFile(string filename, string extractPath) {
    try {
        Bit7zLibrary lib{ zipDllPath };
        BitArchiveReader archive{ lib, filename, BitFormat::Auto };

        archive.test();
        archive.extractTo(extractPath);
    }
    catch(const bit7z::BitException& ex) {
        return false;
    }
    return true;
}

bool zipFolder(string folderPath, string zipFileDest) {
    try { // bit7z classes can throw BitException objects
        Bit7zLibrary lib{ zipDllPath };
        BitArchiveWriter archive{ lib, BitFormat::Zip };

        string id = stdfs::path(folderPath).filename().string();

        // Adding the items to be compressed (no compression is performed here)
        //archive.addFile("path/to/file.txt");
        archive.addDirectory(folderPath);

        // Compressing the added items to the output archive
        archive.compressTo(zipFileDest);
    }
    catch (const bit7z::BitException& ex) {
        return false;
    }
    return true;
}

int main(int argc, char** args) {
    if (argc < 2) {
        error("No mod specified. Exiting...");
        return 1;
    }

    string exeFolder = stdfs::path(args[0]).remove_filename().string();
    zipDllPath = exeFolder + "7z.dll";

    if (!stdfs::exists(zipDllPath)) {
        zipDllPath = "C:\\Program Files\\7-Zip\\7z.dll";
        if (!stdfs::exists(zipDllPath)) {
            error("Could not find 7z.dll. Either redownload this program to get the bundled 7z.dll, or install 7zip.");
            return 1;
        }
    }

    // Get identifier for mod
    string modID;
    cout << "Please enter a name for this main menu background" << endl;
    getline(cin, modID);

    // Extract to temp folder
    string modArchive = args[1];
    string tempFolder = stdfs::temp_directory_path().string() + "\\" + modID;
    stdfs::create_directories(tempFolder);
    if (!extractFile(modArchive, tempFolder)) {
        error("Unable to extract mod archive: " + modArchive);
        return 1;
    }

    // Search for logo.nif
    string logoPath;
    if (!findFile(tempFolder, "logo.nif", logoPath)) {
        if (!findFile(tempFolder, "logo01ae.nif", logoPath)) {
            error("Unable to find main menu mesh in mod. Exiting...");
            return 1;
        }
    }

    // Load into nifly
    NifFile nif;
    if (nif.Load(logoPath)) {
        error("Unable to load " + logoPath + ". Is it a valid nif file? Exiting...");
        return 1;
    }

    // Save original texture and replace it with new path
    string oldTexPath;
    vector<NiShape*> x = nif.GetShapes();
    for each (NiShape * shape in x) {
        nif.GetTextureSlot(shape, oldTexPath, 0);
        nif.SetTextureSlot(shape, "Interface/MainMenu/" + modID + ".dds", 0);
    }

    // Save nif
    string newMeshFolder = tempFolder + "\\Data\\Meshes\\Interface\\MainMenu";
    string newMeshFile = newMeshFolder + "\\" + modID + ".nif";
    stdfs::create_directories(newMeshFolder);

    if (nif.Save(newMeshFile)) {
        error("Unable to save mesh to path: " + newMeshFile + ". Exiting...");
        return 1;
    }

    // Find old texture in mod
    string oldTexFile;
    string oldTexFilename = stdfs::path(oldTexPath).filename().string();
    if (!findFile(tempFolder, oldTexFilename, oldTexFile)) {
        error("Unable to find old texture file. Exiting...");
        return 1;
    }

    // Copy old texture file to new path
    string newTextureFolder = tempFolder + "\\Data\\Textures\\Interface\\MainMenu";
    stdfs::create_directories(newTextureFolder);

    string newTextureFile = newTextureFolder + "\\" + modID + ".dds";
    stdfs::copy_file(oldTexFile, newTextureFile, stdfs::copy_options::overwrite_existing);

    if (!zipFolder(tempFolder + "\\Data", "C:\\Users\\Coldrifting\\Desktop\\" + modID + ".zip")) {
        error("Unable to zip up mod contents. Exiting...");
        return 1;
    }

    stdfs::remove_all(tempFolder);
}