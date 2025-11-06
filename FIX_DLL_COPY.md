# Fix for snap7.dll Copy Issue

## Problem
The application was failing to start because the wrong version of `snap7.dll` was in the output directory.

## Solution Applied
The correct `snap7.dll` (265,728 bytes) has been manually copied to `x64\Debug\snap7.dll`.

## Permanent Fix Required
You need to add a post-build event to the Visual Studio project to automatically copy the correct DLL after each build.

### Steps to Add Post-Build Event:

1. **Close Visual Studio** (the project file needs to be closed)

2. Open `S7Server\S7Server.vcxproj` in a text editor

3. Find the `<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">` section

4. Before the closing `</ItemDefinitionGroup>` tag, add:
   ```xml
   <PostBuildEvent>
     <Command>xcopy /y /d "$(ProjectDir)snap7\snap7.dll" "$(OutDir)"</Command>
     <Message>Copying snap7.dll to output directory...</Message>
   </PostBuildEvent>
   ```

5. Find the `<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">` section

6. Before the closing `</ItemDefinitionGroup>` tag, add the same post-build event:
   ```xml
   <PostBuildEvent>
     <Command>xcopy /y /d "$(ProjectDir)snap7\snap7.dll" "$(OutDir)"</Command>
     <Message>Copying snap7.dll to output directory...</Message>
   </PostBuildEvent>
 ```

7. Save the file and reopen Visual Studio

## Alternative: Add via Visual Studio

1. Right-click on the **S7Server** project in Solution Explorer
2. Select **Properties**
3. Go to **Configuration Properties > Build Events > Post-Build Event**
4. In **Command Line**, add:
   ```
 xcopy /y /d "$(ProjectDir)snap7\snap7.dll" "$(OutDir)"
   ```
5. In **Description**, add:
   ```
   Copying snap7.dll to output directory...
   ```
6. Make sure to do this for both **Debug** and **Release** configurations

## Running the Application

The application requires **Administrator privileges** to bind to port 102 (the standard S7 port).

To run:
1. Right-click on `x64\Debug\S7Server.exe`
2. Select **Run as administrator**

Or run Visual Studio as Administrator and debug from there.
