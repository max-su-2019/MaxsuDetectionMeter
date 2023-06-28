#pragma once

// clib
#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

// winnt
#include <ShlObj_core.h>

using namespace std::literals;
using namespace REL::literals;

#define DLLEXPORT extern "C" [[maybe_unused]] __declspec(dllexport)

// Plugin
#include "Plugin.h"

// DKUtil
#include "DKUtil/Logger.hpp"

namespace stl
{
	template <class T>
	void write_thunk_call()
	{
		auto& trampoline = SKSE::GetTrampoline();
		REL::Relocation<std::uintptr_t> hook{ T::id, T::offset };
		T::func = trampoline.write_call<5>(hook.address(), T::thunk);
	}
}  // namespace stl