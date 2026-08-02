#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "LevelDesignAI.h"

using namespace geode::prelude;

// ─────────────────────────────────────────────────────────────────────────────
// DesignAIPopup — modal dialog accessed from the editor toolbar
// ─────────────────────────────────────────────────────────────────────────────
class DesignAIPopup : public Popup<> {
public:
    static DesignAIPopup* create();

protected:
    bool setup() override;

private:
    TextInput*      m_input       = nullptr;
    CCLabelBMFont*  m_statusLabel = nullptr;
    ScrollLayer*    m_resultScroll= nullptr;
    CCLabelBMFont*  m_resultLabel = nullptr;

    void onAsk(CCObject*);
    void showResult(const LevelDesignAI::Suggestion& s);
    void setStatus(const std::string& msg, bool isError = false);
};
