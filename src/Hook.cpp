#include "Hook.h"
#include "DataHandler.h"

namespace MaxsuDetectionMeter
{
	std::int32_t  DetectionLevelHook::RequestDetectionLevel(RE::Actor* a_owner, RE::Actor* a_target, RE::DETECTION_PRIORITY a_priority)
	{
		//logger::debug("RequestDetectionLevel Trigger!");

		auto ReCalculateDetectionLevel = [](std::int32_t detectionLevel) -> int32_t {
			if (detectionLevel < 0) {
				detectionLevel += 100;
				return detectionLevel = min(max(detectionLevel, 0), 100);
			}
			else
				return 100;
		};

		auto level = ReCalculateDetectionLevel(_RequestDetectionLevel(a_owner, a_target, a_priority));

		auto camera = RE::PlayerCamera::GetSingleton();
		auto cameraRoot = camera ? camera->cameraRoot : nullptr;

		if (a_owner && a_target && a_target->IsPlayerRef() && cameraRoot) {
			const auto ownerID = a_owner->formID;
			auto meterHandler = MeterHandler::GetSingleton();

			auto CamTrans = RE::NiTransform(cameraRoot->world.rotate, a_target->GetPosition());
			auto const angle = CamTrans.GetHeadingAngle(a_owner->GetPosition());
				
			if (!meterHandler->meterArr.count(ownerID) && level >= meterHandler->minTriggerLevel && a_owner->HasLOS(a_target)) {
				auto meterObj = std::make_shared<MeterObj>(MeterObj(angle));
				meterHandler->meterArr.emplace(MeterPair(ownerID, meterObj));
				logger::debug("Add a Meter ID : {:x}", ownerID);
			}

			auto it = meterHandler->meterArr.find(ownerID);
			if (it != meterHandler->meterArr.end()) {
				logger::debug("Find a Meter ID: {:x}", it->first);
				if (it->second && it->second->Update(a_owner, level, angle))
					logger::debug("Update Meter Successfully!");
				else {
					meterHandler->meterArr.erase(it);
					logger::debug("Update Meter Fail!");
				}
			}
		}
			
		return _RequestDetectionLevel(a_owner, a_target, a_priority);
	}

}