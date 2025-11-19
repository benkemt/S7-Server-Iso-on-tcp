# Dynamic Value Update Test Plan

This document describes how to test the dynamic tag value update feature.

## Prerequisites

- S7Server.exe built successfully
- address.csv file in the same directory as the executable
- Node-RED with node-red-contrib-s7 installed (for verification)

## Test Cases

### Test 1: Basic Value Update Verification

**Objective**: Verify that values change over time

**Steps**:
1. Start the S7Server.exe as administrator
2. Verify startup messages include:
   - "Initialized 57 tag states for dynamic updates."
   - "Dynamic tag value updates enabled with 100ms update interval."
3. Connect Node-RED S7 input node to read DB101.REAL184
4. Observe the value over 10-15 seconds
5. Values should start at 0 and increment by 0.5 every 2 seconds

**Expected Result**: Value changes from 0 upward in 0.5 increments

### Test 2: Max Boundary and Direction Reversal

**Objective**: Verify that values reverse direction at max boundary

**Steps**:
1. Use a tag with small range for faster testing (e.g., DB203.REAL184: min=0, max=20, echelon=0.5, cycletime=2000)
2. Monitor the value until it reaches 20
3. Continue monitoring for another 10 seconds

**Expected Result**: 
- Value reaches 20 (max)
- Then starts decreasing by 0.5 every 2 seconds
- Value goes from 20 down to 19.5, 19.0, etc.

### Test 3: Min Boundary and Direction Reversal

**Objective**: Verify that values reverse direction at min boundary

**Steps**:
1. Continue from Test 2 until value reaches 0 (min)
2. Continue monitoring for another 10 seconds

**Expected Result**:
- Value reaches 0 (min)
- Then starts increasing again by 0.5 every 2 seconds

### Test 4: Independent Cycletime

**Objective**: Verify that different tags update at different rates

**Steps**:
1. Monitor two tags simultaneously:
   - DB101.REAL184 (cycletime=2000ms)
   - DB352.REAL14 (cycletime=3000ms)
2. Observe update frequency for each

**Expected Result**:
- DB101.REAL184 updates every 2 seconds
- DB352.REAL14 updates every 3 seconds
- They update independently

### Test 5: Negative Value Range

**Objective**: Verify that negative ranges work correctly

**Steps**:
1. Monitor DB301.REAL112 (range: -50 to 50, echelon=0.1, cycletime=3000)
2. Observe values over several minutes

**Expected Result**:
- Values start at -50
- Increment by 0.1 every 3 seconds
- Reach 50 and reverse to decreasing
- Decrease back to -50 and reverse to increasing

### Test 6: Multiple Simultaneous Tags

**Objective**: Verify system handles all 57 tags updating simultaneously

**Steps**:
1. Use Node-RED MultiRead to read 10-15 different tags at once
2. Monitor for at least 5 minutes
3. Check server performance and responsiveness

**Expected Result**:
- All tags update correctly according to their schedules
- No performance degradation
- No missed updates
- Server remains responsive

### Test 7: Write Override Behavior

**Objective**: Verify behavior when client writes to a tag

**Steps**:
1. Monitor DB101.REAL184
2. Write a custom value (e.g., 999.5) to the tag
3. Continue monitoring

**Expected Result**:
- Written value appears immediately
- Server's automatic update will overwrite this on next cycle
- This is expected behavior - automatic updates continue regardless of writes

## Performance Metrics

- Server should maintain 100ms update cycle
- CPU usage should remain reasonable (<50% on modern systems)
- Memory usage should be stable (no leaks)
- Network responsiveness should not degrade

## Known Limitations

- Manual writes by clients will be overwritten on the next automatic update cycle
- To disable automatic updates for specific tags, remove them from address.csv

## Troubleshooting

### Values Not Changing

1. Check that address.csv is in the executable directory
2. Verify startup messages confirm tag states initialized
3. Check that cycletime values are reasonable (not 0 or negative)

### Server Performance Issues

1. Consider increasing cycletime values to reduce update frequency
2. Reduce number of tags in address.csv
3. Check for other system issues

## Success Criteria

All test cases pass with expected results, demonstrating:
- ✓ Values update according to cycletime
- ✓ Values follow sawtooth pattern (min -> max -> min)
- ✓ Direction reverses correctly at boundaries
- ✓ Multiple tags update independently
- ✓ System performs well under load
