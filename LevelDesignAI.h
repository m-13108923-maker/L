#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <functional>

using namespace geode::prelude;

// ─────────────────────────────────────────────────────────────────────────────
// LevelDesignAI
//
// Sends the user's level description to the Gemini API (free tier) and returns
// structured level-design suggestions in a callback.
// ─────────────────────────────────────────────────────────────────────────────
class LevelDesignAI {
public:
    struct Suggestion {
        std::string theme;          // e.g. "Space / neon"
        std::string difficulty;     // e.g. "Hard"
        std::string songAdvice;     // e.g. "Use a fast-paced EDM track"
        std::string blockIdeas;     // Comma-separated GD object suggestions
        std::string colorScheme;    // e.g. "bg:#0a0a2e, ground:#1a1aff"
        std::string triggerAdvice;  // Gameplay tips (portals, speed changes, etc.)
        std::string fullText;       // Raw Gemini response
        bool        success;
        std::string errorMsg;
    };

    using Callback = std::function<void(Suggestion)>;

    // Send a prompt to Gemini and call `cb` on the game thread when done.
    static void ask(const std::string& userPrompt, Callback cb);

private:
    static std::string buildPrompt(const std::string& userPrompt);
    static Suggestion  parseSuggestion(const std::string& raw);
    static std::string getApiKey();
};
