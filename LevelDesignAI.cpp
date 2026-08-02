#include "LevelDesignAI.h"
#include <Geode/utils/web.hpp>
#include <Geode/loader/Mod.hpp>
#include <matjson.hpp>
#include <sstream>

using namespace geode::prelude;

// ─────────────────────────────────────────────────────────────────────────────
// Gemini REST endpoint (free model: gemini-1.5-flash — no billing required)
// Get a free key at: https://aistudio.google.com/app/apikey
// ─────────────────────────────────────────────────────────────────────────────
static constexpr const char* GEMINI_MODEL = "gemini-1.5-flash";
static constexpr const char* GEMINI_BASE  =
    "https://generativelanguage.googleapis.com/v1beta/models/";

std::string LevelDesignAI::getApiKey() {
    return Mod::get()->getSettingValue<std::string>("gemini-api-key");
}

std::string LevelDesignAI::buildPrompt(const std::string& userPrompt) {
    return
        "You are an expert Geometry Dash level designer. "
        "A player has described the level they want to create. "
        "Reply with EXACTLY these labelled lines and nothing else:\n"
        "THEME: <one-line theme>\n"
        "DIFFICULTY: <Easy|Normal|Hard|Harder|Insane|Demon>\n"
        "SONG_ADVICE: <brief song recommendation>\n"
        "BLOCK_IDEAS: <comma-separated list of GD object types/IDs to use>\n"
        "COLOR_SCHEME: <bg hex, ground hex, object hex>\n"
        "TRIGGER_ADVICE: <portal, speed change, and trigger suggestions>\n"
        "DETAILS:\n<paragraph of additional design tips>\n\n"
        "Player description: " + userPrompt;
}

LevelDesignAI::Suggestion LevelDesignAI::parseSuggestion(const std::string& raw) {
    Suggestion s;
    s.fullText = raw;
    s.success  = true;

    auto extract = [&](const std::string& label) -> std::string {
        auto pos = raw.find(label + ": ");
        if (pos == std::string::npos) return "";
        pos += label.size() + 2;
        auto end = raw.find('\n', pos);
        return raw.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
    };

    s.theme        = extract("THEME");
    s.difficulty   = extract("DIFFICULTY");
    s.songAdvice   = extract("SONG_ADVICE");
    s.blockIdeas   = extract("BLOCK_IDEAS");
    s.colorScheme  = extract("COLOR_SCHEME");
    s.triggerAdvice= extract("TRIGGER_ADVICE");

    // Grab the DETAILS paragraph
    auto pos = raw.find("DETAILS:\n");
    if (pos != std::string::npos)
        s.fullText = raw.substr(pos + 9);

    return s;
}

void LevelDesignAI::ask(const std::string& userPrompt, Callback cb) {
    std::string apiKey = getApiKey();
    if (apiKey.empty()) {
        Suggestion err;
        err.success  = false;
        err.errorMsg = "No Gemini API key set. Go to Mod Settings and paste your free key from https://aistudio.google.com/app/apikey";
        Loader::get()->queueInMainThread([cb, err]() { cb(err); });
        return;
    }

    std::string url =
        std::string(GEMINI_BASE) + GEMINI_MODEL +
        ":generateContent?key=" + apiKey;

    std::string prompt = buildPrompt(userPrompt);

    // Build JSON body
    auto body = matjson::makeObject({
        {"contents", matjson::makeArray({
            matjson::makeObject({
                {"parts", matjson::makeArray({
                    matjson::makeObject({{"text", prompt}})
                })}
            })
        })}
    });

    web::WebRequest req;
    req.header("Content-Type", "application/json");
    req.bodyJSON(body);

    req.post(url)
       .map([](web::WebResponse* res) -> std::string {
           if (!res->ok()) return "";
           auto json = res->json();
           if (!json.has_value()) return "";
           auto& j = json.value();
           // Navigate: candidates[0].content.parts[0].text
           if (!j.contains("candidates")) return "";
           auto cands = j["candidates"];
           if (!cands.isArray() || cands.asArray().value().empty()) return "";
           auto first = cands[0];
           if (!first.contains("content")) return "";
           auto parts = first["content"]["parts"];
           if (!parts.isArray() || parts.asArray().value().empty()) return "";
           return parts[0]["text"].asString().value_or("");
       })
       .listen([cb](auto* result) {
           Suggestion s;
           if (!result || result->isErr()) {
               s.success  = false;
               s.errorMsg = "Network error — check your internet connection.";
           } else {
               std::string text = **result;
               if (text.empty()) {
                   s.success  = false;
                   s.errorMsg = "Gemini returned an empty response. Check your API key.";
               } else {
                   s = parseSuggestion(text);
               }
           }
           cb(s);
       });
}
