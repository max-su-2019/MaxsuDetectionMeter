#include "MeterObj.h"
#include "DataHandler.h"

namespace MaxsuDetectionMeter
{
	/*
	bool MeterInfo::Update(RE::Actor* a_owner, std::int32_t a_level)
	{
		if (a_owner) {
			switch (METERTYPE)
			{
			case MeterType::kFrame: {
				FrameMeterInfo* meter = dynamic_cast<FrameMeterInfo*>(this);
				return meter ? meter->Update(a_owner, a_level) : false;
			}

			case MeterType::kNormal: {
				NormalMeterInfo* meter = dynamic_cast<NormalMeterInfo*>(this);
				return meter ? meter->Update(a_owner, a_level) : false;
			}

			default:
				return false;
			}
		}

		return false;
	}
	*/

	FrameMeterInfo::FrameMeterInfo()
	{
		this->filling.SetTargetFilling(1.0f);
		this->filling.SetCurrentFilling(1.0f);
		// Set Filling to 100 when init.
	}

	bool FrameMeterInfo::Update(RE::Actor* a_owner, std::int32_t a_level)
	{
		auto meterHandler = MeterHandler::GetSingleton();

		if (a_owner) {
			a_level >= meterHandler->minTriggerLevel ? this->alpha.SetFadeAction(FadeType::KFadeIn) : this->alpha.SetFadeAction(FadeType::KFadeOut);	//Update AlphaInfo
			return true;
		}

		return false;
	}


	bool NormalMeterInfo::Update(RE::Actor* a_owner, std::int32_t a_level)
	{
		auto meterHandler = MeterHandler::GetSingleton();

		if (!a_owner)
			return false;

		if (a_level < 100) {
			a_level >= meterHandler->minTriggerLevel ? this->alpha.SetFadeAction(FadeType::KFadeIn) : this->alpha.SetFadeAction(FadeType::KFadeOut); //Update Alpha Fade
		}
		else {
			this->alpha.SetFadeAction(FadeType::KFlashing);	//Flashing when 100
		}

		this->filling.SetTargetFilling(a_level / 100.f); //Update Filling

		return true;
	}
}