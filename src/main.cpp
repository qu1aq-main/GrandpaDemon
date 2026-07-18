#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/utils/web.hpp>
#include "ListManager.h"
#include <string>
#include <thread>

using namespace geode::prelude;

static std::string fetchAredl() {
    // Primary endpoint
    static const char* urls[] = {
        "https://api.aredl.net/v2/api/aredl/levels",
        // Mirror via raw GitHub — works even if aredl.net is blocked
        "https://raw.githubusercontent.com/aredl/aredl/refs/heads/main/public/export/levels.json",
    };

    for (auto url : urls) {
        auto res = web::WebRequest()
            .header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36")
            .header("Accept", "application/json")
            .getSync(url);

        auto str  = res.string().unwrapOr("");
        int  code = res.code();
        bool ok   = !str.empty() && (str.front() == '[' || str.front() == '{');

        if (code == 200 && ok) return str;
    }
    return "";
}

class $modify(MenuLayer) {

	bool init() {
		if (!MenuLayer::init()) return false;

		if (!ListManager::firstTimeOpen) return true;
		ListManager::firstTimeOpen = false;
		ListManager::preloadBgTexture();

		std::thread([]() {
			auto str = fetchAredl();

			if (str.empty()) {
				std::string msg =
					"<cy>Could not load AREDL data.</c>\n\n"
					"Possible reasons:\n"
					"- AREDL servers are temporarily down\n"
					"- The site is blocked in your region\n"
					"- No internet connection\n\n"
					"<cg>Restart the game to try again.</c>\n\n"
					"<cb>-Grandpa Demon</c>";

				Loader::get()->queueInMainThread([msg]() {
					ListManager::filterType = -2;
					FLAlertLayer::create("AREDL Unavailable", msg.c_str(), "OK")->show();
				});
				return;
			}

			Loader::get()->queueInMainThread([str]() {
				ListManager::parseRequestString(str);
				ListManager::filterType = -1;
			});
		}).detach();

		return true;
	}
};
