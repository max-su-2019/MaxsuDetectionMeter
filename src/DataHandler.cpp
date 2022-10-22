#include "DataHandler.h"
#include "ConsoleCommands.h"

namespace MaxsuDetectionMeter
{
	MeterHandler::MeterHandler()
	{
		meterSettings = std::make_unique<MeterSettings>();
		ConsoleCommands::Register();
	}


	bool MeterHandler::ShouldDisplayMeter(RE::Actor* a_owner)
	{
		auto& data = a_owner->GetActorRuntimeData();
		auto s = a_owner && !a_owner->IsPlayerRef() && !a_owner->IsDead() && a_owner->Is3DLoaded() && !a_owner->GetActorRuntimeData().boolFlags.all(RE::Actor::BOOL_FLAGS::kDoNotShowOnStealthMeter) &&
		         !a_owner->IsPlayerTeammate() && !a_owner->IsSummonedByPlayer() && data.currentProcess && data.currentProcess->high;
		return s;
	}

	std::int32_t MeterHandler::ReCalculateDetectionLevel(std::int32_t a_level)
	{
		if (a_level < 0) {
			a_level += 100;
			return a_level = min(max(a_level, 0), 100);
		}
		else
			return 100;
	}


	std::optional<float> MeterHandler::GetStealthPoint(RE::Actor* a_owner)
	{
		std::optional<float> result;

		if (!a_owner || a_owner->IsPlayerRef())
			return result;

		auto IsInCombatWithPlayer = [](const RE::CombatTarget& targetData) -> bool {
			if (targetData.targetHandle.get())
				return targetData.targetHandle.get()->IsPlayerRef();
			else if (targetData.attackedMember.get() && targetData.attackedMember.get()->GetActorRuntimeData().currentCombatTarget.get())
				return targetData.attackedMember.get()->GetActorRuntimeData().currentCombatTarget.get()->IsPlayerRef();
			else
				return false;
		};

		auto group = a_owner->GetCombatGroup();
		if (group) {
			for (auto target : group->targets) {
				if (IsInCombatWithPlayer(target)) {
					return result.emplace(std::clamp(100.f - target.stealthPoints, 0.f, 100.f));
				}
			}
		}

		return result;
	}


	bool MeterHandler::HeadTarckingOnPlayer(RE::Actor* a_owner)
	{
		if (!a_owner)
			return false;

		auto extraTarget = a_owner->extraList.GetByType<RE::ExtraHeadingTarget>();
		if (extraTarget && extraTarget->target.get())
			return extraTarget->target.get()->IsPlayerRef();
		
		auto& data = a_owner->GetActorRuntimeData();
		if (data.currentProcess && data.currentProcess->high && data.currentProcess->high->pathLookAtTarget.get())
			return data.currentProcess->high->pathLookAtTarget.get()->IsPlayerRef();

		return false;
	}


	bool MeterHandler::DisplayForNonCombat(RE::Actor* a_owner, const std::int32_t a_level, RE::PlayerCharacter* playerRef) const
	{
		if (a_level >= 100)
			return true;

		bool discard;
		return a_level >= meterSettings->minTriggerLevel.get_data() && a_owner->AsActorState()->GetSitSleepState() != RE::SIT_SLEEP_STATE::kIsSitting &&
			(a_owner->HasLineOfSight(playerRef, discard) || HeadTarckingOnPlayer(a_owner));
	}
}