# Contributing to S7-Server-Iso-on-tcp

Thank you for your interest in contributing to this project! This document provides guidelines and information for contributors.

## Getting Started

1. Fork the repository
2. Clone your fork
3. Set up the development environment (see README.md)
4. Create a feature branch
5. Make your changes
6. Test your changes
7. Submit a pull request

## Development Environment

### Prerequisites

- Windows 10 or later
- Visual Studio 2022 with C++ development tools
- Snap7 library - Clone from the [official GitHub repository](https://github.com/davenardella/snap7) (recommended) or see README.md for setup instructions

### Building the Project

1. Clone and build the Snap7 library from https://github.com/davenardella/snap7, or run `setup_snap7.bat` to set up the Snap7 library
2. Open `S7Server.sln` in Visual Studio 2022
3. Select Debug|x64 configuration
4. Build the solution (F7)

## Code Style

### C++ Style Guidelines

- Use meaningful variable names
- Add comments for complex logic
- Follow the existing code style
- Keep functions focused and modular
- Use const correctness
- Handle errors appropriately

### Example:

```cpp
// Good: Clear naming and error handling
int RegisterMemoryArea(TS7Server* server, int areaType, int index, byte* data, int size) {
    if (!server || !data || size <= 0) {
        return -1;
    }
    return Srv_RegisterArea(server, areaType, index, data, size);
}

// Bad: Unclear and no error handling
int reg(TS7Server* s, int t, int i, byte* d, int sz) {
    return Srv_RegisterArea(s, t, i, d, sz);
}
```

## Testing

### Manual Testing

1. Build the project
2. Run S7Server.exe as Administrator
3. Verify server starts successfully
4. Test with Node-RED:
   - Configure S7 node to connect to localhost:102
   - Read from DB1, DB2, DB3
   - Write to memory areas
   - Verify read/write operations in server logs

### Test Checklist

- [ ] Server starts without errors
- [ ] Server accepts client connections
- [ ] Read operations work correctly
- [ ] Write operations work correctly
- [ ] Server logs events properly
- [ ] Server shuts down gracefully

## Adding New Features

### Adding Memory Areas

If you want to add new memory areas:

1. Allocate memory in `main.cpp`
2. Initialize the memory
3. Register with `Srv_RegisterArea()`
4. Update README.md with new area information
5. Update CONTRIBUTING.md if needed

Example:
```cpp
// Add DB100 with 2048 bytes
byte* DB100 = new byte[2048];
memset(DB100, 0, 2048);
int result = Srv_RegisterArea(S7Server, srvAreaDB, 100, DB100, 2048);
if (result != 0) {
    std::cerr << "ERROR: Failed to register DB100!" << std::endl;
    return 1;
}
```

### Adding Event Handlers

To add custom event handling:

1. Create a callback function following Snap7 conventions
2. Register the callback with the server
3. Implement your custom logic
4. Document the behavior

Example:
```cpp
void S7API CustomEventCallback(void* usrPtr, PSrvEvent PEvent, int Size) {
    // Your custom logic here
    switch (PEvent->EvtCode) {
        case evcCustomEvent:
            // Handle custom event
            break;
    }
}

// Register the callback
Srv_SetEventsCallback(S7Server, CustomEventCallback, nullptr);
```

## Documentation

### Updating README.md

When making changes that affect usage:

1. Update the relevant sections in README.md
2. Add examples if introducing new features
3. Update troubleshooting if adding known issues
4. Keep the documentation clear and concise

### Code Comments

- Comment complex algorithms
- Document function parameters and return values
- Explain non-obvious behavior
- Keep comments up-to-date with code changes

## Pull Request Process

1. **Create a descriptive title**: Clearly state what the PR does
2. **Provide a detailed description**: Explain what changed and why
3. **Reference issues**: Link to related issues if applicable
4. **Test your changes**: Ensure everything works as expected
5. **Update documentation**: Update README.md and other docs as needed
6. **Keep it focused**: One feature/fix per PR when possible

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Performance improvement

## Testing
Describe how you tested these changes

## Checklist
- [ ] Code builds without errors
- [ ] Code runs without errors
- [ ] Documentation updated
- [ ] Tested with Node-RED (if applicable)
```

## Reporting Issues

### Bug Reports

Include:
- Description of the issue
- Steps to reproduce
- Expected behavior
- Actual behavior
- System information (Windows version, VS version)
- Server logs if applicable

### Feature Requests

Include:
- Description of the feature
- Use case/motivation
- Proposed implementation (if you have ideas)
- Any alternatives considered

## Questions?

If you have questions:
1. Check the README.md
2. Review existing issues
3. Open a new issue with your question

## License

By contributing to this project, you agree that your contributions will be licensed under the MIT License.
