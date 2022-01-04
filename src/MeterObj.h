#pragma once

namespace MaxsuDetectionMeter
{
	struct MeterTypes
	{
		enum MeterType : std::int32_t
		{
			kFrame,
			kNormal,
			kCombat,
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
				KFadeOut
			};

			AlphaInfo() = default;

			_NODISCARD FadeType GetFadeAction() const { return fadeAction; }
			void SetFadeAction(const FadeType a_action) { fadeAction = a_action; }

			_NODISCARD std::int32_t GetCurrentValue() const { return value; }
			void SetValue(const std::int32_t a_value) { value = a_value; }

		protected:
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


		struct FlashingInfo : public AlphaInfo
		{
			FlashingInfo() = default;

			_NODISCARD bool IsFlashingStart() const { return flashingStart; }
			void SetFlashingStart(const bool a_start) { flashingStart = a_start; }

		private:
			FadeType fadeAction = FadeType::KFadeOut;
			std::int32_t value = 255;
			bool flashingStart = false;
		};

	public:
		using FadeType = AlphaInfo::FadeType;

		MeterInfo() = default;

		virtual bool Update(RE::Actor* a_owner, std::int32_t a_level, std::optional<float> a_stealthPoints) = 0;

		AlphaInfo		alpha;
		FillingInfo		filling;
		FlashingInfo	flashing;

	};


	class FrameMeterInfo : public MeterInfo
	{
	public:
		FrameMeterInfo();

		virtual bool Update(RE::Actor* a_owner, std::int32_t a_level, std::optional<float> a_stealthPoints) override;
	};


	class NormalMeterInfo : public MeterInfo
	{
	public:
		NormalMeterInfo() = default;

		virtual bool Update(RE::Actor* a_owner, std::int32_t a_level, std::optional<float> a_stealthPoints) override;
	};


	class CombatMeterInfo : public MeterInfo
	{
	public:
		CombatMeterInfo() = default;

		virtual bool Update(RE::Actor* a_owner, std::int32_t a_level, std::optional<float> a_stealthPoints) override;
	};



	class MeterObj
	{
	public:

		MeterObj(float a_angle) : headingAngle(a_angle)
		{
			infos[MeterType::kFrame] = std::make_shared<FrameMeterInfo>();
			infos[MeterType::kNormal] = std::make_shared<NormalMeterInfo>();
			infos[MeterType::kCombat] = std::make_shared<CombatMeterInfo>();
		}

		bool Update(RE::Actor* a_owner);
		void MarkForRemove() { shouldRemove = true; }
		bool ShouldRemove() const { return shouldRemove; }

		std::shared_ptr<MeterInfo>	infos[MeterType::kTotal];
		float						headingAngle;

	private:
		bool shouldRemove = false;
	};
}