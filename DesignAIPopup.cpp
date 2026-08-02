#include "DesignAIPopup.h"
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

DesignAIPopup* DesignAIPopup::create() {
    auto* ret = new DesignAIPopup();
    if (ret && ret->initAnchored(420.f, 320.f)) { ret->autorelease(); return ret; }
    delete ret; return nullptr;
}

bool DesignAIPopup::setup() {
    setTitle("Level Design AI");

    auto* size = m_mainLayer->getContentSize();
    float cx   = size.width  / 2.f;
    float cy   = size.height / 2.f;

    // Prompt input
    auto* promptLabel = CCLabelBMFont::create("Describe your level:", "goldFont.fnt");
    promptLabel->setScale(0.55f);
    promptLabel->setPosition({cx, size.height - 55.f});
    m_mainLayer->addChild(promptLabel);

    m_input = TextInput::create(380.f, "e.g. A dark space level with lots of ship sections and spikes...");
    m_input->setPosition({cx, size.height - 90.f});
    m_mainLayer->addChild(m_input);

    // Ask button
    auto* askBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Ask AI", "goldFont.fnt", "GJ_button_01.png", 0.8f),
        this,
        menu_selector(DesignAIPopup::onAsk)
    );
    auto* menu = CCMenu::create(askBtn, nullptr);
    menu->setPosition({cx, size.height - 130.f});
    m_mainLayer->addChild(menu);

    // Status label
    m_statusLabel = CCLabelBMFont::create("Enter a description and press Ask AI", "chatFont.fnt");
    m_statusLabel->setScale(0.45f);
    m_statusLabel->setPosition({cx, size.height - 160.f});
    m_mainLayer->addChild(m_statusLabel);

    // Scrollable result area
    m_resultScroll = ScrollLayer::create({380.f, 110.f});
    m_resultScroll->setPosition({cx - 190.f, 16.f});
    m_mainLayer->addChild(m_resultScroll);

    m_resultLabel = CCLabelBMFont::create("", "chatFont.fnt");
    m_resultLabel->setAnchorPoint({0.f, 1.f});
    m_resultLabel->setScale(0.42f);
    m_resultLabel->setWidth(360.f);
    m_resultScroll->m_contentLayer->addChild(m_resultLabel);

    return true;
}

void DesignAIPopup::onAsk(CCObject*) {
    std::string prompt = m_input->getString();
    if (prompt.empty()) {
        setStatus("Please enter a level description first.", true);
        return;
    }
    setStatus("Asking Gemini AI... please wait.");

    LevelDesignAI::ask(prompt, [this](LevelDesignAI::Suggestion s) {
        if (!s.success) {
            setStatus(s.errorMsg, true);
            return;
        }
        showResult(s);
    });
}

void DesignAIPopup::showResult(const LevelDesignAI::Suggestion& s) {
    setStatus("Done! Scroll down to read suggestions.");

    std::string text =
        "Theme: "         + s.theme        + "\n" +
        "Difficulty: "    + s.difficulty   + "\n" +
        "Song: "          + s.songAdvice   + "\n" +
        "Objects: "       + s.blockIdeas   + "\n" +
        "Colors: "        + s.colorScheme  + "\n" +
        "Triggers: "      + s.triggerAdvice+ "\n\n" +
        s.fullText;

    m_resultLabel->setString(text.c_str());
    // Resize content layer to fit
    float textH = m_resultLabel->getContentSize().height * m_resultLabel->getScale() + 8.f;
    m_resultScroll->m_contentLayer->setContentSize({380.f, std::max(textH, 110.f)});
    m_resultLabel->setPositionY(m_resultScroll->m_contentLayer->getContentSize().height);
    m_resultScroll->scrollToTop();
}

void DesignAIPopup::setStatus(const std::string& msg, bool isError) {
    m_statusLabel->setString(msg.c_str());
    m_statusLabel->setColor(isError ? ccRED : ccWHITE);
}
