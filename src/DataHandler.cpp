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


	bool MeterHandler::GetStealthPoint(RE::Actor* a_owner, RE::Actor* a_target, float& a_result)
	{
		if (!a_owner || a_owner->IsPlayerRef())
			return false;

		auto group = a_owner->GetCombatGroup();
		if (group) {
			for (auto target : group->targets) {
				if (target.targetHandle.get() && target.targetHandle.get()->IsPlayerRef()) {
					a_result = std::clamp(100.f - target.attackedStealthPoints, 0.f, 100.f);
					return true;
				}
			}
		}

		return false;
	}


}