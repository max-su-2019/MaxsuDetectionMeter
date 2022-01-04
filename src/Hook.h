namespace MaxsuDetectionMeter
{
	class DetectionLevelHook
	{
	public:
		static void InstallHook()
		{
			SKSE::AllocTrampoline(1 << 4);
			auto& trampoline = SKSE::GetTrampoline();

			static std::uint32_t baseID = 38606, offset = 0x414;
			REL::Relocation<std::uintptr_t> GetDetectedBase{ REL::ID(baseID) };
			_RequestDetectionLevel = trampoline.write_call<5>(GetDetectedBase.address() + offset, RequestDetectionLevel);

			logger::debug("Hook RequestDetectionLevel!, Base:{}, Offset:{:x}", baseID, offset);
		}

	private:
		static std::int32_t RequestDetectionLevel(RE::Actor* a_owner, RE::Actor* a_target, RE::DETECTION_PRIORITY a_priority);

		static inline REL::Relocation<decltype(RequestDetectionLevel)> _RequestDetectionLevel;
	};


	class ActorUpdateHook
	{
	public:
		static void InstallHook()
		{
			static std::uint32_t baseID = 261397, offset = 0xAD;

			REL::Relocation<std::uintptr_t> CharacterVtbl{ REL::ID(baseID) };
			_ActorUpdate = CharacterVtbl.write_vfunc(offset, ActorUpdate);
			logger::info("Hook Actor Update!");
		}

	private:
		static void ActorUpdate(RE::Actor* a_actor, float a_delta);

		static inline REL::Relocation<decltype(ActorUpdate)> _ActorUpdate;
	};
}