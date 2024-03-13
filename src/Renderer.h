#pragma once

namespace MaxsuDetectionMeter
{
	class Renderer
	{
		struct WndProcHook
		{
			static LRESULT thunk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			static inline WNDPROC func;
		};

		struct D3DInitHook
		{
			static void thunk();
			static inline REL::Relocation<decltype(&thunk)> func;

			static constexpr auto id = REL::RelocationID(75595, 77226);
			static constexpr auto offset = REL::VariantOffset(0x9, 0x275, 0x00);  // VR unknown

			static inline std::atomic<bool> initialized = false;
		};

		class MenuPresentHook : public RE::HUDMenu
		{
		public:
			static void Install()
			{
				REL::Relocation<std::uintptr_t> vtbl{ VTABLE[0] };
				func = vtbl.write_vfunc(0x6, &Hook_PostDisplay);
			}

		private:
			static void Hook_PostDisplay(RE::IMenu* Menu);
			static inline REL::Relocation<decltype(&PostDisplay)> func;
		};

	private:
		Renderer() = delete;

		static void DrawMeters();  //Rendering Meters.
		static bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, std::int32_t& out_width, std::int32_t& out_height);
		static void MessageCallback(SKSE::MessagingInterface::Message* msg);

		static inline bool ShowMeters = false;
		static inline ID3D11Device* device = nullptr;
		static inline ID3D11DeviceContext* context = nullptr;

	public:
		static bool Install();

		static float GetResolutionScaleWidth();   // { return ImGui::GetIO().DisplaySize.x / 1920.f; }
		static float GetResolutionScaleHeight();  //{ return ImGui::GetIO().DisplaySize.y / 1080.f; }
	};
}
