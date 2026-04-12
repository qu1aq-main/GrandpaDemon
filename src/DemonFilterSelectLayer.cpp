#include <Geode/Bindings.hpp>
#include <Geode/modify/DemonFilterSelectLayer.hpp>
#include "ListManager.h"

class $modify(GrdDemonFilterSelectLayer, DemonFilterSelectLayer) {

    struct Fields {
        CCSprite* m_demon0Spr = nullptr;
        CCSprite* m_demon1Spr = nullptr;
        CCSprite* m_demon2Spr = nullptr;
        CCSprite* m_demon3Spr = nullptr;
        CCSprite* m_demon4Spr = nullptr;
        CCSprite* m_demon5Spr = nullptr;
    };

    static void onModify(auto &self) {
        (void)self.setHookPriority("DemonFilterSelectLayer::init", -1);
    }

    bool init() {
        DemonFilterSelectLayer::init();

        if (ListManager::demonIDList.empty()) return true;

        handleTouchPriority(this);
        CCLayer* layer = nullptr;
        for (auto* o : CCArrayExt<CCNode*>(this->getChildren())) {
            if (auto n = static_cast<CCLayer*>(o)) layer = n;
        }
        if (!layer) return true;

        CCScale9Sprite* s9spr = nullptr;
        CCLabelBMFont* label = nullptr;
        CCMenu* menu = nullptr;
        for (auto* o : CCArrayExt<CCNode*>(layer->getChildren())) {
            if (auto n = dynamic_cast<CCScale9Sprite*>(o)) s9spr = n;
            else if (auto n = dynamic_cast<CCLabelBMFont*>(o)) label = n;
            else if (auto n = dynamic_cast<CCMenu*>(o)) menu = n;
        }
        if (!s9spr || !label || !menu) return true;

        CCMenuItemSpriteExtra* okButton = nullptr;
        for (auto* o : CCArrayExt<CCNode*>(menu->getChildren())) {
            if (auto n = dynamic_cast<CCMenuItemSpriteExtra*>(o)) {
                if (n->getPositionY() < 0) { okButton = n; break; }
            }
        }

        auto csize = s9spr->getContentSize();
        s9spr->setContentSize({csize.width, csize.height * 1.5f});
        auto newcsize = s9spr->getContentSize();
        label->setPositionY(newcsize.height + 0.f);
        label->setScale(0.8f);
        menu->setPositionY(newcsize.height * 0.82f);
        if (okButton) okButton->setPositionY(-160.f);

        handleTouchPriority(this);

        auto newMenu = CCMenu::create();
        layer->addChild(newMenu);
        newMenu->setPosition({layer->getContentSize().width / 2 - 3, layer->getContentSize().height * 0.38f});
        newMenu->setLayout(RowLayout::create()->setGap(-1.f)->setAxisAlignment(AxisAlignment::Center));

        auto instantSearch = CCLabelBMFont::create("Instant Search", "bigFont.fnt");
        instantSearch->setPosition({label->getPositionX(), layer->getContentSize().height * 0.52f});
        instantSearch->setScale(0.8f);
        layer->addChild(instantSearch);

        // Use natural sprite size from the spritesheet — the sprites were designed
        // to be the correct size. No manual scaling needed.
        auto addBtn = [&](const char* file, SEL_MenuHandler sel, CCSprite*& field) {
            auto* spr = ListManager::createDemonSprite(file);
            field = spr;
            newMenu->addChild(CCMenuItemSpriteExtra::create(spr, this, sel));
        };

        addBtn("GrD_demon0_text.png", menu_selector(GrdDemonFilterSelectLayer::onButton0), this->m_fields->m_demon0Spr);
        addBtn("GrD_demon1_text.png", menu_selector(GrdDemonFilterSelectLayer::onButton1), this->m_fields->m_demon1Spr);
        addBtn("GrD_demon2_text.png", menu_selector(GrdDemonFilterSelectLayer::onButton2), this->m_fields->m_demon2Spr);
        addBtn("GrD_demon3_text.png", menu_selector(GrdDemonFilterSelectLayer::onButton3), this->m_fields->m_demon3Spr);
        addBtn("GrD_demon4_text.png", menu_selector(GrdDemonFilterSelectLayer::onButton4), this->m_fields->m_demon4Spr);

        if (!Mod::get()->getSettingValue<bool>("grandpa-demon-disable"))
            addBtn("GrD_demon5_text.png", menu_selector(GrdDemonFilterSelectLayer::onButton5), this->m_fields->m_demon5Spr);

        newMenu->updateLayout();
        return true;
    }

    void onButton0(CCObject* s) { ListManager::filterType=0; ListManager::isSupremeSearching=true;  geode::cocos::switchToScene(LevelBrowserLayer::create(ListManager::getSearchObject(349,249))); }
    void onButton1(CCObject* s) { ListManager::filterType=1; ListManager::isSupremeSearching=false; geode::cocos::switchToScene(LevelBrowserLayer::create(ListManager::getSearchObject(249,149))); }
    void onButton2(CCObject* s) { ListManager::filterType=2; ListManager::isSupremeSearching=false; geode::cocos::switchToScene(LevelBrowserLayer::create(ListManager::getSearchObject(149,74)));  }
    void onButton3(CCObject* s) { ListManager::filterType=3; ListManager::isSupremeSearching=false; geode::cocos::switchToScene(LevelBrowserLayer::create(ListManager::getSearchObject(74,24)));   }
    void onButton4(CCObject* s) { ListManager::filterType=4; ListManager::isSupremeSearching=false; geode::cocos::switchToScene(LevelBrowserLayer::create(ListManager::getSearchObject(24,0)));    }
    void onButton5(CCObject* s) { ListManager::filterType=5; ListManager::isSupremeSearching=false; geode::cocos::switchToScene(LevelBrowserLayer::create(ListManager::getSearchObject(0,0)));     }
};
