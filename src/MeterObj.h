#pragma once

namespace MaxsuDetectionMeter
{
	struct MeterTypes
	{
		enum MeterType : std::int32_t
		{
			kFrame,
			kNormal,
			//kCombat,
			kTotal
		};
	};
	using MeterType = MeterTypes::MeterType;


	class MeterInfo 
	{
		

		struct AlphaInfo
		{
			enum class FadeType : std::uint32_t
			{
				KNone = 0,
				KFadeIn,
				KFadeOut,
				KFlashing
			};

			AlphaInfo() = default;

			_NODISCARD FadeType GetFadeAction() const { return fadeAction; }
			void SetFadeAction(const FadeType a_action) { fadeAction = a_action; }

			_NODISCARD std::int32_t GetCurrentValue() const { return value; }
			void SetValue(const std::int32_t a_value) { value = a_value; }

		private:
			FadeType fadeAction = FadeType::KNone;
			std::int32_t value = 0;
		};

		struct FillingInfo
		{
			FillingInfo() = default;

			_NODISCARD float GetTargetFilling() const { return targFilling; }
			void SetTargetFilling(const float a_targFliing) { targFilling = a_targFliing; }

			_NODISCARD float GetCurrentFilling() const { return curFilling; }
			void SetCurrentFilling(const float a_curFilling) { curFilling = a_curFilling; }

		private:
			float targFilling = 0.f;
			float curFilling = 0.f;
		};

	public:
		using FadeType = AlphaInfo::FadeType;

		MeterInfo() = default;

		virtual bool Update(RE::Actor* a_owner, std::int32_t a_level) = 0;

		AlphaInfo		alpha;
		FillingInfo		filling;

	};


	class FrameMeterInfo : public MeterInfo
	{
	public:
		FrameMeterInfo();

		virtual bool Update(RE::Actor* a_owner, std::int32_t a_level) override;
	};


	class NormalMeterInfo : public MeterInfo
	{
	public:
		NormalMeterInfo() = default;

		virtual bool Update(RE::Actor* a_owner, std::int32_t a_level) override;
	};


	class MeterObj
	{
	public:

		MeterObj(float a_angle) : headingAngle(a_angle)
		{
			infos[MeterType::kFrame] = std::make_shared<FrameMeterInfo>();
			infos[MeterType::kNormal] = std::make_shared<NormalMeterInfo>();
		}

		bool Update(RE::Actor* a_owner, std::int32_t a_level, float a_angle)
		{
			if (!a_owner)
				return false;

			headingAngle = a_angle;
			
			for (std::uint32_t type = MeterType::kFrame; type < MeterType::kTotal; type++) {
				if (!infos[type])
					return false;

				switch (type){
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
				}
			}

			return true;
		}

		std::shared_ptr<MeterInfo>	infos[MeterType::kTotal];
		float						headingAngle;
	};
}