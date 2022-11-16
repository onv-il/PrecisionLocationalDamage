#include "LocationalDamageHandler.h"
#include "PrecisionAPI.h"

namespace LocationalDamageHandler
{

	auto magicSource = RE::MagicSystem::CastingSource::kInstant;

	PRECISION_API::PreHitCallbackReturn OnPrecisionPreHit(const PRECISION_API::PrecisionHitData& a_precisionHitData) {
		PRECISION_API::PreHitCallbackReturn ret;

		RE::TESObjectREFR* target = a_precisionHitData.target;
		RE::hkpRigidBody* hitRigidBody = a_precisionHitData.hitRigidBody;

		if (!target) {
			return ret;
		}

		if (!hitRigidBody) {
			return ret;
		}

		if (target->GetFormType() != RE::FormType::ActorCharacter) {
			return ret;
		}

		if (target->IsDead()) {
			return ret;
		}

		//ctds happen if we expect the node to have a name by default
		//plus the entire mechanic kind of hinges on nodes having a name
		if (!hitRigidBody->name.data()) {
			return ret;
		}

		std::string hitRigidBodyName = hitRigidBody->name.data();

		PRECISION_API::PreHitModifier newModifier;

		newModifier.modifierOperation = PRECISION_API::PreHitModifier::ModifierOperation::Multiplicative;
		newModifier.modifierType = PRECISION_API::PreHitModifier::ModifierType::Damage;
		newModifier.modifierValue = 1.0f;

		for (auto& hitEffect : g_hitEffectVector) {
			[&] {
				if (find(hitEffect.nodeNames.begin(), hitEffect.nodeNames.end(), hitRigidBodyName) == hitEffect.nodeNames.end())
					return;

				if (hitEffect.damageMult)
					newModifier.modifierValue = hitEffect.damageMult;
			}();
		}

		ret.modifiers.push_back(newModifier);

		return ret;
	}

	void OnPrecisionPostHit(const PRECISION_API::PrecisionHitData& a_precisionHitData, const RE::HitData& a_vanillaHitData)
	{
		RE::TESObjectREFR* target = a_precisionHitData.target;
		RE::hkpRigidBody* hitRigidBody = a_precisionHitData.hitRigidBody;

		if (!target) {
			return;
		}

		if (!hitRigidBody) {
			return;
		}

		if (target->GetFormType() != RE::FormType::ActorCharacter) {
			return;
		}

		if (target->IsDead()) {
			return;
		}

		//ditto
		if (!hitRigidBody->name.data()) {
			return;
		}

		std::string hitRigidBodyName = hitRigidBody->name.data();

		bool isCriticalHit = a_vanillaHitData.flags.any(RE::HitData::Flag::kCritical);
		bool isPowerAttack = a_vanillaHitData.flags.any(RE::HitData::Flag::kPowerAttack);

		RE::Actor* targetActor = target->As<RE::Actor>();
		RE::Actor* attacker = a_precisionHitData.attacker;

		RE::TESObjectWEAP* weapon = a_vanillaHitData.weapon;

		RE::MagicCaster* magicCaster = attacker->GetMagicCaster(magicSource);

		for (auto hitEffect : g_hitEffectVector) {
			[&] {
				if (!isPowerAttack && hitEffect.spellOnlyPowerAttacks)
					return;
				if (!isCriticalHit && hitEffect.spellOnlyCriticalHits)
					return;
				if (!hitRigidBodyName.empty() && find(hitEffect.nodeNames.begin(), hitEffect.nodeNames.end(), hitRigidBodyName) == hitEffect.nodeNames.end())
					return;
				if (weapon && !hitEffect.weaponKeywords.empty() && !weapon->HasKeywordInArray(hitEffect.weaponKeywords, false))
					return;

				if (hitEffect.spellForm)
					magicCaster->CastSpellImmediate(hitEffect.spellForm, false, targetActor, 1.0f, false, 0, NULL);
			}();
		}    
	}

	void Initialize() {
		auto precisionInterface = reinterpret_cast<PRECISION_API::IVPrecision3*>(PRECISION_API::RequestPluginAPI(PRECISION_API::InterfaceVersion::V3));
		
		auto preHitResult = precisionInterface->AddPreHitCallback(SKSE::GetPluginHandle(), OnPrecisionPreHit);
		if (preHitResult == PRECISION_API::APIResult::OK || preHitResult == PRECISION_API::APIResult::AlreadyRegistered) {
			INFO("precision pre hit callback registered");
		}

		auto postHitResult = precisionInterface->AddPostHitCallback(SKSE::GetPluginHandle(), OnPrecisionPostHit);
		if (postHitResult == PRECISION_API::APIResult::OK || postHitResult == PRECISION_API::APIResult::AlreadyRegistered) {
			INFO("precision post hit callback registered");
		}
	}
}