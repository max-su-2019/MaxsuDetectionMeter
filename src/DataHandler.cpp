#include"DataHandler.h"
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
		return a_owner && !a_owner->IsPlayerRef() && !a_owner->IsDead() && a_owner->Is3DLoaded() && !a_owner->NotShowOnStealthMeter() && 
			!a_owner->IsPlayerTeammate() && !a_owner->IsSummonedByPlayer() && a_owner->currentProcess && a_owner->currentProcess->high;
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

		auto IsInCombatWithPlayer = [](const RE::CombatGroup::TargetData& targetData) -> bool {
			if (targetData.targetHandle.get())
				return targetData.targetHandle.get()->IsPlayerRef();
			else if (targetData.attackedMember.get() && targetData.attackedMember.get()->currentCombatTarget.get())
				return targetData.attackedMember.get()->currentCombatTarget.get()->IsPlayerRef();
			else
				return false;
		};

		auto group = a_owner->GetCombatGroup();
		if (group) {
			for (auto target : group->targets) {
				if (IsInCombatWithPlayer(target)) {
					return result.emplace(std::clamp(100.f - target.attackedStealthPoints, 0.f, 100.f));
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
		
		if (a_owner->currentProcess && a_owner->currentProcess->high && a_owner->currentProcess->high->pathLookAtTarget.get())
			return a_owner->currentProcess->high->pathLookAtTarget.get()->IsPlayerRef();

		return false;
	}


	bool MeterHandler::DisplayForNonCombat(RE::Actor* a_owner, const std::int32_t a_level, RE::PlayerCharacter* playerRef) const
	{
		if (a_level >= 100)
			return true;

		return a_level >= meterSettings->minTriggerLevel.get_data() && a_owner->GetSitSleepState() != RE::SIT_SLEEP_STATE::kIsSleeping &&
			(a_owner->HasLOS(playerRef) || HeadTarckingOnPlayer(a_owner));
	}
}