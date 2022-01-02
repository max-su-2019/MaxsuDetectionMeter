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
		
		std::map<RE::FormID, std::atomic<std::shared_ptr<MeterObj>>> meterArr;

		const float		fadeSpeed = 155.f;

		const float		flashSpeed = 500.f;
		const float		flashScale = 0.25f;

		const float		minFillingSpeed = 0.25f;
		const float		maxFillingSpeed = 0.95f;

		const std::int32_t minTriggerLevel = 20;

		const float radiusX = 500.f;
		const float radiusY = 300.f;

		//mutable RE::BSSpinLock meterLock;

	private:
		MeterHandler() = default;

		~MeterHandler() = default;

		MeterHandler(const MeterHandler&) = delete;

		MeterHandler(MeterHandler&&) = delete;

		MeterHandler& operator= (const MeterHandler&) = delete;

		MeterHandler& operator= (MeterHandler&&) = delete;
	};

}