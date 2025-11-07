# CSV Configuration Guide

This guide explains how to use CSV-based configuration to customize the S7 Server's memory layout.

## Overview

The S7 Server now supports dynamic memory initialization through a CSV configuration file (`dresse.csv`). This eliminates the need to modify code or recompile the project when changing memory configurations.

## CSV File Format

The configuration file must be named `dresse.csv` and placed in the same directory as the server executable.

### Structure

```csv
tag,min,max,echelon,cycletime
"DB101,REAL184",0,1800,0.5,2000
"DB101,REAL14",0,1800,0.5,2000
```

### Column Definitions

| Column | Description | Example |
|--------|-------------|---------|
| `tag` | S7 address in format `DB<number>,REAL<offset>` | `"DB101,REAL184"` |
| `min` | Minimum value for initialization | `0` |
| `max` | Maximum value for initialization | `1800` |
| `echelon` | Step/increment value (reserved for future use) | `0.5` |
| `cycletime` | Update interval in ms (reserved for future use) | `2000` |

### Tag Format

The tag field specifies the Data Block number and offset for a REAL (4-byte float) value:

- `DB<number>` - Data Block number (e.g., DB101, DB2, DB303)
- `REAL<offset>` - Byte offset for the REAL value (e.g., REAL184, REAL14)

**Example:** `"DB101,REAL184"` means a REAL value at byte offset 184 in Data Block 101.

## How It Works

1. **On Startup**: The server reads `dresse.csv` from the current directory
2. **Size Calculation**: Automatically calculates the required size for each Data Block based on the highest offset
3. **Memory Allocation**: Allocates Data Blocks dynamically
4. **Initialization**: Initializes each REAL value with a random number between `min` and `max`
5. **Registration**: Registers all Data Blocks with the S7 server

## Example Configuration

The default `dresse.csv` includes 57 entries configuring 36 Data Blocks:

```csv
tag,min,max,echelon,cycletime
"DB101,REAL184",0,1800,0.5,2000
"DB101,REAL14",0,1800,0.5,2000
"DB151,REAL14",0,100,1,2000
"DB201,REAL184",0,20,0.5,2000
"DB201,REAL14",0,20,0.5,2000
"DB251,REAL14",0,100,1,2000
```

This configuration:
- Creates DB101 with at least 188 bytes (offset 184 + 4 bytes for REAL)
- Initializes two REAL values in DB101 at offsets 184 and 14
- Sets values randomly between 0 and 1800
- Creates additional blocks (DB151, DB201, DB251) with their own configurations

## Customizing Your Configuration

### Adding a New Data Block

To add a new Data Block with REAL values:

```csv
"DB999,REAL0",10,100,0.1,1000
"DB999,REAL4",10,100,0.1,1000
"DB999,REAL8",10,100,0.1,1000
```

This creates DB999 with three REAL values at offsets 0, 4, and 8.

### Modifying Value Ranges

To change the range of values, edit the `min` and `max` columns:

```csv
"DB101,REAL184",-50,50,0.5,2000  # Values from -50 to 50
"DB101,REAL14",0,100,1,2000       # Values from 0 to 100
```

### Multiple Values in Same Block

You can define multiple REAL values at different offsets in the same Data Block:

```csv
"DB301,REAL112",-50,50,0.1,3000
"DB301,REAL14",-50,50,0.1,3000
"DB301,REAL116",0,100,1,3000
```

The server will allocate DB301 with at least 120 bytes (offset 116 + 4 bytes).

## Important Notes

1. **File Location**: The CSV file must be in the same directory as `S7Server.exe`
2. **Quoted Tags**: Tags containing commas must be enclosed in quotes
3. **REAL Size**: REAL values are always 4 bytes (IEEE 754 float)
4. **Auto-Sizing**: Data Blocks are sized based on the highest offset in the CSV
5. **Random Initialization**: Values are randomly generated within the specified range on each server start
6. **Persistence**: Written values persist until server restart

## Troubleshooting

### CSV File Not Found

If you see: `WARNING: Could not open CSV file 'dresse.csv'`

**Solution:** Copy `dresse.csv` to the executable directory:
```powershell
Copy-Item "dresse.csv" -Destination "x64\Release\"
```

### Invalid Tag Format

If you see: `WARNING: Failed to parse tag: <tag>`

**Cause:** The tag doesn't match the format `DB<number>,REAL<offset>`

**Solution:** Ensure tags are formatted correctly and enclosed in quotes if they contain commas.

### Memory Allocation Errors

If you see: `ERROR: Failed to register DB<number>!`

**Cause:** The Data Block may be too large or there's insufficient memory

**Solution:** Check the offsets in your CSV and ensure they're reasonable values.

## Migration from Hard-Coded Configuration

The previous version had hard-coded values like:

```cpp
DB1[0] = 42;
SetReal(DB1, 4, 23.5f);
```

These are now replaced by CSV entries:

```csv
"DB1,REAL4",23,24,0.1,2000   # Will initialize to a value near 23.5
```

Note: Values are now randomized within the range. If you need specific values, adjust `min` and `max` to be close together.

## Future Enhancements

The `echelon` and `cycletime` columns are reserved for future features:

- **echelon**: Could be used for value stepping in dynamic updates
- **cycletime**: Could be used to periodically update values to simulate real PLC behavior

## Support

For issues or questions about CSV configuration, please refer to:
- [README.md](README.md) - General server documentation
- [GitHub Issues](https://github.com/benkemt/S7-Server-Iso-on-tcp/issues) - Report problems or request features
