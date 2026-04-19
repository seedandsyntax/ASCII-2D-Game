## Introduction
The game's name nor story is solidified, however, it will be a:
- TUI (Terminal User Interface) game.
- Uses ASCII (ANSI 24-bit) for rendering custom ASCII characters we will make in the Inkscape software.
- The game's entire foundation will be Imperative Programming (Specifically, OOPs)
- The project's main goal is to make a simple game whose storyline can be expanded upon in later versions, meaning we will be making a simple engine that could
	- Render ASCII characters as per our requirements of the game.
	- Generate scenes / Levels with the Characters as per our need. 
	- Dialogues and Menu section that are to be interactive.
- The game is meant to be 2D presentation and therefore the game-engine will be significantly simple.
- For now, we will use CMake only, later on we can expand to use more than CMake to make it easier for ourselves to compile with many compiler versions.
- The project strictly uses C++23 with GCC compiler.
---
## Table of Content
1. [[#Codebase Structure]]
2. [[#GitHub Branches]]
3. [[#Contributing to ASCII RPG]]
4. [[#Game Engine Development Pipeline]]
5. [[#Assets Generation Pipeline]]
6. Story & World Building Pipeline
7. Utilization of Game Engine
8. Supported Terminals (And till where we will support for now)
---
## Codebase Structure
```tree
ascii-rpg/
├── assets/
│   ├── source-art/             # <-- INKSCAPE .SVG FILES LIVE HERE
│   └── compiled-font/          # <-- Generated .hpp header maps
├── docs/
│   ├── PDD.md                  # The document we just outlined
│   └── CONTRIBUTING.md         # See Section 3 below
├── src/
│   ├── api/
│   ├── engine/                 # LIBRARY (Static Lib target in CMake)
│   │   ├── terminal-manager.cpp   # Raw input, Alt Buffer, ANSI codes
│   │   ├── renderer.cpp          # Double buffering, Viewport clipping
│   │   ├── color.hpp             # RGB lerping, ANSI string generation
│   │   └── CMakeLists.txt
│   ├── game/                   # EXECUTABLE (Depends on Engine)
│   │   ├── entities/
│   │   │   ├── entity.cpp
│   │   │   └── player.cpp
│   │   ├── dialog-manager/
│   │   │   └── dialogue-manager.cpp
│   │   ├── main.cpp
│   │   └── CMakeLists.txt
│   └── utils/                  # Shared headers (String utils, File IO)
├── CMakeLists.txt              # Root: defines CMAKE_CXX_STANDARD 17
└── README.md
```
---
## GitHub Branches
- There will be three types of branches:
	- `main`: It is protected, meaning only PRs that have demonstrated to be stable will be allowed to be integrated.
	- `dev`: It is unprotected, the PRs to integrate a new feature will be made here, but no direct work will be pushed to it.
	- `feature/xyz`: the `xyz` can be any feature, these branches can be pushed to at any time and are the ones that you request to PR to the `dev` branch.
- A few rules to follow for a more stable and predictable building process:
	- _Rule 1:_ To merge `dev` -> `main`, you need **1 approval** from another human. The approver **must** run `./build.sh` and confirm the `@` moves on screen.
	- _Rule 2:_ The `main` branch rejects any commit that doesn't compile with CMake (optional GitHub Action recommended).
	- _Rule 3:_ Anyone can create a `feature/` branch off `dev` and push freely.
---
## Contributing to ASCII RPG

Welcome! This document defines **exactly** how we write code, name things, test changes, and collaborate. Following these rules keeps the project maintainable and prevents merge conflicts and architectural drift.

If something isn't covered here, refer to `docs/PDD.md`. If still unsure, ask in `#dev-general` on Discord.

---

### 1. Naming Conventions

Consistency is non‑negotiable. It reduces cognitive load when reading unfamiliar code.

#### Files and Folders

| Type | Convention | Example |
| :--- | :--- | :--- |
| C++ Source Files | `PascalCase.cpp` | `TerminalManager.cpp` |
| C++ Header Files | `PascalCase.hpp` | `Color.hpp` |
| CMake Files | Exactly `CMakeLists.txt` | `CMakeLists.txt` |
| Asset Files | `snake_case.ext` | `player.png`, `dragon.sprite` |
| Documentation | `UPPER_SNAKE_CASE.md` | `CONTRIBUTING.md`, `PDD.md` |
| Folders | `PascalCase/` or `snake_case/` | `src/Engine/`, `assets/source_art/` |

**Rationale:** `PascalCase` for C++ classes matches the class name inside. `snake_case` for assets avoids case‑sensitivity issues across platforms.

#### Code Identifiers

| Element | Convention | Example |
| :--- | :--- | :--- |
| Classes / Structs | `PascalCase` | `class TerminalManager`, `struct Glyph` |
| Functions / Methods | `camelCase` | `void drawChar()`, `bool isSolid()` |
| Variables (local & member) | `camelCase` | `int playerX`, `bool m_isInitialized` |
| Member Variables (private) | `m_camelCase` | `m_backBuffer`, `m_originalTermios` |
| Constants / Enums | `UPPER_SNAKE_CASE` | `const int MAX_HEALTH = 100;` |
| Namespaces | `PascalCase` | `namespace Engine`, `namespace GameAssets` |
| Macros (avoid if possible) | `UPPER_SNAKE_CASE` | `#define UNUSED(x) (void)(x)` |

**Example:**
```cpp
class TerminalManager {
public:
    void drawString(int x, int y, const std::string& text);
private:
    std::vector<Cell> m_backBuffer;
    bool m_initialized;
};
```

#### Git Branches

| Type | Convention | Example |
| :--- | :--- | :--- |
| Feature branches | `feature/short-description` | `feature/inventory-system` |
| Bugfix branches | `fix/short-description` | `fix/terminal-resize-crash` |
| Experimental | `experiment/short-description` | `experiment/sixel-support` |

Use **lowercase** and **hyphens** for separators. Keep descriptions under 4 words.

---

### 2. Imperative Programming with OOP Emphasis

Our architecture is **imperative at the top level** (the main game loop) but uses **object‑oriented design** for entities and systems.

#### Core Principle: Data + Behavior Together

Every game object is an instance of a class that inherits from a common base.

```cpp
// Base class (abstract)
class IEntity {
public:
    virtual ~IEntity() = default;
    virtual void update(float deltaTime) = 0;
    virtual void draw(Renderer& renderer) const = 0;
    virtual bool isSolid() const { return false; }
    virtual int getX() const = 0;
    virtual int getY() const = 0;
};

// Concrete class
class Player : public IEntity {
public:
    Player(int startX, int startY);
    void update(float deltaTime) override;
    void draw(Renderer& renderer) const override;
    int getX() const override { return m_x; }
    int getY() const override { return m_y; }
private:
    int m_x, m_y;
    int m_health;
};
```

#### Imperative Game Loop

The `main()` function drives the game imperatively:

```cpp
while (running) {
    float dt = measureDeltaTime();
    processInput();
    for (auto& entity : world.entities) {
        entity->update(dt);
    }
    renderer.draw(world);
}
```

#### What NOT to Do

- ❌ Giant `Game` class that does everything (God Object anti‑pattern).
- ❌ Free functions that modify global state without clear ownership.
- ❌ Mixing rendering logic inside entity update methods.

---

### 3. Test File Requirements (Pre‑Merge)

**Every new `.cpp` file added to `src/` must be accompanied by a corresponding test file before its feature branch can be merged to `dev`.**

#### Test File Naming and Location

```
src/Engine/TerminalManager.cpp
src/Engine/TerminalManager.test.cpp   ← Test file lives alongside source
```

#### What a Test File Must Contain

A minimal test file includes:

1. A `main()` function that exercises the public interface of the class.
2. At least one **assertion** that verifies expected behavior.
3. Cleanup that restores terminal state (critical for TUI code).

#### Example Test File

```cpp
// src/Engine/TerminalManager.test.cpp
#include "TerminalManager.hpp"
#include <cassert>
#include <iostream>

int main() {
    Engine::TerminalManager tm;
    
    // Test 1: Initialization
    assert(tm.initialize() == true);
    
    // Test 2: Cursor movement doesn't crash
    tm.moveCursor(10, 5);
    tm.draw('@');
    tm.flush();
    
    // Test 3: Color set doesn't crash
    tm.setColor(255, 0, 0);
    tm.draw("Red text");
    tm.resetColor();
    
    tm.shutdown();
    std::cout << "[PASS] TerminalManager tests passed.\n";
    return 0;
}
```

#### Running Tests

Tests are **not** part of the main game executable. They are compiled as separate targets:

```cmake
# In src/Engine/CMakeLists.txt
add_executable(test_TerminalManager TerminalManager.test.cpp)
target_link_libraries(test_TerminalManager Engine)
```

Before merging a PR, the reviewer will run:

```bash
cd build
cmake .. && make
./test_TerminalManager
```

If the test passes and the terminal is left in a usable state, the PR is eligible for merge.

---

### 4. Git Commit Message Format

We use the **Conventional Commits** standard. This makes changelog generation and `git blame` more useful.

#### Format

```
<type>(<scope>): <short description>

[optional body]

[optional footer]
```

#### Types

| Type | Purpose |
| :--- | :--- |
| `feat` | A new feature (e.g., `feat(engine): add double buffering`) |
| `fix` | A bug fix |
| `docs` | Documentation changes only |
| `style` | Code formatting, missing semicolons, etc. (no logic change) |
| `refactor` | Code change that neither fixes a bug nor adds a feature |
| `test` | Adding or updating tests |
| `chore` | Build process, tooling, or auxiliary changes |

#### Scope

The name of the module or component affected (e.g., `engine`, `game`, `renderer`, `assets`, `cmake`).

#### Examples

```
feat(renderer): implement dirty rect tracking for performance

Added a `std::vector<Rect> m_dirtyRects` to Renderer. Only cells
within dirty rects are redrawn. Reduces STDOUT writes by ~60%.

Closes #42
```

```
fix(terminal): restore cursor visibility on SIGINT

Previously, Ctrl+C would leave the terminal cursor hidden. Now the
signal handler calls `shutdown()` before exiting.
```

```
test(engine): add unit test for Color::lerp

Verifies that lerping between red and green at t=0.5 yields yellow.
```

#### Commit Frequency

- Commit **early and often** on your feature branch.
- Squash commits before merging to `dev` so the history remains clean. Use `git rebase -i` to combine small "wip" commits into logical units.

---

### 5. Top‑Down Architecture of OOP Code

We design from the **top down**: first define the public interface, then implement the private details.

#### Step‑by‑Step for Adding a New Class

1. **Create the Header (`.hpp`) with Only Public Interface**

```cpp
// src/Game/Systems/DialogueManager.hpp
#pragma once
#include <string>
#include <vector>

namespace Game {

class DialogueManager {
public:
    // Public API – this is what other code will call
    void startDialogue(const std::string& npcId);
    void nextLine();
    bool isActive() const;
    std::string getCurrentLine() const;
    
private:
    // Leave private section empty initially
};

} // namespace Game
```

2. **Write the Test File (`.test.cpp`)**

Write code that uses the public API exactly as it will be used in the game. This validates your design before you write implementation.

3. **Implement the `.cpp` File**

Only after the interface and test are reviewed do you fill in the private members and method bodies.

#### Dependency Direction

Always follow **top‑down dependency flow**:

```
Game (depends on) → Engine (depends on) → Utils
```

- `Game` may call `Engine::Renderer`.
- `Engine` may call `Utils::StringHelper`.
- `Engine` **must never** call `Game::Player`.

This ensures the engine remains reusable and the game logic stays separate.

---

### 6. Test File Format

All test files follow a consistent structure for easy parsing and execution.

#### Required Sections (in order)

1. **Header Comment** – What is being tested.
2. **Includes** – The header under test + `<cassert>`.
3. **Test Functions** – Named `test_<feature>()` returning `bool`.
4. **`main()`** – Runs all tests and reports results.

#### Full Template

```cpp
/**
 * Test file for: TerminalManager
 * Purpose: Verify raw mode, alternate screen, and color escape sequences.
 */

#include "TerminalManager.hpp"
#include <cassert>
#include <iostream>

// ------------------------------------------------------------------
// Individual Test Functions
// ------------------------------------------------------------------

bool test_initialization() {
    Engine::TerminalManager tm;
    bool result = tm.initialize();
    tm.shutdown();
    return result;
}

bool test_cursor_movement() {
    Engine::TerminalManager tm;
    tm.initialize();
    // Moving to (0,0) should not throw or crash
    tm.moveCursor(0, 0);
    tm.draw('X');
    tm.flush();
    tm.shutdown();
    return true;
}

bool test_color_output() {
    Engine::TerminalManager tm;
    tm.initialize();
    tm.setColor(255, 0, 0);   // Red
    tm.draw("RED");
    tm.resetColor();
    tm.draw(" NORMAL");
    tm.flush();
    tm.shutdown();
    return true;
}

// ------------------------------------------------------------------
// Test Runner
// ------------------------------------------------------------------

int main() {
    int passed = 0;
    int total = 0;

    auto runTest = [&](bool (*testFunc)(), const char* name) {
        total++;
        std::cout << "Running " << name << "... ";
        try {
            if (testFunc()) {
                std::cout << "PASS\n";
                passed++;
            } else {
                std::cout << "FAIL\n";
            }
        } catch (const std::exception& e) {
            std::cout << "FAIL (exception: " << e.what() << ")\n";
        }
    };

    runTest(test_initialization, "test_initialization");
    runTest(test_cursor_movement, "test_cursor_movement");
    runTest(test_color_output, "test_color_output");

    std::cout << "\n" << passed << "/" << total << " tests passed.\n";
    return (passed == total) ? 0 : 1;
}
```

#### Acceptance Criteria for Test Files

- [ ] Each public method has at least one corresponding test function.
- [ ] Tests clean up after themselves (terminal state restored).
- [ ] No manual intervention required (tests run automatically).
- [ ] Return code `0` only if all tests pass.

---

### Quick Reference Checklist for PRs

Before opening a pull request to `dev`, confirm:

- [ ] File and folder names follow conventions.
- [ ] Class/function/variable names follow conventions.
- [ ] Code is OOP: entities inherit from `IEntity`, systems are classes.
- [ ] A `.test.cpp` file exists and passes.
- [ ] Commit messages use Conventional Commits format.
- [ ] Dependencies flow downward (Game → Engine → Utils).
- [ ] No `std::println` outside of `TerminalManager` or test files.
- [ ] Terminal cursor is restored even if the test fails.

---
## Game Engine Development Pipeline
- The pipeline so far is AI generated Suggestion so far, we will have to figure out how things go from here on out. Read through the section and discuss what to keep and what to remove.
- Once the pipeline is established, we can begin to work on the first task, the game engine itself.
### What We're Building

A game engine split into two parts:

- The Engine is a reusable toolbox (`libascii_engine.a`) that handles the terminal, drawing, and colors
- The Game is the actual game that _uses_ that toolbox

The engine gets built first, once. The game just borrows from it.

---

#### Phase 1 -> Talking to the Terminal

**File:** `src/Engine/TerminalManager.cpp`

The terminal wasn't designed for games. This phase fixes that by doing three things:

**1. Alternate Screen** - When the game launches, it switches to a blank screen (like `vim` does). When it exits, your previous terminal is restored clean. Without this, game output would just scroll down your history forever.

**2. Raw Mode** - Normally the terminal waits for you to press Enter before sending input to a program. Raw mode removes that delay so every keypress is delivered instantly. Without this, a real-time game is impossible.

**3. Resize Handling** - When you resize the terminal window, the OS sends a signal called `SIGWINCH`. The rule here is simple: the signal handler does nothing except flip a flag:

```cpp
volatile sig_atomic_t g_terminalResized = false;
// signal fires -> sets flag to true
// main loop sees flag -> recalculates layout -> resets flag
```

This matters because signal handlers are dangerous places to run real logic. Keep them as dumb as possible.

---

#### Phase 2 -> Drawing Without Flicker

**File:** `src/Engine/Renderer.cpp`

Even with Phase 1 working, drawing directly to the screen causes flicker -- the user sees half-drawn frames. This phase fixes that.

**Double Buffering** - Instead of drawing straight to the screen, you draw into a hidden array in memory (the back buffer). Once the whole frame is ready, you push it all at once. The user only ever sees complete frames.

**Screen Layout** - The screen is divided into fixed regions based on terminal height (`H`):

```
Row 0         +---------------------------+
...           |  Game world draws here    |
Row H-6       |  (VIEWPORT_ROWS = H-5)   |
Row H-5       +---------------------------+
Row H-4       |  Dialogue box (4 rows)    |
Row H-3       |                           |
Row H-2       +---------------------------+
Row H-1       |  Menu bar (1 row)         |
              +---------------------------+
```

Everything is relative to terminal height so it adapts when the window is resized.

---

#### Phase 3 -> Colors

**File:** `src/Engine/Color.hpp`

Terminals support full 24-bit color, but you have to send a specific escape code to use it -- something like `\x1b[38;2;255;0;0m` for red. Writing that everywhere by hand is a nightmare.

This phase wraps all of that into a simple struct:

```cpp
Color c(255, 0, 0);     // make a red color
c.ToForegroundANSI();   // -> "\x1b[38;2;255;0;0m"  (handled for you)
```

It also provides **Lerp** (linear interpolation) -- a way to blend smoothly between two colors:

```cpp
Color::Lerp(green, red, 0.0f)  // -> full green  (full health)
Color::Lerp(green, red, 0.5f)  // -> yellow      (half health)
Color::Lerp(green, red, 1.0f)  // -> full red    (critical)
```

Being header-only (`.hpp`) means there's no separate `.cpp` to compile. Anyone who includes the file gets the code directly. Fine for small math utilities like this.

---

#### Phase 4 -> The Main Loop

**File:** `src/Game/main.cpp`

This is the heartbeat -- a loop that runs ~60 times per second until the player quits. Every single iteration does these steps in order:

```
1. Measure how long the last frame took       (delta time)
2. Read any keys pressed since last frame     (input)
3. Update the game world                      (logic)
4. Draw the new state into the back buffer    (render)
5. Sleep for whatever time is left in 16.67ms (frame cap)
```

Two things beginners often get wrong here:

**Delta time must be measured, not guessed.** You might think "60 FPS means 16ms per frame, so just hardcode `0.016f`." But if one frame takes 30ms due to lag, the game world still advances by only 16ms -- causing drift and desync. Always measure:

```cpp
float deltaSeconds = duration<float>(frameStart - previousFrameStart).count();
```

**16ms is not 60 FPS.** True 60 FPS is 16.666...ms. The code uses `std::ratio<1,60>` which represents exactly 1/60th of a second, avoiding that rounding error entirely.

---

#### Phase 5 -> Building It All

This phase is just the build configuration -- how CMake compiles everything into the actual library.

The notable flags are `-Wall -Wextra -Wpedantic` -- these tell the compiler to warn you about any suspicious code, not just outright errors. Good habit. `-Werror` (treats warnings as errors) is saved for the `main` branch only, so developers aren't blocked by it during normal work.

There's also a minimal **smoke test** -- a quick sanity check that verifies raw mode works, the buffer doesn't crash, and the color codes are formatted correctly. Not a full test suite, just "does the engine boot without exploding?"

---

### The Full Picture

```
+---------------------------------------------------------------+
|                        GAME EXECUTABLE                        |
|                       src/Game/main.cpp                       |
|                                                               |
|   Phase 4: Main Loop                                          |
|   - Runs 60x per second                                       |
|   - Orchestrates all phases in order:                         |
|     [Input] -> [Update] -> [Render] -> [Sleep]                |
+----+---------------------------+------------------------------++
     |                           |
     | calls                     | calls
     v                           v
+----+-------------------+  +----+-------------------+
|  Phase 1: TAL          |  |  Phase 2: Renderer     |
|  TerminalManager.cpp   |  |  Renderer.cpp          |
|                        |  |                        |
|  - Alternate screen    |  |  - Back buffer         |
|  - Raw mode input      |  |  - Screen layout       |
|  - Resize handling     |  |  - Dirty rect (todo)   |
+----+-------------------+  +----+-------------------+
     |                           |
     | talks to                  | uses
     v                           v
+----+-------------------+  +----+-------------------+
|  OS / Terminal         |  |  Phase 3: Color        |
|                        |  |  Color.hpp             |
|  - Linux (termios)     |  |                        |
|  - Windows (Console)   |  |  - RGB constructor     |
|                        |  |  - ToForegroundANSI()  |
+------------------------+  |  - Lerp()              |
                            +------------------------+

+---------------------------------------------------------------+
|                       Phase 5: Build                          |
|                                                               |
|   CMake packages Phases 1-3 into libascii_engine.a            |
|   The game links against it at compile time.                  |
+---------------------------------------------------------------+

Rule: every phase only ever calls DOWNWARD.
      Nothing in Color knows about Renderer.
      Nothing in Renderer knows about the game world.
      That is what keeps the whole thing from tangling into a mess.
```
---
## Assets Generation Pipeline
- These are the basic constraints for the assets we will make in this project.
	- Document Setup
		- Canvas size: 16x16 pixels for single glyphs. For multi-cell sprites, use a multiple of 16 (e.g. 48x48 for a 3x3 sprite.)
	- Drawing Rules
		- Units: `px` (pixels)
	- Export Process
		- Grid: Rectangular grid, spacing `1.0 px`, visible, and snap to grid enabled.
	- Tool recommended to use: InkScape
- Drawing Rules
	- Use fills only; no strokes. Strokes cause anti-aliasing artifacts at 1-pixel scale.
	- Use a limited palette of solid colors. The pipeline will map each unique RGB value to an ANSI escape code.
	- For multi-cell sprites, draw the entire composite image at the larger canvas size. A separate `.sprite` file will define how to slice it.
- Export Process
	- Select the object(s) to export
	- `File` -> `Export PNG Image`
	- Choose Selection (not page)
	- Set Width and Height to the exact pixel dimensions (e.g., 16x16 or 48x48).
	- Set DPI to `96`.
	- Save to `assets/source_art/png/[name].png`
### Directory Structure
```tree
ascii-rpg/
├── assets/
│   ├── source_art/                  # Human‑editable files
│   │   ├── master_glyphs.svg        # Inkscape master document
│   │   ├── png/                     # Exported 16×16 PNGs
│   │   │   ├── player.png
│   │   │   ├── wall.png
│   │   │   ├── goblin.png
│   │   │   └── dragon.png           # Larger sprite sheet (e.g., 48×48)
│   │   └── sprites        # Optional: multi‑cell sprite definitions
│   │       └── dragon.sprite        # Text file defining cell grid
│   └── compiled_font/               # Generated C++ headers (never edit manually)
│       ├── glyph_data.hpp
│       └── sprite_data.hpp
├── tools/
│   ├── glyph_compiler.py            # Main conversion script
│   ├── requirements.txt             # Python dependencies
|   └── test_glyph.cpp               # For testing glyph setup
└── src/
    └── engine/
        ├── glyph.hpp                # Data structure for glyphs/sprites
        ├── renderer.cpp             # Uses generated headers
        └── ...
```
### Conversion Logic (Python Script)
- The script `tools/glyph_compiler.py` performs the following steps for each PNG:
	- Image Loading and validation via `PIL`.
	- Color Quantization and Mapping.
	- Character Approximation.
- The steps of these will be more elaborate after we make and test our first few assets conversion that will be compatible with the engine.
### Sprite Definition for Multi-Cell Entities
- For images larger than 16x16, the script slices them into cells of 16x16 pixels.
- A `.sprite` file defines how the cells are arranged, example:
```pseudocode
width 3
height 3
cells:
  cell_0_0 cell_1_0 cell_2_0
  cell_0_1 cell_1_1 cell_2_1
  cell_0_2 cell_1_2 cell_2_2
```
- We may change this to JSON for generality in file I/O section, but for now, the crux of the structure remains the same.
### Generation of C++ Headers
- The motive is to get a generation of the assets directly loaded into `glyph_data.hpp` and `sprite_data.hpp`. Then we will continue to use them.
- The Python Script will automatically generate these files (the program is called `glyph_compiler.py`).
### Engine Data Structures
- `Glyph.hpp` will consist of all the structure that will be responsible to act as an API for loading the glyph or sprite data.
### Testing the Pipeline
- A minimal test program (`tools/test_glyph.cpp`) can be compiled that:
	- Includes the generated header.
	- Iterates over all glyphs and prints them to the terminal.
	- Allows visual verification that the block characters and colors match the source art.
- This test is run manually by any artists before committing changes.
### Possible Limitations and Mitigations
- Terminal Lacks Unicode block characters:
	- Fallback to ASCII `#`, `.`, etc. This can also be configurable, hence one character can be changed to anther one for the fallback.
- Terminal lacks 24-bit color support:
	- This is slightly challenging, but downgrade to 256-color palette is the only option here (therefore we might as well ignore it for the while.)
- Sprite exceeds terminal width:
	- Simple one, we keep the track of sprite but renderer does not show it.
- Missing glyph name in map:
	- Engine logs error and displays red `?`.
---
