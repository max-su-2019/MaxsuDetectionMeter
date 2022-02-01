
namespace MaxsuDetectionMeter
{
	class ActorUpdateHook
	{
	public:
		static void InstallHook()
		{
		#if ANNIVERSARY_EDITION

		#else	
			static std::uint32_t baseID = 261397, offset = 0xAD;	//Special Edition
		#endif
			REL::Relocation<std::uintptr_t> CharacterVtbl{ REL::ID(baseID) };
			_ActorUpdate = CharacterVtbl.write_vfunc(offset, ActorUpdate);
			logger::info("Hook Actor Update!");
		}

	private:
		static void ActorUpdate(RE::Actor* a_actor, float a_delta);

		static inline REL::Relocation<decltype(ActorUpdate)> _ActorUpdate;
	};

}