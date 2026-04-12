#include <Geode/Bindings.hpp>
#include <Geode/modify/LevelCell.hpp>
#include "ListManager.h"
#include "EffectsManager.h"

using namespace geode::prelude;

class $modify(LevelCell) {

    void loadCustomLevelCell() {
        LevelCell::loadCustomLevelCell();

        if (ListManager::demonIDList.size() == 0) {
            return;
        }

        if (m_level->m_stars != 10) {
            return;
        }

        int aredlPos = ListManager::getPositionOfID(m_level->m_levelID);
        if (aredlPos == -1 || aredlPos > 499) {
            return;
        }

        CCSprite* originalIcon = nullptr;

        for (auto* obj : CCArrayExt<CCNode*>(m_mainLayer->getChildren())) {
            if (CCNode* newObj = dynamic_cast<CCNode*>(obj)) {
                if (newObj->getZOrder() == 2) {
                    newObj->setID("grd-demon-icon-layer");
                    for (auto* obj2 : CCArrayExt<CCNode*>(newObj->getChildren())) {
                        if (CCSprite* newObj2 = dynamic_cast<CCSprite*>(obj2)) {
                            if (newObj2->getZOrder() == 3) {
                                originalIcon = newObj2;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (originalIcon == nullptr) {
            return;
        }

        CCSprite* newIcon = ListManager::getSpriteFromPosition(aredlPos, false);
        if (!newIcon) return;
        auto layer = m_mainLayer->getChildByID("grd-demon-icon-layer");
        if (!layer) return;

        auto newPos = originalIcon->getPosition();
        newIcon->setPosition(originalIcon->getPosition());
        newIcon->setZOrder(originalIcon->getZOrder()+25);
        if (newIcon->getContentSize().height > 0) {
            float targetH = originalIcon->getContentSize().height * originalIcon->getScale();
            float fillRatio = 0.95f;
            newIcon->setScale((targetH / newIcon->getContentSize().height) / fillRatio);
        }
        
        for (auto* clearObj : CCArrayExt<CCNode*>(originalIcon->getChildren())) {
            if (CCSprite* newObj = dynamic_cast<CCSprite*>(clearObj)) {
                if (newObj->getTag() == 69420) {
                    newObj->removeFromParentAndCleanup(true);
                }
            }
        }

        for (auto* iconObj : CCArrayExt<CCNode*>(originalIcon->getChildren())) {
            if (CCSprite* newObj = dynamic_cast<CCSprite*>(iconObj)) {
                newObj->setTag(69420);
                layer->addChild(newObj);
                newObj->setPosition(newPos);
            }
        }

        originalIcon->setVisible(false);
        layer->addChild(newIcon);
    }
};
