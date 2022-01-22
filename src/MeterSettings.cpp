#include "DKUtil/Config.hpp"
#include "MeterSettings.h"

namespace MaxsuDetectionMeter
{
	static auto MainConfig = RUNTIME_PROXY("MaxsuDetectionMeter.ini"sv);

	MeterSettings::MeterSettings()
	{
		MainConfig.Bind(enableDebugLog, false);
		MainConfig.Bind(fadeSpeed, 255.f);
		MainConfig.Bind(flashSpeed, 550.f);
		MainConfig.Bind(flashScale, 0.25f);
		MainConfig.Bind(minFillingSpeed, 0.35f);
		MainConfig.Bind(maxFillingSpeed, 0.95f);
		MainConfig.Bind(minTriggerLevel, 25);
		MainConfig.Bind(radiusX, 310.f);
		MainConfig.Bind(radiusY, 310.f);
		MainConfig.Bind(centerOffsetX, 0);
		MainConfig.Bind(centerOffsetY, -40);

		MainConfig.Load();

		PrintSettingValue(fadeSpeed.get_key(),fadeSpeed.get_data());
		PrintSettingValue(flashSpeed.get_key(), flashSpeed.get_data());
		PrintSettingValue(flashScale.get_key(), flashScale.get_data());
		PrintSettingValue(minFillingSpeed.get_key(), minFillingSpeed.get_data());
		PrintSettingValue(maxFillingSpeed.get_key(), maxFillingSpeed.get_data());
		PrintSettingValue(minTriggerLevel.get_key(), minTriggerLevel.get_data());
		PrintSettingValue(radiusX.get_key(), radiusX.get_data());
		PrintSettingValue(radiusY.get_key(), radiusY.get_data());
		PrintSettingValue(centerOffsetX.get_key(), centerOffsetX.get_data());
		PrintSettingValue(centerOffsetY.get_key(), centerOffsetY.get_data());
	}

}