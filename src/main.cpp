#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/utils/web.hpp>
#include "ListManager.h"
#include <string>
#include <thread>

using namespace geode::prelude;

class $modify(MenuLayer) {

	bool init() {
		if (!MenuLayer::init()) return false;

		if (!ListManager::firstTimeOpen) {
			return true;
		}

		ListManager::firstTimeOpen = false;

		// Preload the large bg texture now (main thread, before any level page opens)
		// so the first visit to a Mythical/Legendary/Infinite level doesn't lag.
		ListManager::preloadBgTexture();

		std::thread([]() {
			auto res = web::WebRequest().getSync("https://api.aredl.net/v2/api/aredl/levels");
			auto str = res.string().unwrapOr("Failed.");

			if (res.code() != 200 || str == "Failed." || str == "-1") {
				std::string errorStr =
					"\n\n<cr>Could not load data from AREDL.</c>\n"
					"The API could be down, but chances are, your internet just sucks.\n\n"
					"<cg>Restart your game to try again.</c>\n\n"
					"<cb>-Grandpa Demon</c>";

				Loader::get()->queueInMainThread([str, errorStr]() {
					ListManager::filterType = -2;
					FLAlertLayer::create(
						"What the??",
						(str + errorStr).c_str(),
						"OK"
					)->show();
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
