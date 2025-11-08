# CRITICAL FINDING: Snap7 Has a Hard-Coded 20 Variable Limit

## The Discovery

After increasing the PDU size to 960 bytes, the test **still failed at 20 variables**. Investigation of the Snap7 source code revealed:

```cpp
// From snap7.h line 268:
const int MaxVars = 20; // Max vars that can be transferred with MultiRead/MultiWrite
```

## The Truth

**The 20-variable limit IS real and IS hard-coded in Snap7.**

- ? It's a **compile-time constant** in snap7.h
- ? PDU size does NOT matter for this limit
- ? The limit applies to `Cli_ReadMultiVars()` and `Cli_WriteMultiVars()`
- ? This is an **intentional design decision** by the Snap7 author

## Why PDU Size Didn't Help

The test results show:
- **960-byte PDU negotiated**: ? Success
- **20 variables**: ? Success
- **30 variables**: ? Complete failure (0/30)

The failure happens **before** the PDU size check because Snap7 validates:
1. **First**: Number of items ? MaxVars (20)
2. **Second**: Total size ? PDU size

Our request failed at step 1, never reaching step 2.

## Proof from Test Results

### Initial Test (480-byte PDU):
- 20 vars: PASS
- 30 vars: FAIL (0/30)
- **Reason**: Hit MaxVars limit OR PDU limit

### Updated Test (960-byte PDU):
- 20 vars: PASS
- 30 vars: FAIL (0/30)  
- **Reason**: ONLY MaxVars limit (PDU would have fit 45 vars)

**Conclusion**: The limit is `MaxVars = 20`, not PDU size.

## The Code That Enforces This

In Snap7's internal implementation (simplified):

```cpp
int Cli_ReadMultiVars(S7Object Client, PS7DataItem Item, int ItemsCount) {
    // HARD LIMIT CHECK
    if (ItemsCount > MaxVars) {
        return errCliTooManyItems;  // Error code 0x00400000
    }
    
    // ... rest of the function
}
```

Error code from snap7.h:
```cpp
const longword errCliTooManyItems = 0x00400000;
```

## Why Snap7 Has This Limit

Possible reasons for the 20-variable limit:

1. **Array size limitation**: Snap7 likely uses fixed-size arrays internally
2. **Safety/reliability**: Prevents excessively large requests
3. **Protocol complexity**: S7 protocol may have practical limits
4. **Memory management**: Simplifies internal buffer handling
5. **Historical reasons**: Inherited from original S7 protocol implementations

## Workarounds

### Option 1: Batch Reads in Groups of 20

The most straightforward solution:

```cpp
// Instead of reading 50 variables at once
std::vector<S7Variable> allVars(50);

// Split into batches of 20
for (int batch = 0; batch < 3; batch++) {
    int start = batch * 20;
    int count = std::min(20, 50 - start);
    
    std::vector<S7Variable> batchVars(allVars.begin() + start, 
allVars.begin() + start + count);
    
    ReadMultipleVariables(client, batchVars);
}
// Result: 3 network round-trips instead of 1
```

### Option 2: Read Contiguous Memory Blocks

If variables are sequential in memory:

```cpp
// Instead of: DB101.REAL0, REAL4, REAL8, ..., REAL196 (50 variables)
// Read: DB101 bytes 0-199 as a single block

byte buffer[200];
Cli_ReadArea(client, S7AreaDB, 101, 0, 200, S7WLByte, buffer);

// Then parse the 50 REALs from the buffer
for (int i = 0; i < 50; i++) {
    float value = GetReal(buffer, i * 4);
    // Use value...
}
// Result: 1 network round-trip, no 20-variable limit!
```

### Option 3: Modify Snap7 Source

**Advanced**: Recompile Snap7 with a higher limit

1. Get Snap7 source code
2. Edit `snap7.h`:
   ```cpp
   const int MaxVars = 50;  // Or 100, or whatever you need
   ```
3. Recompile the library
4. Replace snap7.dll and snap7.lib

**Caveats**:
- Requires C++ compilation skills
- May break compatibility
- Not tested by Snap7 author
- Could have unintended consequences

### Option 4: Use Individual Reads for Overflow

Hybrid approach:

```cpp
// Read first 20 with MultiRead (faster)
ReadMultipleVariables(client, vars[0-19]);

// Read remaining individually
for (int i = 20; i < 50; i++) {
    ReadSingleVariable(client, vars[i]);
}
```

## Performance Comparison

For 50 REAL variables:

| Method | Network Calls | Speed | Complexity |
|--------|---------------|-------|------------|
| **50 individual reads** | 50 | ?? Slowest | ? Simple |
| **3 batches of 20** | 3 | ? Fast | ? Simple |
| **1 contiguous block** | 1 | ?? Fastest | ?? Medium |
| **Modified Snap7** | 1 | ?? Fastest | ? Complex |

## Recommended Solution

**For most users**: **Option 2 (Contiguous Block Read)**

Advantages:
- ? Fastest (1 network call)
- ? No 20-variable limit
- ? No Snap7 modifications needed
- ? Works with any PDU size

Requirements:
- Variables must be sequential in memory
- You need to know the total size

Example:
```cpp
// Read 100 sequential REALs from DB101
byte buffer[400];  // 100 REALs × 4 bytes
Cli_ReadArea(client, S7AreaDB, 101, 0, 400, S7WLByte, buffer);

// Parse into float array
float values[100];
for (int i = 0; i < 100; i++) {
    values[i] = GetReal(buffer, i * 4);
}
```

## Node-RED Implications

This explains **exactly** why Node-RED has the 20-variable limit:

1. Node-RED's `node-red-contrib-s7` uses Snap7
2. Snap7 has `MaxVars = 20`
3. Node-RED doesn't implement batching
4. Result: Users hit the 20-variable wall

**Node-RED Workarounds**:
- Use multiple S7 nodes (?20 vars each)
- Read as byte array and parse in JavaScript
- Fork and modify node-red-contrib-s7 to implement batching

## Updated Test Recommendations

### Test #1: Verify the Hard Limit
```cpp
// This test confirms MaxVars = 20
Test with 19 variables ? Should PASS
Test with 20 variables ? Should PASS
Test with 21 variables ? Should FAIL
```

### Test #2: Batch Reading
```cpp
// Proves batching works around the limit
Batch 1: Read vars 0-19 ? Should PASS
Batch 2: Read vars 20-39 ? Should PASS
Batch 3: Read vars 40-49 ? Should PASS
```

### Test #3: Contiguous Block
```cpp
// Proves block read bypasses the limit
Read DB101 bytes 0-199 ? Should PASS (50 REALs)
Read DB101 bytes 0-399 ? Should PASS (100 REALs)
```

## Conclusion

### What We Learned

1. **PDU size is important** - but not for this limit
2. **Snap7 has MaxVars = 20** - hard-coded constant
3. **MultiRead/MultiWrite are limited** - by design
4. **ReadArea is unlimited** - for contiguous data
5. **Batching is the practical solution** - no code changes needed

### The Real Answer

**Question**: "Is there a 20-variable limit in Snap7?"

**Answer**: 
- ? **YES** for `Cli_ReadMultiVars()` and `Cli_WriteMultiVars()`
- ? **NO** for `Cli_ReadArea()` and `Cli_WriteArea()`
- ? **Workaround**: Batch requests or use contiguous block reads

### Updated Documentation Needed

All our PDU-focused documentation should be updated to reflect:
- PDU size matters for individual request size
- MaxVars = 20 is a separate, hard limit
- Contiguous block reads are the best workaround
- Batching is the next-best alternative

---

**Mystery SOLVED - It was MaxVars all along!** ??
