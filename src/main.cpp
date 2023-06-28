#include "Hook.h"
#include "Renderer.h"

DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {
		Sleep(100);
	}
#endif

	DKUtil::Logger::Init(Plugin::NAME, REL::Module::get().version().string());

	SKSE::Init(a_skse);

	INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);

	// do stuff
	MaxsuDetectionMeter::Renderer::Install();
	MaxsuDetectionMeter::CharacterEx::InstallHook();

	return true;
}
