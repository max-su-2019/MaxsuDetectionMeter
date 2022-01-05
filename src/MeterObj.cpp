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

	bool FrameMeterInfo::Update(RE::Actor* a_owner, std::int32_t a_level, std::optional<float> a_stealthPoints)
	{
		auto meterHandler = MeterHandler::GetSingleton();

		if (a_owner) {
			if (this->alpha.GetFadeAction() == FadeType::KFadeOut && this->alpha.GetCurrentValue() == 0)	//Check if meter is compelery fade out.
				return false;

			//Update AlphaInfo
			if (a_stealthPoints.has_value()) 
				this->alpha.SetFadeAction(FadeType::KFadeIn);	//Frame Meter always shown when in combat state
			else
				a_level >= meterHandler->minTriggerLevel ? this->alpha.SetFadeAction(FadeType::KFadeIn) : this->alpha.SetFadeAction(FadeType::KFadeOut);	

			return true;
		}

		return false;
	}


	bool NormalMeterInfo::Update(RE::Actor* a_owner, std::int32_t a_level, std::optional<float> a_stealthPoints)
	{
		auto meterHandler = MeterHandler::GetSingleton();

		if (!a_owner)
			return false;

		//Update AlphaInfo
		if (a_stealthPoints.has_value()) {
			//Always hide non-combat meter when in combat state.
			this->alpha.SetFadeAction(FadeType::KFadeOut);	
			this->alpha.SetValue(0);
		}
		else
			a_level >= meterHandler->minTriggerLevel ? this->alpha.SetFadeAction(FadeType::KFadeIn) : this->alpha.SetFadeAction(FadeType::KFadeOut); 
		
		//Update Flashing
		if (a_level >= 100 && !a_stealthPoints.has_value())
			this->flashing.SetFlashingStart(true);
		else
			this->flashing.SetFlashingStart(false);
	
		//Update Filling
		if (!a_stealthPoints.has_value()) {
			this->filling.SetTargetFilling(a_level / 100.f);
		}
		else
			this->filling.SetTargetFilling(0.f);

		return true;
	}


	bool CombatMeterInfo::Update(RE::Actor* a_owner, std::int32_t, std::optional<float> a_stealthPoints)
	{
		if (!a_owner)
			return false;

		//Update AlphaInfo
		a_stealthPoints.has_value() && a_stealthPoints.value() > 0.f ? this->alpha.SetFadeAction(FadeType::KFadeIn), this->alpha.SetValue(max(155, this->alpha.GetCurrentValue())): this->alpha.SetFadeAction(FadeType::KFadeOut);

		//Update Flashing
		if(a_stealthPoints.has_value() && a_stealthPoints.value() >= 99.9f)
			this->flashing.SetFlashingStart(true);
		else
			this->flashing.SetFlashingStart(false);

		//Update Filling
		a_stealthPoints.has_value() ? this->filling.SetTargetFilling(a_stealthPoints.value() / 100.f) : this->filling.SetTargetFilling(0.f);

		return true;
	}

	bool MeterObj::Update(RE::Actor* a_owner)
	{
		if (!a_owner || !a_owner->currentProcess || !a_owner->currentProcess->high || !a_owner->currentProcess->InHighProcess())
			return false;

		auto playerRef = RE::PlayerCharacter::GetSingleton();
		if (!playerRef)
			return false;

		auto camera = RE::PlayerCamera::GetSingleton();
		auto cameraRoot = camera ? camera->cameraRoot : nullptr;
		if (!cameraRoot)
			return false;

		auto CamTrans = RE::NiTransform(cameraRoot->world.rotate, playerRef->GetPosition());
		headingAngle = CamTrans.GetHeadingAngle(a_owner->GetPosition());

		auto const level = MeterHandler::ReCalculateDetectionLevel(a_owner->RequestDetectionLevel(playerRef));

		auto stealthPoints = MeterHandler::GetStealthPoint(a_owner);
		if (!stealthPoints.has_value() && playerRef->IsInCombat())	//Remove non-combat meters when player in combat state.
			return false;

		for (std::uint32_t type = MeterType::kFrame; type < MeterType::kTotal; type++) {
			if (!infos[type] || !infos[type]->Update(a_owner,level,stealthPoints))
				return false;
		}

		return true;
	}
}