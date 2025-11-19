# Dynamic Value Update - Example Walkthrough

This document provides a practical example of how the dynamic value update feature works.

## Example Tag Configuration

From `address.csv`:
```csv
tag,min,max,echelon,cycletime
"DB203,REAL184",0,20,0.5,2000
```

This configuration means:
- **Tag**: DB203, REAL at byte offset 184
- **Starting value**: 0 (min)
- **Maximum value**: 20 (max)
- **Increment/Decrement**: 0.5 (echelon)
- **Update frequency**: Every 2000ms (2 seconds)

## Value Changes Over Time

### Timeline of Updates

| Time (seconds) | Value | Direction | Explanation |
|---------------|-------|-----------|-------------|
| 0 | 0.0 | Increasing ↑ | Starting value (min) |
| 2 | 0.5 | Increasing ↑ | First update: 0.0 + 0.5 |
| 4 | 1.0 | Increasing ↑ | 0.5 + 0.5 |
| 6 | 1.5 | Increasing ↑ | 1.0 + 0.5 |
| ... | ... | Increasing ↑ | Continues incrementing |
| 78 | 19.5 | Increasing ↑ | Almost at max |
| 80 | 20.0 | **Decreasing ↓** | **Reached max, direction reverses** |
| 82 | 19.5 | Decreasing ↓ | 20.0 - 0.5 |
| 84 | 19.0 | Decreasing ↓ | 19.5 - 0.5 |
| ... | ... | Decreasing ↓ | Continues decrementing |
| 158 | 0.5 | Decreasing ↓ | Almost at min |
| 160 | 0.0 | **Increasing ↑** | **Reached min, direction reverses** |
| 162 | 0.5 | Increasing ↑ | 0.0 + 0.5 |
| ... | ... | Increasing ↑ | Cycle repeats |

### Visual Representation

```
Value
  ↑
20├─────────────────────┐
  │                     │ ╲
  │                    ╱   ╲
  │                   ╱     ╲
15│                  ╱       ╲
  │                 ╱         ╲
  │                ╱           ╲
10│               ╱             ╲
  │              ╱               ╲
  │             ╱                 ╲
 5│            ╱                   ╲
  │           ╱                     ╲
  │          ╱                       ╲
 0└─────────┘                         └─────────→ Time
  0        80s                       160s
```

This creates a **sawtooth wave** pattern that repeats continuously.

## Multiple Tags with Different Cycletimes

Consider two tags being updated simultaneously:

### Tag 1: DB203.REAL184
- cycletime: 2000ms (2 seconds)
- echelon: 0.5

### Tag 2: DB352.REAL14
- cycletime: 3000ms (3 seconds)  
- echelon: 1.0

### Timeline Comparison

| Time | Tag 1 Value | Tag 1 Updated? | Tag 2 Value | Tag 2 Updated? |
|------|-------------|----------------|-------------|----------------|
| 0s   | 0.0         | ✓ (init)       | 0.0         | ✓ (init)       |
| 2s   | 0.5         | ✓              | 0.0         | -              |
| 3s   | 0.5         | -              | 1.0         | ✓              |
| 4s   | 1.0         | ✓              | 1.0         | -              |
| 6s   | 1.5         | ✓              | 2.0         | ✓              |
| 8s   | 2.0         | ✓              | 2.0         | -              |
| 9s   | 2.0         | -              | 3.0         | ✓              |

Each tag maintains its own schedule independently.

## How the 100ms Timer Works

The server's main loop runs every 100ms and checks all tags:

```
Every 100ms:
  For each tag:
    Calculate time since last update
    If (time_elapsed >= tag.cycletime):
      Update tag value
      Reset timer for this tag
```

### Example for DB203.REAL184 (cycletime=2000ms)

```
Time    | Elapsed | Action
--------|---------|----------------------------------
0ms     | 0ms     | Initialize, set lastUpdate=0ms
100ms   | 100ms   | Check: 100 < 2000? No update
200ms   | 200ms   | Check: 200 < 2000? No update
...     | ...     | Continue checking
1900ms  | 1900ms  | Check: 1900 < 2000? No update
2000ms  | 2000ms  | Check: 2000 >= 2000? UPDATE!
                  | Increment value: 0.0 → 0.5
                  | Reset: lastUpdate=2000ms
2100ms  | 100ms   | Check: 100 < 2000? No update
...     | ...     | Continue...
```

## Advantages of This Design

1. **Precision**: 100ms checking provides accurate timing
2. **Independence**: Each tag updates on its own schedule
3. **Efficiency**: Only updates tags when needed
4. **Scalability**: Handles 50+ tags without performance issues
5. **Simplicity**: Easy to understand and maintain

## Customization Examples

### Fast Changing Temperature Sensor
```csv
"DB301,REAL14",-50,50,0.1,500
```
- Updates every 0.5 seconds
- Small steps (0.1) for smooth changes
- Range: -50 to 50

### Slow Changing Tank Level
```csv
"DB305,REAL184",0,1400,1.0,5000
```
- Updates every 5 seconds
- Larger steps (1.0) for noticeable changes
- Large range: 0 to 1400

### Percentage Meter
```csv
"DB352,REAL14",0,100,2.5,1000
```
- Updates every second
- Moderate steps (2.5%)
- Standard percentage range: 0 to 100

## Testing Tips

1. **Start with fast updates** (cycletime=1000ms) to see changes quickly
2. **Use small ranges** (e.g., 0-20) to reach boundaries faster
3. **Monitor multiple tags** to verify independence
4. **Check boundary behavior** at min and max values

## Common Use Cases

- **Process simulation**: Simulating temperature, pressure, flow sensors
- **Load testing**: Generating continuous data for testing applications
- **Development**: Testing UI updates and trend displays
- **Demos**: Showing live data without actual hardware
- **Training**: Educational purposes for PLC/SCADA systems
