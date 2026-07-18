#ifndef LISTMANAGER_H
#define LISTMANAGER_H

#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

#include <Geode/Geode.hpp>
#include <matjson.hpp>

using namespace geode::prelude;

class ListManager {
public:
    inline static std::vector<int> demonIDList;
    inline static bool firstTimeOpen     = true;
    inline static int  filterType        = 0;
    inline static bool isSupremeSearching = false;

    // Cache CCSpriteFrame* with retain().
    // createWithSpriteFrame(frame) bypasses CCSpriteFrameCache::spriteFrameByName
    // (which is what happy_textures hooks and was causing crashes/slowdowns),
    // while correctly handling atlas rotation flags internally.
    inline static std::unordered_map<std::string, CCSpriteFrame*> s_frameCache;

    inline static void cacheFrame(const std::string& modId, const char* filename) {
        auto frameName = fmt::format("{}/{}", modId, filename);
        auto* frame = CCSpriteFrameCache::sharedSpriteFrameCache()
                          ->spriteFrameByName(frameName.c_str());
        if (frame) {
            frame->retain();                    // keep frame alive
            frame->getTexture()->retain();      // keep underlying texture alive
            s_frameCache[filename] = frame;
        }
    }

    inline static void preloadBgTexture() {
        static bool done = false;
        if (done) return;
        done = true;

        auto modId = Mod::get()->getID();

        // Retain bg + sheet so removeUnusedTextures() can't purge them
        auto bgPath = fmt::format("{}/GrD_demon4_bg.png", modId);
        if (auto* t = CCTextureCache::sharedTextureCache()->addImage(bgPath.c_str(), false))
            t->retain();

        auto sheetPath = fmt::format("{}/GrD_IconSheet.png", modId);
        if (auto* t = CCTextureCache::sharedTextureCache()->addImage(sheetPath.c_str(), false))
            t->retain();

        static const char* files[] = {
            "GrD_demon0.png",  "GrD_demon0_text.png",
            "GrD_demon1.png",  "GrD_demon1_text.png",
            "GrD_demon2.png",  "GrD_demon2_text.png",
            "GrD_demon3.png",  "GrD_demon3_text.png",
            "GrD_demon4.png",  "GrD_demon4_text.png",
            "GrD_demon5.png",  "GrD_demon5_text.png",
        };
        for (auto f : files) cacheFrame(modId, f);

        log::info("GrandpaDemon: cached {} frames", s_frameCache.size());
    }

    inline static CCSprite* createDemonSprite(const char* filename) {
        auto it = s_frameCache.find(filename);
        if (it != s_frameCache.end() && it->second) {
            // createWithSpriteFrame bypasses spriteFrameByName — no hook overhead,
            // handles atlas rotation correctly
            auto* spr = CCSprite::createWithSpriteFrame(it->second);
            if (spr) return spr;
        }

        // Slow fallback (before preload or cache miss)
        auto frameName = fmt::format("{}/{}", Mod::get()->getID(), filename);
        auto* spr = CCSprite::createWithSpriteFrameName(frameName.c_str());
        if (spr && spr->getContentSize().width > 0.f) return spr;

        log::warn("GrandpaDemon: sprite not found: {}", filename);
        return CCSprite::create();
    }

    inline static CCSprite* createBgSprite(const char* filename) {
        auto path = fmt::format("{}/{}", Mod::get()->getID(), filename);
        auto* spr = CCSprite::create(path.c_str());
        if (!spr || spr->getContentSize().width == 0.f) {
            log::warn("GrandpaDemon: bg sprite not found: {}", path);
            return CCSprite::create();
        }
        return spr;
    }

    inline static void parseRequestString(std::string const& str) {
        demonIDList.clear();
        auto parsed = matjson::parse(str);
        if (!parsed) { log::error("Failed to parse AREDL JSON"); return; }
        auto json = parsed.unwrap();
        if (!json.isArray()) { log::error("AREDL JSON is not an array"); return; }
        for (auto& level : json.asArray().unwrap()) {
            if (!level.isObject()) continue;
            if (!level.contains("level_id")) continue;
            auto levelId = level["level_id"];
            if (!levelId.isNumber()) continue;
            demonIDList.push_back(levelId.asInt().unwrap());
        }
        log::info("Loaded {} AREDL demon IDs", demonIDList.size());
    }

    inline static int getPositionOfID(int id) {
        for (size_t i = 0; i < demonIDList.size(); i++) {
            if (demonIDList[i] == id) return static_cast<int>(i);
        }
        return -1;
    }

    inline static CCSprite* getSpriteFromPosition(int pos, bool hasText) {
        if (pos <= 499 && pos > 249)
            return createDemonSprite(hasText ? "GrD_demon0_text.png" : "GrD_demon0.png");
        if (pos <= 249 && pos > 149)
            return createDemonSprite(hasText ? "GrD_demon1_text.png" : "GrD_demon1.png");
        if (pos <= 149 && pos > 74)
            return createDemonSprite(hasText ? "GrD_demon2_text.png" : "GrD_demon2.png");
        if (pos <= 74  && pos > 24)
            return createDemonSprite(hasText ? "GrD_demon3_text.png" : "GrD_demon3.png");
        if (pos <= 24  && pos > 0)
            return createDemonSprite(hasText ? "GrD_demon4_text.png" : "GrD_demon4.png");
        if (pos == 0) {
            bool disabled = Mod::get()->getSettingValue<bool>("grandpa-demon-disable");
            return createDemonSprite(
                hasText ? (disabled ? "GrD_demon4_text.png" : "GrD_demon5_text.png")
                        : (disabled ? "GrD_demon4.png"      : "GrD_demon5.png"));
        }
        log::warn("GrandpaDemon: position out of range: {}", pos);
        return CCSprite::create();
    }

    inline static GJSearchObject* getSearchObject(int upper, int lower) {
        std::stringstream download;
        bool first = true;
        if (!(upper == 0 && lower == 0)) {
            for (int i = upper; i > lower; i--) {
                if (!first) download << ",";
                download << demonIDList.at(i);
                first = false;
            }
        } else if (!demonIDList.empty()) {
            download << demonIDList.at(0);
        }
        download << "&gameVersion=22";
        return GJSearchObject::create(SearchType::Type19, download.str());
    }
};

#endif
