#pragma once
#include "MeterObj.h"
#include "MeterSettings.h"

namespace MaxsuDetectionMeter
{
	using MeterPair = std::pair <const RE::FormID, std::atomic<std::shared_ptr<MeterObj>>>;

	class MeterHandler
	{
	public:
		static MeterHandler* GetSingleton()
		{
			static MeterHandler singleton;
			return std::addressof(singleton);
		}

		static bool ShouldDisplayMeter(RE::Actor* a_owner);
		static std::int32_t ReCalculateDetectionLevel(std::int32_t a_level);
		static std::optional<float> GetStealthPoint(RE::Actor* a_owner);
		static bool HeadTarckingOnPlayer(RE::Actor* a_owner);
		
		std::map<RE::FormID, std::atomic<std::shared_ptr<MeterObj>>> meterArr;

		std::unique_ptr<MeterSettings> meterSettings;

		mutable std::mutex m_mutex;

	private:
		MeterHandler();

		~MeterHandler() = default;

		MeterHandler(const MeterHandler&) = delete;

		MeterHandler(MeterHandler&&) = delete;

		MeterHandler& operator= (const MeterHandler&) = delete;

		MeterHandler& operator= (MeterHandler&&) = delete;
	};

}