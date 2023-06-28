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
		MainConfig.Bind<0.f, 1.f>(minFillingSpeed, 0.35f);
		MainConfig.Bind<0.f, 1.f>(maxFillingSpeed, 0.95f);
		MainConfig.Bind<0, 100>(minTriggerLevel, 25);
		MainConfig.Bind(radiusX, 310.f);
		MainConfig.Bind(radiusY, 310.f);
		MainConfig.Bind(centerOffsetX, 0);
		MainConfig.Bind(centerOffsetY, -40);

		MainConfig.Load();

		PrintSettingValue(fadeSpeed);
		PrintSettingValue(flashSpeed);
		PrintSettingValue(flashScale);
		PrintSettingValue(flashShift);
		PrintSettingValue(minFillingSpeed);
		PrintSettingValue(maxFillingSpeed);
		PrintSettingValue(minTriggerLevel);
		PrintSettingValue(radiusX);
		PrintSettingValue(radiusY);
		PrintSettingValue(centerOffsetX);
		PrintSettingValue(centerOffsetY);

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
