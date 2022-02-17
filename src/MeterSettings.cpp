#include "MeterSettings.h"
#include "DKUtil/Config.hpp"
#include "Renderer.h"

namespace MaxsuDetectionMeter
{
	static auto MainConfig = RUNTIME_PROXY("MaxsuDetectionMeter.ini"sv);

	MeterSettings::MeterSettings()
	{
		MainConfig.Bind(enableDebugLog, false);
		MainConfig.Bind(fadeSpeed, 255.f);
		MainConfig.Bind(flashSpeed, 550.f);
		MainConfig.Bind(flashScale, 0.25f);
		MainConfig.Bind(flashShift, 36);
		MainConfig.Bind(minFillingSpeed, 0.35f);
		MainConfig.Bind(maxFillingSpeed, 0.95f);
		MainConfig.Bind(minTriggerLevel, 25);
		MainConfig.Bind(radiusX, 310.f);
		MainConfig.Bind(radiusY, 310.f);
		MainConfig.Bind(centerOffsetX, 0);
		MainConfig.Bind(centerOffsetY, -40);

		MainConfig.Load();

		PrintSettingValue(fadeSpeed.get_key(), fadeSpeed.get_data());
		PrintSettingValue(flashSpeed.get_key(), flashSpeed.get_data());
		PrintSettingValue(flashScale.get_key(), flashScale.get_data());
		PrintSettingValue(flashShift.get_key(), flashShift.get_data());
		PrintSettingValue(minFillingSpeed.get_key(), minFillingSpeed.get_data());
		PrintSettingValue(maxFillingSpeed.get_key(), maxFillingSpeed.get_data());
		PrintSettingValue(minTriggerLevel.get_key(), minTriggerLevel.get_data());
		PrintSettingValue(radiusX.get_key(), radiusX.get_data());
		PrintSettingValue(radiusY.get_key(), radiusY.get_data());
		PrintSettingValue(centerOffsetX.get_key(), centerOffsetX.get_data());
		PrintSettingValue(centerOffsetY.get_key(), centerOffsetY.get_data());

		ScalingForResolution();
	}

	void MeterSettings::ScalingForResolution()
	{
		float widthScale = Renderer::GetResolutionScaleWidth();
		radiusX.set_data(std::vector<double>{ radiusX.get_data() * widthScale });
		centerOffsetX.set_data(std::vector<int64_t>{ int64_t(centerOffsetX.get_data() * widthScale) });

		float heightScale = Renderer::GetResolutionScaleHeight();
		radiusY.set_data(std::vector<double>{ radiusY.get_data() * heightScale });
		centerOffsetY.set_data(std::vector<int64_t>{ int64_t(centerOffsetY.get_data() * heightScale) });
		flashShift.set_data(std::vector<int64_t>{ int64_t(flashShift.get_data() * heightScale) });
	}

	void MeterSettings::Reload()
	{
		MainConfig.Load();
		ScalingForResolution();
		logger::debug("Reload Configuration Settings!");
	}
}
