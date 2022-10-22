#pragma once
#include "MeterObj.h"
#include "MeterSettings.h"

namespace MaxsuDetectionMeter
{
	using MeterPair = std::pair<const RE::FormID, std::atomic<std::shared_ptr<MeterObj>>>;

	class MeterHandler :
		public DKUtil::model::Singleton<MeterHandler>
	{
	public:
		static bool ShouldDisplayMeter(RE::Actor* a_owner);
		static std::int32_t ReCalculateDetectionLevel(std::int32_t a_level);
		static std::optional<float> GetStealthPoint(RE::Actor* a_owner);
		static bool HeadTarckingOnPlayer(RE::Actor* a_owner);

		bool DisplayForNonCombat(RE::Actor* a_owner, const std::int32_t a_level, RE::PlayerCharacter* playerRef) const;

		std::map<RE::FormID, std::atomic<std::shared_ptr<MeterObj>>> meterArr;
		std::unique_ptr<MeterSettings> meterSettings;
		mutable std::mutex m_mutex;

		MeterHandler();
	};

}
