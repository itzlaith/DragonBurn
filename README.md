# DragonBurn Fork

This is a fork of the original DragonBurn CS2 cheat with significant performance improvements and feature enhancements.

## Changes Made

This fork is **not tailored towards kernel-mode anti-cheats** but rather focuses on **VAC bypass only**. The entire codebase has been extensively reworked for better performance and functionality:

### Performance Improvements
- **Rewritten the entire `cheats.cpp` main loop** for better efficiency
- **Massive FPS optimization**: Improved from ~150 FPS to a stable 500+ FPS
- **Ultra-efficient main loop**: Running on only ~1800 CPU cycles
- **Batch memory operations**: Single batch call to kernel mode instead of multiple individual calls
- **Multithreaded main loop** for better performance
- **Fullscreen support** added

### Rendering Optimizations
- **Batch rendering system**: Replaced individual draw calls with collected batch rendering
- **Optimized ESP**: No longer renders every element, player, and text with separate draw calls
- **Efficient data collection**: Collect all data in user mode and sort it locally instead of constant kernel calls

### Feature Enhancements
- **Proper VPK visibility check** implementation
- **Improved triggerbot system**: Proper raycast-based triggerbot (hitbox capsules not yet implemented)
- **Aimbot humanization** features added
- **Enhanced system architecture** with better memory management

## Links

- **Original DragonBurn**: [https://github.com/ByteCorum/DragonBurn](https://github.com/ByteCorum/DragonBurn)
- **Custom Kernel Driver**: [https://github.com/itzlaith/laith-km-driver](https://github.com/itzlaith/laith-km-driver)

## Usage

This fork requires the custom kernel driver linked above. Follow the original DragonBurn installation process but use the custom driver instead of the original one.

---

*Note: This is a performance-focused fork optimized for VAC bypass only. Use at your own risk.*
