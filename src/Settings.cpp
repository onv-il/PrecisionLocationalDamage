#include "boost/algorithm/string.hpp"

#include "nlohmann/json.hpp"
#include "tojson.h"

#include "LocationalDamageHandler.h"
#include "Settings.h"

using namespace nlohmann;

std::vector<Settings::HitEffect> g_hitEffectVector;

namespace Settings {
	void RunConfig(json& a_jsonData) {

		auto dataHandler = RE::TESDataHandler::GetSingleton();

		for (auto& element : a_jsonData) {
			HitEffect newHitEffect;

			if (element.contains("damageMult")) {
				newHitEffect.damageMult = element["damageMult"];
			}

			if (element.contains("weaponKeywords")) {
				if (element["weaponKeywords"].is_array()) {
					for (auto& entry : element["weaponKeywords"]) {
						std::string input = entry;

						std::vector<std::string> result;

						boost::split(result, input, boost::is_any_of("|"));

						std::string pluginName = result[0];
						std::string formIDString = result[1];

						int formID = stoi(formIDString, nullptr, 0);

						RE::BGSKeyword* newKeyword = dataHandler->LookupForm(formID, pluginName)->As<RE::BGSKeyword>();

						if (newKeyword) {
							newHitEffect.weaponKeywords.push_back(newKeyword);
						} else {
							WARN("could not get keyword from {}", input)
						}
						
					}

				} else {
					std::string input = element["weaponKeywords"];

					std::vector<std::string> result;

					boost::split(result, input, boost::is_any_of("|"));

					std::string pluginName = result[0];
					std::string formIDString = result[1];

					int formID = stoi(formIDString, nullptr, 0);

					RE::BGSKeyword* newKeyword = dataHandler->LookupForm(formID, pluginName)->As<RE::BGSKeyword>();

					if (newKeyword) {
						newHitEffect.weaponKeywords.push_back(newKeyword);
					} else {
						WARN("could not get keyword from {}", input)
					}
				}
			}

			if (element.contains("spell")) {
				std::string input = element["spell"];

				std::vector<std::string> result;

				boost::split(result, input, boost::is_any_of("|"));

				std::string pluginName = result[0];
				std::string formIDString = result[1];

				int formID = stoi(formIDString, nullptr, 0);

				RE::SpellItem* newSpellItem = dataHandler->LookupForm(formID, pluginName)->As<RE::SpellItem>();

				if (newSpellItem) {
					newHitEffect.spellForm = newSpellItem;
				} else {
					WARN("could not get spell from {}", input)
				}
				
			}

			if (element.contains("spellOnlyCriticalHits")) {
				newHitEffect.spellOnlyCriticalHits = element["spellOnlyCriticalHits"];
			}

			if (element.contains("spellOnlyPowerAttacks")) {
				newHitEffect.spellOnlyPowerAttacks = element["spellOnlyPowerAttacks"];
			}

			auto nodeNames = element.find("nodeNames");

			if (nodeNames != element.end()) {
				if (nodeNames->is_array()) {
					newHitEffect.nodeNames = element["nodeNames"].get<std::vector<std::string>>();
				} else {
					newHitEffect.nodeNames.push_back(*nodeNames);
				}
			}

			g_hitEffectVector.push_back(newHitEffect);
		}

		INFO("finished parsing .yamls :)")

		LocationalDamageHandler::Initialize();
	}

	void ParseConfigs(std::set<std::string>& a_configs) {
		for (auto config : a_configs) {
			auto path = std::filesystem::path(config).filename();
			auto filename = path.string();
			INFO("Parsing {}", filename);

			try {
				std::ifstream i(config);
				if (i.good()) {
					json data;
					try {
						INFO("Converting {} to JSON object", filename);
						data = tojson::loadyaml(config);
					}
					catch (const std::exception& exc) {
						std::string errorMessage = std::format("Failed to convert {} to JSON object\n{}", filename, exc.what());
						ERROR("{}", errorMessage);
						continue;
					}
					i.close();
					RunConfig(data);
				}
				else {
					std::string errorMessage = std::format("Failed to parse {}\nBad file stream", filename);
					ERROR("{}", errorMessage);
				}
			}
			catch (const std::exception& exc) {
				std::string errorMessage = std::format("Failed to parse {}\n{}", filename, exc.what());
				ERROR("{}", errorMessage);
			}
		}
	}

	void Initialize() {

		std::set<std::string> configs;

		auto constexpr folder = R"(Data\)"sv;
		for (const auto& entry : std::filesystem::directory_iterator(folder)) {
			if (entry.exists() && !entry.path().empty() && (entry.path().extension() == ".yaml"sv)) {
				const auto path = entry.path().string();
				const auto filename = entry.path().filename().string();
				auto lastindex = filename.find_last_of(".");
				auto rawname = filename.substr(0, lastindex);
				if (rawname.ends_with("_PLD")) {
					const auto path = entry.path().string();
					configs.insert(path);
				}
			}
		}

		if (configs.empty()) {
			WARN("no valid .yaml files found; returning")
			return;
		}

		ParseConfigs(configs);
	}
}