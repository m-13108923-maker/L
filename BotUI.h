#pragma once
#include <Geode/Geode.hpp>
#include "LevelBot.h"

using namespace geode::prelude;

// ─────────────────────────────────────────────────────────────────────────────
// BotUI — In-game overlay shown while the LevelBot trains/replays
// ─────────────────────────────────────────────────────────────────────────────
class BotUI : public CCLayer {
public:
    static BotUI* create();
    bool init() override;
    void update(float dt) override;

private:
    CCLabelBMFont* m_statusLabel   = nullptr;
    CCLabelBMFont* m_genLabel      = nullptr;
    CCLabelBMFont* m_fitnessLabel  = nullptr;

    void refresh();
};
