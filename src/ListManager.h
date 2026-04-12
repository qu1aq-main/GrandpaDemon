#ifndef LISTMANAGER_H
#define LISTMANAGER_H

#include <vector>
#include <string>
#include <sstream>

#include <Geode/Geode.hpp>
#include <matjson.hpp>

using namespace geode::prelude;

class ListManager {
public:
    inline static std::vector<int> demonIDList;
    inline static bool firstTimeOpen = true;
    inline static int filterType = 0;
    inline static bool isSupremeSearching = false;

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
            if (demonIDList[i] == id)
                return static_cast<int>(i);
        }
        return -1;
    }

    // Завантажуємо через sprite atlas (spritesheets у mod.json → GrD_IconSheet).
    // Frame name: "qu1aq.grandpa_demon/filename.png" — аналог "filename.png"_spr,
    // але з динамічним іменем файлу.
    inline static CCSprite* createDemonSprite(const char* filename) {
        auto frameName = fmt::format("{}/{}", Mod::get()->getID(), filename);
        auto* spr = CCSprite::createWithSpriteFrameName(frameName.c_str());
        if (!spr || spr->getContentSize().width == 0.f) {
            log::warn("GrandpaDemon: failed to load sprite frame '{}'", frameName);
            return CCSprite::create();
        }
        return spr;
    }

    // Preloads and RETAINS both the bg and icon sheet textures so GD's
    // removeUnusedTextures() on scene transitions (level enter/exit) cannot
    // purge them. Without retain(), the textures get freed on every scene
    // switch and reloaded on the way back — causing the visible micro-freeze.
    inline static void preloadTextures() {
        static bool done = false;
        if (done) return;
        done = true;

        auto modId = Mod::get()->getID();

        // Background sprite (large — 7.5 MB, most noticeable freeze)
        auto bgPath = fmt::format("{}/GrD_demon4_bg.png", modId);
        auto* bgTex = CCTextureCache::sharedTextureCache()->addImage(bgPath.c_str(), false);
        if (bgTex) bgTex->retain(); // keep alive across scene switches

        // Icon spritesheet (plist frames used by createDemonSprite)
        auto sheetPath = fmt::format("{}/GrD_IconSheet.png", modId);
        auto* sheetTex = CCTextureCache::sharedTextureCache()->addImage(sheetPath.c_str(), false);
        if (sheetTex) sheetTex->retain();

        log::info("GrandpaDemon: preloaded and retained textures");
    }

    // Keep old name working (called from main.cpp)
    inline static void preloadBgTexture() { preloadTextures(); }

    // Bg sprite lives in sprites (not spritesheets), so Geode exposes it via
    // "<mod_id>/filename" relative path in CCFileUtils — same as demons.
    inline static CCSprite* createBgSprite(const char* filename) {
        auto relativePath = fmt::format("{}/{}", Mod::get()->getID(), filename);
        auto* spr = CCSprite::create(relativePath.c_str());
        if (!spr || spr->getContentSize().width == 0.f) {
            log::warn("GrandpaDemon: failed to load bg sprite '{}'", relativePath);
            return CCSprite::create();
        }
        return spr;
    }

    inline static CCSprite* getSpriteFromPosition(int pos, bool hasText) {
        if (pos <= 499 && pos > 249)
            return createDemonSprite(hasText ? "GrD_demon0_text.png" : "GrD_demon0.png");
        if (pos <= 249 && pos > 149)
            return createDemonSprite(hasText ? "GrD_demon1_text.png" : "GrD_demon1.png");
        if (pos <= 149 && pos > 74)
            return createDemonSprite(hasText ? "GrD_demon2_text.png" : "GrD_demon2.png");
        if (pos <= 74 && pos > 24)
            return createDemonSprite(hasText ? "GrD_demon3_text.png" : "GrD_demon3.png");
        if (pos <= 24 && pos > 0)
            return createDemonSprite(hasText ? "GrD_demon4_text.png" : "GrD_demon4.png");
        if (pos == 0) {
            bool disabled = Mod::get()->getSettingValue<bool>("grandpa-demon-disable");
            return createDemonSprite(
                hasText
                    ? (disabled ? "GrD_demon4_text.png" : "GrD_demon5_text.png")
                    : (disabled ? "GrD_demon4.png"      : "GrD_demon5.png")
            );
        }
        log::warn("GrandpaDemon: getSpriteFromPosition out of range: {}", pos);
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
