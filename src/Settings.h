#pragma once

namespace Settings {
	struct HitEffect {

		float damageMult;

		std::vector<RE::BGSKeyword*> weaponKeywords;

		RE::SpellItem* spellForm;

		bool spellOnlyCriticalHits;
		bool spellOnlyPowerAttacks;

		std::vector<std::string> nodeNames;
	};

	void Initialize();
}