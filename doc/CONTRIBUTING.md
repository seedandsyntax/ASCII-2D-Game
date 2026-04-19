## Contributing to ASCII RPG

Welcome! This document defines **exactly** how we write code, name things, test changes, and collaborate. Following these rules keeps the project maintainable and prevents merge conflicts and architectural drift.

If something isn't covered here, refer to `docs/PDD.md`. If still unsure, ask in `#dev-general` on Discord.

---

### 1. Naming Conventions

Consistency is non‑negotiable. It reduces cognitive load when reading unfamiliar code.

#### Files and Folders

| Type             | Convention                     | Example                             |
| :--------------- | :----------------------------- | :---------------------------------- |
| C++ Source Files | `PascalCase.cpp`               | `TerminalManager.cpp`               |
| C++ Header Files | `PascalCase.hpp`               | `Color.hpp`                         |
| CMake Files      | Exactly `CMakeLists.txt`       | `CMakeLists.txt`                    |
| Asset Files      | `snake_case.ext`               | `player.png`, `dragon.sprite`       |
| Documentation    | `UPPER_SNAKE_CASE.md`          | `CONTRIBUTING.md`, `PDD.md`         |
| Folders          | `PascalCase/` or `snake_case/` | `src/Engine/`, `assets/source_art/` |

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
- There are only three kinds of branches, these are the types. (`feature`, `fix`, `experiment`)

| Type             | Convention                     | Example                     |
| :--------------- | :----------------------------- | :-------------------------- |
| Feature branches | `feature/short-description`    | `feature/inventory-system`  |
| Bugfix branches  | `fix/short-description`        | `fix/terminal-resize-crash` |
| Experimental     | `experiment/short-description` | `experiment/sixel-support`  |

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
