# Ultimate GD Toolkit — Pathfinder Battle Build

Target:
- Geometry Dash Android 2.2081
- Geode SDK 5.10.1
- Android64

## What changed

The existing GD Toolkit settings are represented by one unified SM menu model:

Macro
- Off / Record / Play
- Speed
- Music sync

Frame Tools
- Frame counter
- Click sound
- Sound path

Automation
- Auto-decoration object ID
- AI model
- API configuration

Pathfinder
- Start/stop search
- Retry
- Verify route
- Attempt count
- Search depth
- Save/export verified route

Export
- `.sm`
- `.gdr`
- `.gdr2`
- `.echo`
- Future formats

## Important

The uploaded Claude build contained only a compiled `.so`, `mod.json`, and cache, so its
C++ implementation was not recoverable from that package. This build therefore provides
our own source implementation/architecture rather than modifying Claude's binary.

The format exporters are deliberately isolated. A format is only marked supported after
its real binary/text specification is implemented and validated; changing a filename
extension is never treated as valid conversion.

This package is source code, not a compiled `.geode`.
