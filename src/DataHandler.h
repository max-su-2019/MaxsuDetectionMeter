#pragma once
#include "MeterObj.h"

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

		static std::int32_t ReCalculateDetectionLevel(std::int32_t a_level);
		static std::optional<float> GetStealthPoint(RE::Actor* a_owner);
		
		std::map<RE::FormID, std::atomic<std::shared_ptr<MeterObj>>> meterArr;

		const float		fadeSpeed = 255.f;

		const float		flashSpeed = 550.f;
		const float		flashScale = 0.25f;

		const float		minFillingSpeed = 0.25f;
		const float		maxFillingSpeed = 0.95f;

		const std::int32_t minTriggerLevel = 25;

		const float radiusX = 350.f;
		const float radiusY = 350.f;

	private:
		MeterHandler() = default;

		~MeterHandler() = default;

		MeterHandler(const MeterHandler&) = delete;

		MeterHandler(MeterHandler&&) = delete;

		MeterHandler& operator= (const MeterHandler&) = delete;

		MeterHandler& operator= (MeterHandler&&) = delete;
	};

}