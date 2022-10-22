#include "MeterSettings.h"
#include "DKUtil/Config.hpp"
#include "Renderer.h"

namespace MaxsuDetectionMeter
{
	static auto MainConfig = COMPILE_PROXY("MaxsuDetectionMeter.ini"sv);

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

		PrintSettingValue(fadeSpeed.get_key(), *fadeSpeed);
		PrintSettingValue(flashSpeed.get_key(), *flashSpeed);
		PrintSettingValue(flashScale.get_key(), *flashScale);
		PrintSettingValue(flashShift.get_key(), *flashShift);
		PrintSettingValue(minFillingSpeed.get_key(), *minFillingSpeed);
		PrintSettingValue(maxFillingSpeed.get_key(), *maxFillingSpeed);
		PrintSettingValue(minTriggerLevel.get_key(), *minTriggerLevel);
		PrintSettingValue(radiusX.get_key(), *radiusX);
		PrintSettingValue(radiusY.get_key(), *radiusY);
		PrintSettingValue(centerOffsetX.get_key(), *centerOffsetX);
		PrintSettingValue(centerOffsetY.get_key(), *centerOffsetY);

		ScalingForResolution();
	}

	void MeterSettings::ScalingForResolution()
	{
		float widthScale = Renderer::GetResolutionScaleWidth();
		*radiusX *= widthScale;
		*centerOffsetX *= widthScale;

		float heightScale = Renderer::GetResolutionScaleHeight();
		*radiusY *= heightScale;
		*centerOffsetY *= heightScale;
		*flashShift *= heightScale;
	}

	void MeterSettings::Reload()
	{
		MainConfig.Load();
		ScalingForResolution();
		DEBUG("Reload Configuration Settings!");
	}
}
