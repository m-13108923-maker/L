# GD AI Mod — Geode Mod for Geometry Dash

Two AI tools in one mod:
1. **Level Design AI** — describe your level, get structured design suggestions powered by Google Gemini (free).
2. **Level Bot** — a genetic algorithm that trains itself to complete any level. No API key or internet required.

---

## Quick Start

### Step 1 — Prerequisites

| Tool | Where to get it |
|------|----------------|
| Geometry Dash (Steam or official) | Steam / store |
| Geode mod loader | https://geode-sdk.org |
| CMake ≥ 3.21 | https://cmake.org |
| Geode SDK | https://github.com/geode-sdk/geode (follow their build guide) |
| A C++ compiler | MSVC (Windows) / Clang (macOS/Linux) |

### Step 2 — Build

```bash
# Clone / unzip this folder, then:
cd geode-ai-mod
mkdir build && cd build

# Point CMake at your Geode SDK (adjust path):
set GEODE_SDK=C:\Users\you\GeodeSDK      # Windows
export GEODE_SDK=~/GeodeSDK              # macOS/Linux

cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --config RelWithDebInfo
```

After building you'll find a file like `build/GDAIMod.geode`.

### Step 3 — Install

Copy `GDAIMod.geode` into your Geode mods folder:
- **Windows:** `%AppData%\Geode\mods\`
- **macOS:** `~/Library/Application Support/Geode/mods/`

Launch Geometry Dash. The mod appears in the Geode menu.

---

## Level Design AI

### Setup (one time)
1. Visit https://aistudio.google.com/app/apikey and create a **free** API key (no credit card needed).
2. In GD → Geode → GD AI Mod → Settings → paste the key into **Gemini API Key**.

### Using it
1. Open the **Editor**.
2. Click the **[AI]** button in the toolbar (top-right area).
3. Describe your level in plain English, e.g.:
   > *"A dark space level with lots of ship sections, tight corridors and sudden speed boosts"*
4. Press **Ask AI**. Suggestions appear in a few seconds:
   - Theme & difficulty
   - Song recommendation
   - Object / block ideas
   - Color scheme (hex codes)
   - Portal & trigger advice
   - Detailed design tips

The model used is **gemini-1.5-flash** — Google's free tier, no billing required.

---

## Level Bot (Genetic Algorithm)

No internet or API key needed. Runs fully on your PC.

### How it works
- The bot keeps a **population of action sequences** (jump or don't jump per frame).
- Each genome "plays" the level silently. The further it gets, the higher its fitness score.
- The top performers are kept, crossed together, and slightly mutated to form the next generation.
- This repeats until the level is solved or you stop it.

### Controls (while playing a level)

| Key | Action |
|-----|--------|
| `T` | Start training (or stop if already running) |
| `P` | Pause / resume training |
| `R` | Replay the best genome found so far |

A small HUD in the top-left shows: **Bot status / Generation / Best fitness %**.

### Settings (Geode → GD AI Mod → Settings)

| Setting | Default | Effect |
|---------|---------|--------|
| Bot Training Speed | 30 | Jump actions attempted per second |
| Population Size | 50 | Genomes per generation — more = slower but smarter |

### Tips
- Start with **Population Size 50** on normal/hard levels.
- Increase to **100–150** for demon-tier levels.
- Let it run for 20–50 generations — fitness should climb steadily.
- Press `R` at any time to watch the best run so far without stopping training.

---

## File Structure

```
geode-ai-mod/
├── mod.json               ← Geode manifest (ID, version, settings)
├── CMakeLists.txt         ← Build config
├── README.md              ← This file
└── src/
    ├── main.cpp           ← Geode hooks (PlayLayer, EditorUI, GJBaseGameLayer)
    ├── LevelDesignAI.h/cpp← Gemini API client
    ├── LevelBot.h/cpp     ← Genetic algorithm bot
    ├── BotUI.h/cpp        ← In-game HUD overlay
    ├── DesignAIPopup.h/cpp← Editor popup for the Design AI
```

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| "No Gemini API key set" | Paste your key in Mod Settings |
| "Gemini returned empty response" | Key may be invalid or rate-limited — check aistudio.google.com |
| Bot never improves past ~30% | Try increasing Population Size to 100 |
| Build error: GEODE_SDK not found | Set the `GEODE_SDK` environment variable to your SDK path |
| Crash on launch | Make sure your Geode and GD versions match `mod.json` |

---

## License

MIT — do whatever you want with this.
