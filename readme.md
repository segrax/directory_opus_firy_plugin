# Directory Opus 12 Firy Plugin
[![Build status](https://ci.appveyor.com/api/projects/status/v2c5j33soyoynd3b?svg=true)](https://ci.appveyor.com/project/segrax/directory-opus-firy-plugin)


![image](https://user-images.githubusercontent.com/1327406/47679796-323dbc80-dc19-11e8-91f0-8b82d5a89257.png)

#### About

VFS Plugin which adds support for various disk image types

* ADF/HDF
* D64
* FAT 12/16/32 (Read only)


#### Requirements

* Directory Opus 12 for Windows
* Visual Studio 2022 or newer with the C++ desktop workload
* CMake 3.23 or newer

#### Building

Generate the Visual Studio solution:

```powershell
cmake --preset vs2022-x64
```

Open `build/vs2022-x64/openfiry.sln` in Visual Studio, or build the release DLL from the command line:

```powershell
cmake --build --preset vs2022-x64-release
```

The release DLL is written to `build/vs2022-x64/bin/Release/OPENFIRY.dll`.

#### Releases

The GitHub Actions workflow builds the x64 release DLL on pushes, pull requests, and manual runs. Pushing a tag like `v1.0.0` also publishes a GitHub release with the DLL and zip package attached.
