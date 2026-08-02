#include "BotUI.h"
#include <Geode/cocos/CCDirector.h>

using namespace geode::prelude;

BotUI* BotUI::create() {
    auto* ret = new BotUI();
    if (ret && ret->init()) { ret->autorelease(); return ret; }
    delete ret; return nullptr;
}

bool BotUI::init() {
    if (!CCLayer::init()) return false;

    auto* winSize = CCDirector::get()->getWinSize();

    // Background panel
    auto* bg = CCLayerColor::create({0, 0, 0, 160}, 220.f, 80.f);
    bg->setPosition({8.f, winSize.height - 88.f});
    addChild(bg, 0);

    float ox = 16.f, oy = winSize.height - 32.f;

    m_statusLabel = CCLabelBMFont::create("Bot: Idle", "bigFont.fnt");
    m_statusLabel->setScale(0.4f);
    m_statusLabel->setAnchorPoint({0.f, 0.5f});
    m_statusLabel->setPosition({ox, oy});
    addChild(m_statusLabel, 1);

    m_genLabel = CCLabelBMFont::create("Gen: 0", "bigFont.fnt");
    m_genLabel->setScale(0.35f);
    m_genLabel->setAnchorPoint({0.f, 0.5f});
    m_genLabel->setPosition({ox, oy - 22.f});
    addChild(m_genLabel, 1);

    m_fitnessLabel = CCLabelBMFont::create("Best: 0.0", "bigFont.fnt");
    m_fitnessLabel->setScale(0.35f);
    m_fitnessLabel->setAnchorPoint({0.f, 0.5f});
    m_fitnessLabel->setPosition({ox, oy - 44.f});
    addChild(m_fitnessLabel, 1);

    scheduleUpdate();
    return true;
}

void BotUI::refresh() {
    auto& bot = LevelBot::get();
    const char* stateStr = "Idle";
    switch (bot.getState()) {
        case LevelBot::State::Training:  stateStr = "Training"; break;
        case LevelBot::State::Paused:    stateStr = "Paused";   break;
        case LevelBot::State::Completed: stateStr = "Solved!";  break;
        default: break;
    }
    m_statusLabel->setString(fmt::format("Bot: {}", stateStr).c_str());
    m_genLabel->setString(fmt::format("Gen: {}", bot.getGeneration()).c_str());
    m_fitnessLabel->setString(fmt::format("Best: {:.1f}%", bot.getBestFitness()).c_str());
}

void BotUI::update(float /*dt*/) { refresh(); }
