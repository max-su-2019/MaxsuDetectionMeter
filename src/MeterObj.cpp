#include "MeterObj.h"
#include "DataHandler.h"

namespace MaxsuDetectionMeter
{
	FrameMeterInfo::FrameMeterInfo()
	{
		//Set Filling to 100 when init.
		this->filling.SetTargetFilling(1.0f);
		this->filling.SetCurrentFilling(1.0f);
	}

	bool FrameMeterInfo::Update(RE::Actor* a_owner, std::int32_t a_level)
	{
		auto meterHandler = MeterHandler::GetSingleton();

		if (a_owner) {
			if (this->alpha.GetFadeAction() == FadeType::KFadeOut && this->alpha.GetCurrentValue() == 0)	//Check if meter is compelery fade out.
				return false;

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

		//Update Alpha Fade
		a_level >= meterHandler->minTriggerLevel ? this->alpha.SetFadeAction(FadeType::KFadeIn) : this->alpha.SetFadeAction(FadeType::KFadeOut); 
		
		//Update Flashing
		if (a_level >= 100)
			this->flashing.SetFlashingStart(true);
		else
			this->flashing.SetFlashingStart(false);
	
		//Update Filling
		this->filling.SetTargetFilling(a_level / 100.f); 

		return true;
	}


	bool MeterObj::Update(RE::Actor* a_owner, std::int32_t a_level, float a_angle)
	{
		if (!a_owner || !a_owner->currentProcess || !a_owner->currentProcess->high || !a_owner->currentProcess->InHighProcess())
			return false;

		headingAngle = a_angle;

		for (std::uint32_t type = MeterType::kFrame; type < MeterType::kTotal; type++) {
			if (!infos[type])
				return false;

			switch (type) {
			case MeterType::kFrame: {
				FrameMeterInfo* meter = dynamic_cast<FrameMeterInfo*>(infos[type].get());
				if (meter->Update(a_owner, a_level))
					continue;
				else
					return false;
			}

			case MeterType::kNormal: {
				NormalMeterInfo* meter = dynamic_cast<NormalMeterInfo*>(infos[type].get());
				if (meter->Update(a_owner, a_level))
					continue;
				else
					return false;
			}
			default:
				break;
			}
		}

		return true;
	}
}