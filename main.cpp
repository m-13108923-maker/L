#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "LevelBot.h"
#include "BotUI.h"
#include "DesignAIPopup.h"

using namespace geode::prelude;

// ─────────────────────────────────────────────────────────────────────────────
// PlayLayer hooks — wire the LevelBot into the game loop
// ─────────────────────────────────────────────────────────────────────────────
class $modify(BotPlayLayer, PlayLayer) {
    struct Fields {
        BotUI* botUI = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // If bot is training or replaying, inject the HUD overlay
        auto& bot = LevelBot::get();
        if (bot.getState() == LevelBot::State::Training ||
            bot.getState() == LevelBot::State::Completed) {
            auto* ui = BotUI::create();
            if (ui) {
                m_fields->botUI = ui;
                addChild(ui, 100);
            }
        }
        return true;
    }

    // Player died
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);

        auto& bot = LevelBot::get();
        if (bot.getState() == LevelBot::State::Training) {
            float dist = m_player1 ? m_player1->getPositionX() : 0.f;
            bot.onRunEnd(dist, false);
            // Restart the attempt immediately
            resetLevel();
        }
    }

    // Level complete
    void levelComplete() {
        PlayLayer::levelComplete();
        auto& bot = LevelBot::get();
        if (bot.getState() == LevelBot::State::Training) {
            float dist = m_player1 ? m_player1->getPositionX() : 999999.f;
            bot.onRunEnd(dist, true);
        }
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// GJBaseGameLayer — inject bot clicks each physics update
// ─────────────────────────────────────────────────────────────────────────────
class $modify(BotGameLayer, GJBaseGameLayer) {
    void update(float dt) {
        GJBaseGameLayer::update(dt);

        auto& bot = LevelBot::get();
        if (bot.getState() != LevelBot::State::Training) return;
        if (!m_player1) return;

        bool isDead = m_player1->m_isDead;
        bool click  = bot.onPhysicsFrame(
            m_player1->getPositionX(),
            m_player1->getPositionY(),
            isDead
        );

        if (click && !isDead)
            m_player1->pushButton(PlayerButton::Jump);
        else
            m_player1->releaseButton(PlayerButton::Jump);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// EditorUI hook — add AI and Bot buttons to the editor toolbar
// ─────────────────────────────────────────────────────────────────────────────
class $modify(AIEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        // Find the top-right toolbar menu
        auto* menu = this->getChildByID("toolbar-categories-menu");
        if (!menu) menu = this->getChildByType<CCMenu>(0);
        if (!menu) return true;

        // "Design AI" button
        auto* aiBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("AI", "bigFont.fnt", "GJ_button_05.png", 0.7f),
            this,
            menu_selector(AIEditorUI::onOpenDesignAI)
        );
        aiBtn->setID("gd-ai-design-button");
        menu->addChild(aiBtn);
        menu->updateLayout();

        return true;
    }

    void onOpenDesignAI(CCObject*) {
        DesignAIPopup::create()->show();
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Keybinds (accessible from Pause menu while playing)
// T = start/stop training   P = pause/resume   R = replay best
// ─────────────────────────────────────────────────────────────────────────────
$on_mod(Loaded) {
    // Register keyboard listener on the event dispatcher
    auto* listener = CCEventListenerKeyboard::create();
    listener->onKeyPressed = [](cocos2d::EventKeyboard::KeyCode key, cocos2d::Event*) {
        auto& bot = LevelBot::get();
        if (key == cocos2d::EventKeyboard::KeyCode::KEY_T) {
            if (bot.getState() == LevelBot::State::Idle ||
                bot.getState() == LevelBot::State::Completed)
                bot.startTraining();
            else
                bot.stopTraining();
        }
        if (key == cocos2d::EventKeyboard::KeyCode::KEY_P)
            bot.pauseTraining();
        if (key == cocos2d::EventKeyboard::KeyCode::KEY_R)
            bot.replayBest();
    };
    CCDirector::get()->getEventDispatcher()->addEventListenerWithFixedPriority(listener, 1);

    log::info("GD AI Mod loaded. T=Train R=Replay P=Pause  |  Editor: click [AI] button");
}
