#pragma once
#include "DKUtil/Config.hpp"

namespace MaxsuDetectionMeter
{
	using namespace DKUtil::Alias;

	struct MeterSettings
	{
	private:
		template<class T>
		static void PrintSettingValue(const std::string a_key, const T a_data)
		{
			logger::info("Setting:\"{}\" is {}"sv, a_key, a_data);
		}

	public:
		MeterSettings();

		void Reload() const;

		Boolean enableDebugLog{ "EnableDebugLog", "Main" };

		Double fadeSpeed{ "FadeSpeed", "AnimatedEffect" };

		Double flashSpeed = { "FlashSpeed", "AnimatedEffect" };
		Double flashScale = { "FlashScale", "AnimatedEffect" };

		Double minFillingSpeed = { "MinFillingSpeed", "AnimatedEffect" };
		Double maxFillingSpeed = { "MaxFillingSpeed", "AnimatedEffect" };

		Integer minTriggerLevel = { "MinTriggerLevel", "Display" };

		Double radiusX = { "RadiusX", "Display" };
		Double radiusY = { "RadiusY", "Display" };

		Integer centerOffsetX = { "CenterOffsetX", "Display" };
		Integer centerOffsetY = { "CenterOffsetY", "Display"};
	};
}