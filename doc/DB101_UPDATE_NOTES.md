# Update: Changed Client to Read from DB101

## What Changed

The S7Client test application has been updated to read from **DB101** instead of DB1 to match your actual `address.csv` configuration.

## Modified Files

1. **S7Client/main.cpp**
   - Test 1: Now reads DB101.REAL0, REAL4, REAL8, REAL12, REAL16
   - Test 2: Now reads 20 variables from DB101 (offsets 0-76)
   - Test 3: Now reads 30 variables from DB101 (offsets 0-116)
   - Test 4: Now reads 50 variables from DB101 (offsets 0-196)

2. **Documentation Updated**
   - `S7Client/README.md`
   - `doc/CLIENT_TEST_QUICKSTART.md`
   - `TEST_20_VARIABLE_LIMIT.md`

## Why This Change?

Your `address.csv` file contains data for DB101-DB309, but not DB1. The original test was configured to read from DB1, which wouldn't exist on your server.

## Current Configuration

Based on your `address.csv`, DB101 has the following entries:
- DB101.REAL184 (offset 184)
- DB101.REAL14 (offset 14)

The server will allocate DB101 with a size of at least 188 bytes (184 + 4 for the REAL at offset 184).

The client test will read from DB101 starting at offset 0 through offset 196, which requires **200 bytes** total in DB101.

## Server Allocation

The server's `CreateDataBlocksFromCSV()` function will:
1. Parse all entries from `address.csv`
2. Find the maximum offset needed for each DB
3. Allocate DB101 with size = max(188 bytes from CSV, 200 bytes needed for test)
4. Initialize the values from the CSV at their specified offsets

## Expected Behavior

When you run the test:

1. **Server will allocate DB101** with 188 bytes (based on CSV)
2. **Client will attempt to read** up to offset 196 (200 bytes)
3. **Tests 1-2 should pass** (reading within allocated range)
4. **Tests 3-4 may partially fail** if they try to read beyond offset 187

## Recommendation

To ensure all tests can read successfully, you could add one more entry to `address.csv`:

```csv
"DB101,REAL196",0,100,1,2000
```

This would ensure DB101 is allocated with at least 200 bytes, allowing all 50 REAL values to be read.

## Test Execution

The tests now read:
- **Test 1** (5 vars): DB101.REAL0, 4, 8, 12, 16 ?
- **Test 2** (20 vars): DB101.REAL0 through REAL76 ?
- **Test 3** (30 vars): DB101.REAL0 through REAL116 ?
- **Test 4** (50 vars): DB101.REAL0 through REAL196 ?? (may exceed allocated size)

All tests should work if DB101 is large enough.

## Build Status

? All changes compiled successfully
? No errors or warnings (except existing size_t to int conversion warning)
? Ready to test

## Next Steps

1. **Option A**: Run the test as-is and see how many variables succeed
2. **Option B**: Add `DB101,REAL196` entry to `address.csv` for guaranteed success
3. **Run the test** and review results

The test will clearly show how many of the 50 variables were successfully read from DB101.
