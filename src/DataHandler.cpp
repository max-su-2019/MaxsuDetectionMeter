#include"DataHandler.h"

namespace MaxsuDetectionMeter
{
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

		auto group = a_owner->GetCombatGroup();
		if (group) {
			for (auto target : group->targets) {
				if (target.targetHandle.get() && target.targetHandle.get()->IsPlayerRef()) {
					return result.emplace(std::clamp(100.f - target.attackedStealthPoints, 0.f, 100.f));
				}
			}
		}

		return result;
	}


}