# POW|FORMER Developer Guide

## Keeping Development Separate from Releases

The key to separating development from releases is using **git branches**. Here's the recommended workflow:

### Branch Strategy

```
master          # Stable releases only (users download from here)
  └── develop   # Development branch (ongoing work, may be unstable)
      ├── feature/new-feature
      └── bugfix/fix-issue
```

### Initial Setup

```bash
# Create develop branch (if it doesn't exist)
git checkout -b develop

# Push develop branch to GitHub
git push -u origin develop
```

### Daily Development Workflow

```bash
# Work on develop branch
git checkout develop

# Make changes, commit regularly
git add src/apps/sequencer/model/NoteTrack.h
git commit -m "feat: Add new parameter"

# Push to develop (not master!)
git push origin develop
```

### Creating a Release

```bash
# 1. Create release branch from develop
git checkout develop
git checkout -b release/v0.0.64

# 2. Update version in Config.h
# 3. Update CHANGELOG_POWFORMER.md
# 4. Build and test firmware
# 5. Test on hardware

# 6. Merge to master
git checkout master
git merge --no-ff release/v0.0.64
git tag -a v0.0.64 -m "POW|FORMER v0.0.64: Description"

# 7. Merge back to develop (to get version updates)
git checkout develop
git merge --no-ff release/v0.0.64

# 8. Clean up
git branch -d release/v0.0.64

# 9. Push everything
git push origin master develop --tags
```

### Branch Usage Summary

| Branch | Purpose | Who Uses It | Stability |
|--------|---------|-------------|-----------|
| **master** | Releases only | End users | Stable |
| **develop** | Active development | Developers | May be unstable |
| **feature/*** | New features | Developers | Experimental |
| **release/*** | Release prep | Release manager | Testing |
| **hotfix/*** | Emergency fixes | Developers | Critical fixes |

### Why This Works

- **master** stays clean - only contains tested, stable releases
- **develop** can have experimental code without affecting users
- **Users** clone/download from master and always get stable code
- **Developers** work on develop without breaking master
- **Releases** are tagged on master for easy identification

## Project Structure

### Fork Hierarchy

```
westlicht/performer (Original PER|FORMER)
  └── djphazer/performer (PEW|FORMER fork)
      └── modulove/performer (POW|FORMER fork - this repo)
```

## Development Documentation

### Where to Keep Development Files

#### Temporary Development Files (NOT in Git)

Keep in `WestlichtDev/` folder (add to `.gitignore`):
- `COMMIT_MESSAGE.txt` - Draft commit messages
- `COMMIT_SUMMARY.md` - Session summaries
- `FILES_MODIFIED.md` - File tracking
- `SESSION_CODE_CHANGES.md` - Detailed changes
- `dev_notes.md` - Scratch notes
- `New Folder With Items/` - Archived notes

Add to `.gitignore`:
```
WestlichtDev/
```

#### Published Documentation (Committed to Git)

At root level:
- `CHANGELOG_POWFORMER.md` - Feature changelog
- `CHANGELOG_v0.0.*.md` - Version changelogs
- `README.md` - User guide
- `DEVELOPER.md` - This file

## Build System

### Build Environment

- **Build directory**: `~/performer-build/`
- **Source directory**: Current repo location
- **Toolchain**: `~/performer-tools/arm-gnu-toolchain-14.2.rel1-darwin-arm64-arm-none-eabi/`

### Build Commands

```bash
# Full build
~/build-performer.sh

# Check binary size
arm-none-eabi-size ~/performer-build/build/stm32/release/src/apps/sequencer/sequencer.elf

# Create UPDATE.DAT file
scripts/makeupdate \
  ~/performer-build/build/stm32/release/src/apps/sequencer/sequencer.bin \
  ~/Desktop/UPDATE_POWFORMER.DAT
```

### Binary Locations

- **ELF**: `~/performer-build/build/stm32/release/src/apps/sequencer/sequencer.elf`
- **Binary**: `~/performer-build/build/stm32/release/src/apps/sequencer/sequencer.bin`
- **UPDATE.DAT**: Created by `makeupdate` script

## Memory Constraints

### STM32F405RGT6 Hardware Limits
- **Flash**: 1 MB (1,048,576 bytes)
- **RAM**: 192 KB total
  - 128 KB main RAM
  - 64 KB CCM (Core Coupled Memory)

### Current Usage (v0.0.63)
```
   text    data     bss      total
 361028    6712  153404     521144
```

- **text**: 361 KB / 1024 KB (35% Flash)
- **bss**: 150 KB / 192 KB (78% RAM)

### Budget Guidelines
- Keep Flash < 512 KB (50%) for safety
- Keep RAM < 160 KB (83%) to avoid stack issues
- Check after every build with `arm-none-eabi-size`

## Code Structure

```
src/apps/sequencer/
├── model/              # Data structures and logic
│   ├── NoteSequence.h/cpp
│   ├── NoteTrack.h/cpp
│   ├── Modulator.h     (POW|FORMER)
│   └── Project.h/cpp
├── engine/             # Real-time playback engines
│   ├── NoteTrackEngine.h/cpp
│   ├── ModulatorEngine.h     (POW|FORMER)
│   └── Engine.h/cpp
├── ui/                 # User interface
│   ├── pages/         # UI screens
│   ├── painters/      # Drawing utilities
│   └── model/         # UI data models
├── Config.h           # Compile-time configuration
└── CMakeLists.txt     # Build configuration
```

## Configuration

### Key Config.h Definitions

```cpp
// Version
#define CONFIG_VERSION_NAME             "POW|FORMER SEQUENCER"
#define CONFIG_VERSION_REVISION         63

// Counts
#define CONFIG_TRACK_COUNT              16  // Was 8
#define CONFIG_PATTERN_COUNT            8   // Per track
#define CONFIG_SONG_SLOT_COUNT          4   // Was 16

// Feature Flags
#define CONFIG_ENABLE_CURVE_TRACKS      0   // Disabled to save RAM
```

## Version Management

### Firmware Version

In `Config.h`:
```cpp
#define CONFIG_VERSION_REVISION         64  // Increment for new release
```

### Project Version (Serialization)

In `ProjectVersion.h`:
```cpp
enum Version : uint16_t {
    Version27 = 27,  // POW|FORMER additions
};
```

When adding new serialized fields:
```cpp
// In write() method
writer.write(_newField);

// In read() method with version check
reader.read(_newField, ProjectVersion::Version27);  // Defaults to 0 for older projects
```

## Testing Checklist

### Pre-Release Testing

- [ ] Compiles without warnings
- [ ] Binary size within limits
- [ ] Boots on hardware
- [ ] Project load/save compatibility
- [ ] All features functional
- [ ] MIDI input/output working
- [ ] CV output working
- [ ] Modulators working
- [ ] UI navigation working
- [ ] Pattern switching reliable
- [ ] Stability test (1+ hour running)

## Keeping Fork Updated

### Add Upstream Remotes (One-Time Setup)

```bash
git remote add westlicht https://github.com/westlicht/performer.git
git remote add djphazer https://github.com/djphazer/performer.git
```

### Merge Upstream Changes

```bash
# Fetch latest from djphazer
git fetch djphazer

# View what changed
git log HEAD..djphazer/main --oneline

# Merge into develop (NOT master!)
git checkout develop
git merge djphazer/main
# Resolve conflicts if any
```

### Cherry-Pick Specific Commits

```bash
# Pick specific commit from upstream
git cherry-pick <commit-hash>
```

## Release Process Summary

1. **Develop** on `develop` branch
2. **Create** release branch: `release/v0.0.64`
3. **Update** version in Config.h and CHANGELOG
4. **Build** and test thoroughly
5. **Merge** to `master` and tag
6. **Merge back** to `develop`
7. **Push** all branches and tags
8. **Create** GitHub Release with binary

## Common Tasks

### Adding a New Parameter to Track

```cpp
// 1. Add to NoteTrack.h
uint8_t _myParameter = 0;

// 2. Add accessors
int myParameter() const { return _myParameter; }
void setMyParameter(int value) { _myParameter = clamp(value, 0, 127); }

// 3. Add serialization
writer.write(_myParameter);
reader.read(_myParameter, ProjectVersion::Version28);

// 4. Add to UI (NoteTrackListModel.h)
case MyParameter: return "My Param";
```

### Memory Analysis

```bash
# Show largest symbols
arm-none-eabi-nm --size-sort --print-size sequencer.elf | tail -n 50

# Show section sizes
arm-none-eabi-size -A sequencer.elf
```

## Credits

- **westlicht** (Simon Kallweit) - Original PER|FORMER
- **djphazer** (Phazerville) - PEW|FORMER fork, toolchain updates
- **jackpf** - Improvements (via PEW|FORMER)
- **modulove** - POW|FORMER fork (16 tracks, modulators, microtiming)
- **Claude** (Anthropic) - AI-assisted development

## License

GPL-3.0 (inherited from westlicht/performer)

---

**Last Updated**: October 27, 2025
**POW|FORMER Version**: 0.0.63
