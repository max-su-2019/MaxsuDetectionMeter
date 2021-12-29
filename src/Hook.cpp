#include "Hook.h"

namespace MaxsuDetectionMeter
{
	std::int32_t  DetectionLevelHook::RequestDetectionLevel(RE::Actor* a_owner, RE::Actor* a_target, RE::DETECTION_PRIORITY a_priority)
	{
		logger::debug("RequestDetectionLevel Trigger!");

		auto level = _RequestDetectionLevel(a_owner, a_target, a_priority);

		if (a_owner && a_target)
			logger::debug("Owner ID is:{:x}, Target ID is:{:x}, Priority is {}, Level is {}", a_owner->formID, a_target->formID, a_priority, level);

		return level;
	}

}