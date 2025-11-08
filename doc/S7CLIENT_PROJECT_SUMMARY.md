# S7Client Project - Summary

## What Was Created

A complete Snap7 client test application to verify whether the 20-variable limit exists in the Snap7 library itself or if it's a Node-RED implementation issue.

## New Files Created

### 1. S7Client/main.cpp
The main client application that:
- Connects to an S7 server using Snap7 client library
- Performs 4 different read tests:
  - **Test 1**: 5 variables read individually (baseline)
  - **Test 2**: 20 variables using MultiRead
  - **Test 3**: 30 variables using MultiRead (exceeds supposed limit)
  - **Test 4**: 50 variables using MultiRead (stress test)
- Measures performance for each test
- Reports success/failure for each variable
- Provides a clear conclusion about whether a 20-variable limit exists

### 2. S7Client/S7Client.vcxproj
Visual Studio project file that:
- Configures C++14 compilation
- Links against snap7.lib
- Includes the Snap7 headers from the server project
- Automatically copies snap7.dll to output directory
- Supports both Debug and Release builds for x64

### 3. S7Client/S7Client.vcxproj.filters
Project filters for organizing files in Visual Studio

### 4. S7Client/README.md
Comprehensive documentation covering:
- Purpose and features
- Build instructions
- Usage examples
- Expected results interpretation
- Implementation details
- Troubleshooting guide

### 5. doc/CLIENT_TEST_QUICKSTART.md
Step-by-step guide for:
- Building both projects
- Preparing the server
- Running the test
- Interpreting results
- Common troubleshooting scenarios
- Advanced testing options

## Solution File Update

The `S7Server.sln` file needs to be updated to include the new S7Client project. Since it's currently open, you'll need to:

1. **Close the solution in Visual Studio**
2. **Manually edit S7Server.sln** to add these lines after the S7Server project:

```
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "S7Client", "S7Client\S7Client.vcxproj", "{B8F3D9E2-7A1C-4F5E-8D3C-9E4F5A6B7C8D}"
EndProject
```

And add these lines in the `GlobalSection(ProjectConfigurationPlatforms)` section:

```
{B8F3D9E2-7A1C-4F5E-8D3C-9E4F5A6B7C8D}.Debug|x64.ActiveCfg = Debug|x64
{B8F3D9E2-7A1C-4F5E-8D3C-9E4F5A6B7C8D}.Debug|x64.Build.0 = Debug|x64
{B8F3D9E2-7A1C-4F5E-8D3C-9E4F5A6B7C8D}.Release|x64.ActiveCfg = Release|x64
{B8F3D9E2-7A1C-4F5E-8D3C-9E4F5A6B7C8D}.Release|x64.Build.0 = Release|x64
```

**Or simply reopen Visual Studio and use "Add Existing Project" to add `S7Client\S7Client.vcxproj`**

## Key Features

### Smart Data Conversion
The client correctly handles S7 REAL data format (big-endian IEEE 754) and converts to native float format.

### Comprehensive Testing
Four different test scenarios systematically verify:
- Basic connectivity and data reading
- Whether 20 variables can be read
- Whether more than 20 variables can be read
- Stress testing with 50 variables

### Performance Measurement
Each test measures execution time, allowing comparison between:
- Individual reads (slower, multiple round-trips)
- Multi-variable reads (faster, single round-trip)

### Clear Results Interpretation
The application provides an automatic conclusion based on the test results, making it easy to determine if a limit exists.

## How It Works

### Connection
1. Creates a Snap7 client instance
2. Connects to the server using IP, rack, slot, and port
3. Displays PDU negotiation information

### Reading Data
Uses two Snap7 functions:
- **Cli_ReadArea**: Reads a single memory area (used in Test 1)
- **Cli_ReadMultiVars**: Reads multiple variables in one request (used in Tests 2-4)

### Data Processing
For each read:
1. Allocates buffer for the data
2. Performs the read operation
3. Checks the result code
4. Converts from S7 REAL to float if successful
5. Reports success/failure and the value

## Expected Outcome

Based on Snap7 documentation and typical S7 protocol behavior, we expect:
- **All tests should PASS** (no 20-variable limit in Snap7)
- This would confirm the limit is in Node-RED's implementation

If the limit exists in Snap7 itself:
- Test 2 (20 vars) would pass
- Tests 3 and 4 (30 and 50 vars) would fail or partially succeed

## Next Steps

1. **Close and reopen the solution** in Visual Studio to load the new project
2. **Build both projects** (S7Server and S7Client)
3. **Update address.csv** to ensure DB1 has at least 50 REAL values
4. **Run S7Server.exe** as Administrator
5. **Run S7Client.exe** in a separate terminal
6. **Review the results** to determine if the limit exists

## Integration with Existing Codebase

The client project:
- ? Uses the same Snap7 library as the server
- ? Shares the snap7.dll and snap7.lib files
- ? Follows the same C++14 standard
- ? Uses consistent coding style
- ? Includes comprehensive documentation
- ? Provides troubleshooting guidance

## Benefits

1. **Isolates the Issue**: Determines if the limit is in Snap7 or Node-RED
2. **Provides Baseline**: Establishes what's possible with native Snap7
3. **Performance Comparison**: Shows efficiency of multi-variable reads
4. **Reusable**: Can be used for future S7 connectivity testing
5. **Educational**: Demonstrates proper Snap7 client usage

## Troubleshooting Support

Both README files include extensive troubleshooting sections covering:
- Connection issues
- Server configuration problems
- Read failures
- Firewall/network issues
- Permission problems with port 102

## Future Enhancements (Optional)

The client could be extended to:
- Test write operations (Cli_WriteArea, Cli_WriteMultiVars)
- Support other data types (INT, DINT, BOOL, etc.)
- Load test configuration from CSV
- Perform continuous stress testing
- Test with different PDU sizes
- Benchmark performance across different scenarios
