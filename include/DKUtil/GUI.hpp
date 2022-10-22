#pragma once

/* 
 * 1.2.2
 * Disabled some features until further fixing;
 * 
 * 1.2.1
 * Minor formatting;
 * 
 * 1.2.0
 * Complete IPC feature;
 * Naming convention formatting;
 * 
 * 1.1.0
 * Added IPC feature to prevent plugins from initializing D3D individually;
 * Separate ImGui and D3D implementation;
 *
 * 1.0.0
 * Basic ImGui implementation;
 * Callback hook setup;
 * RAW_D3D definition allowed for direct control without ImGui;
 *
 */


#define DKU_G_VERSION_MAJOR     1
#define DKU_G_VERSION_MINOR     2
#define DKU_G_VERSION_REVISION  2


// AdditionalInclude
#include <algorithm>
#include <bit>
#include <d3d11.h>
#include <type_traits>
#include <queue>
#include <vector>

// imgui
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

// DKUtil
#include "DKUtil/Hook.hpp"
#include "DKUtil/Logger.hpp"

#ifdef DKU_G_DEBUG
#define DKU_DEBUG
#define DEBUG(...)	INFO(__VA_ARGS__)
#endif

#include "DKUtil/Utility.hpp"

#pragma comment ( lib, "d3d11.lib" )


#define VMT_PRESENT_INDEX   0x8
#define WM_DKU_G_ADD        WM_USER + 107
#define WM_DKU_G_REMOVE     WM_USER + 108
#define WM_DKU_G_QUERY_D    WM_USER + 109
#ifndef RAW_D3D
#define WM_DKU_G_QUERY_I    WM_USER + 110
#endif

#define LR_DKU_G_SUCCESS    static_cast<LRESULT>(1)
#define LR_DKU_G_FAILURE    static_cast<LRESULT>(0)
#define LR_DKU_G_DISABLE    static_cast<LRESULT>(-1)


#define DKU_G(GLOBAL)       DKUtil::GUI::detail::global::GLOBAL
#define DKU_G_DEVICE        DKU_G(D3D11.Device)
#define DKU_G_SWAPCHAIN     DKU_G(D3D11.SwapChain)
#define DKU_G_CONTEXT       DKU_G(D3D11.DeviceContext)
#define DKU_G_TARGETVIEW    DKU_G(D3D11.RenderView)
#define DKU_G_TARGETHWND    DKU_G(D3D11.TargetWindow)


namespace DKUtil
{
    constexpr auto DKU_G_VERSION = DKU_G_VERSION_MAJOR * 10000 + DKU_G_VERSION_MINOR * 100 + DKU_G_VERSION_REVISION;
} // namespace DKUtil


namespace DKUtil::IPC
{
#define DKU_IPC_BASE std::wstring(L"_DKU_IPC_BASE_")
#define DKU_IPC_HOST std::wstring(L"_DKU_IPC_HOST_")


	using WndProcFunc = std::add_pointer_t<LRESULT((__stdcall)(HWND, UINT, WPARAM, LPARAM))>;

	struct HostInfo
	{
		enum class HostType
		{
			kInvalid,

			kSelf,
			kRelay
		};

		std::wstring WndClass{ DKU_IPC_BASE };
		std::wstring WndTitle{ DKU_IPC_HOST };
		HostType Type{ HostType::kInvalid };
		HWND Window{ nullptr };
		std::string Name{ PROJECT_NAME };
	};
	using HostType = HostInfo::HostType;

	template <const LRESULT QueryFlag, const LRESULT RelayFlag, typename payload_t>
	inline bool TryInitHost(
		HostInfo& a_host,
		const char* a_desc,
		const WNDPROC a_proc,
		const payload_t a_payload,
		const std::predicate auto a_evalOp) noexcept
	{
		if (!a_proc) {
			return false;
		}

		wchar_t moduleName[MAX_PATH];
		GetModuleBaseNameW(GetCurrentProcess(), nullptr, moduleName, MAX_PATH);

		// unique ipc window per process, per type
		// 2022/01/05: DISABLED for GUI, until further invenstigation
		a_host.WndClass = moduleName + DKU_IPC_BASE + string::to_wstring(a_desc) + string::to_wstring(PROJECT_NAME);
		a_host.WndTitle = moduleName + DKU_IPC_HOST + string::to_wstring(a_desc) + string::to_wstring(PROJECT_NAME);

		a_host.Window = FindWindowExW(HWND_MESSAGE, nullptr, a_host.WndClass.c_str(), a_host.WndTitle.c_str());

		if (a_host.Window && a_host.Type == HostType::kSelf) {
			return true;
		} else if (!a_host.Window) {
			DEBUG("DKU_IPC: {} hosting...", a_desc);

			WNDCLASSEXW windowClass{};
			windowClass.cbSize = sizeof(windowClass);
			windowClass.lpfnWndProc = DefWindowProcW;
			windowClass.lpszClassName = a_host.WndClass.c_str();

			if (!RegisterClassExW(&windowClass)) {
				return false;
			}

			a_host.Window = CreateWindowExW(WS_EX_NOACTIVATE, windowClass.lpszClassName, a_host.WndTitle.c_str(), WS_CAPTION | WS_DISABLED, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
			if (!a_host.Window) {
				UnregisterClassW(windowClass.lpszClassName, GetModuleHandle(nullptr));
				return false;
			}

			SetWindowLongPtrW(a_host.Window, GWLP_WNDPROC, (LONG_PTR)(a_proc));

			DEBUG("DKU_IPC: {} messaging started!", a_desc);

			if (!a_evalOp()) {
				return false;
			}

			DEBUG("DKU_IPC: {} evaluation op succeeded");

			a_host.Name = PROJECT_NAME;
			a_host.Type = HostType::kSelf;

			DEBUG("DKU_IPC: {} host complete", a_desc);

			return true;
		} else {
			DEBUG("DKU_IPC: {} querying...", a_desc);

			const auto result = SendMessageA(a_host.Window, QueryFlag, std::bit_cast<std::uintptr_t>(std::addressof(a_host.Name)), std::bit_cast<std::uintptr_t>(a_payload));

			if (result == RelayFlag) {
				a_host.Type = HostType::kRelay;
				DEBUG("DKU_IPC: {} relay available -> {}", a_desc, a_host.Name);

				return true;
			} else {
				DEBUG("DKU_IPC: {} relay unavailable, fallback", a_desc);

				a_host.WndClass = string::to_wstring(PROJECT_NAME);
				a_host.WndTitle = string::to_wstring(PROJECT_NAME);

				return TryInitHost<QueryFlag, RelayFlag>(a_host, a_desc, a_proc, a_payload, a_evalOp);
			}
		}
	}

	inline bool TerminateHost(HostInfo& a_host) noexcept
	{
		if (!a_host.Window) {
			return false;
		}

		UnregisterClassW(a_host.WndClass.c_str(), GetModuleHandle(nullptr));
		DestroyWindow(a_host.Window);

		DEBUG("DKU_IPC: {} host terminated", a_host.Name);

		return false;
	}

	inline bool TransferHost() noexcept
	{
		return false;
	}
}  // namespace IPC


namespace DKUtil::GUI
{
    extern inline LRESULT AddCallback(Hook::FuncInfo, const bool = false) noexcept;
    extern inline LRESULT RemoveCallback(Hook::FuncInfo) noexcept;

    namespace detail
    {
        using namespace DKUtil::Alias;


        using PresentFunc = std::add_pointer_t<HRESULT(IDXGISwapChain*, UINT, UINT)>;
        using CallbackFunc = std::add_pointer_t<void(IDXGISwapChain*, UINT, UINT)>;
        using CallbackQueue = std::vector<std::uintptr_t>;

        using ContextFunc = std::add_pointer_t<void(ImGuiContext*)>;
        using AllocFunc = std::add_pointer_t<void(ImGuiMemAllocFunc, ImGuiMemFreeFunc, void*)>;
        using ImGuiCallback = std::pair<std::uintptr_t, std::uintptr_t>;
        using ImGuiCallbackQueue = std::queue<ImGuiCallback>;

        namespace global
        {
            // d3d
            struct D3DState
            {
                ID3D11Device*           Device = nullptr;
                IDXGISwapChain*         SwapChain = nullptr;
                ID3D11DeviceContext*    DeviceContext = nullptr;
                ID3D11RenderTargetView* RenderView = nullptr;
                HWND				    TargetWindow = nullptr;

                std::uintptr_t          CtxCallback = reinterpret_cast<std::uintptr_t>(ImGui::SetCurrentContext);
                std::uintptr_t          AllocCallback = reinterpret_cast<std::uintptr_t>(ImGui::SetAllocatorFunctions);
            };
            static D3DState             D3D11;
            static ImGuiContext*        iContext;
            static ImGuiMemAllocFunc    iAllocFunc;
            static ImGuiMemFreeFunc     iFreeFunc;
            static void*                iBackend;

            // gui
            static PresentFunc          _Present;
            static CallbackQueue        HostQueue;
            static ImGuiCallbackQueue   ImGuiQueue;

            // ipc
            static IPC::HostInfo        D3DHost;
            static std::string          LastSender{ PROJECT_NAME };
        } // namespace Global

        
        template <typename invocable_t, typename func_t>
        inline constexpr invocable_t Invocable(func_t a_func) noexcept
        {
            return std::bit_cast<invocable_t>(a_func);
        }


        /* WndProc */
        inline LRESULT __stdcall HostWndProc([[maybe_unused]] HWND a_window, UINT a_msg, WPARAM a_payload1, LPARAM a_payload2) noexcept
        {
            switch (a_msg) {
            case WM_DKU_G_ADD:
            {
                global::LastSender = std::bit_cast<const char*>(a_payload2);
                auto* func = std::bit_cast<Hook::FuncInfo*>(a_payload1);

                if (!func) {
                    return LR_DKU_G_FAILURE;
                }

                auto result = AddCallback(*func);
                global::LastSender = PROJECT_NAME;

                return result;
            }
            case WM_DKU_G_QUERY_D:
            {
                *std::bit_cast<std::string*>(a_payload1) = global::D3DHost.Name;
                auto* state = std::bit_cast<global::D3DState*>(a_payload2);

                if (!state) {
                    return LR_DKU_G_FAILURE;
                }

                global::ImGuiQueue.push(std::make_pair(state->CtxCallback, state->AllocCallback));

                *state = global::D3D11;

                return LR_DKU_G_SUCCESS;
            }
            default:
                return LR_DKU_G_DISABLE;
            }
        }


        /* Present */

        inline HRESULT __cdecl Hook_Present(IDXGISwapChain* a_this, UINT a_syncInterval, UINT a_flags) noexcept
        {
            using namespace global;

            static std::once_flag D3D11Init;
            std::call_once(D3D11Init, [&]()
                {
                    D3D11.SwapChain = a_this;
                    if (SUCCEEDED(D3D11.SwapChain->GetDevice(IID_PPV_ARGS(&D3D11.Device)))) {
                        D3D11.Device->GetImmediateContext(&D3D11.DeviceContext);
                        DXGI_SWAP_CHAIN_DESC sd;
                        D3D11.SwapChain->GetDesc(&sd);
                        D3D11.TargetWindow = sd.OutputWindow;
                        ID3D11Texture2D* backBuffer;
                        D3D11.SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
                        D3D11.Device->CreateRenderTargetView(backBuffer, nullptr, &D3D11.RenderView);
                        backBuffer->Release();

                        DEBUG("DKU_G: Acquired D3D runtime info"sv);

                        ImGui_ImplWin32_Init(D3D11.TargetWindow);
                        ImGui_ImplDX11_Init(D3D11.Device, D3D11.DeviceContext);

                        ImGui::GetAllocatorFunctions(&iAllocFunc, &iFreeFunc, &iBackend);

                        if (!iAllocFunc || !iFreeFunc) {
                            ERROR("DKU_G: Failed acquiring ImGui runtime info");
                        }

                        DEBUG("DKU_G: Acquired ImGui runtime info"sv);
                    }
                }
            );

            while (!global::ImGuiQueue.empty()) {
                auto& [context, alloc] = global::ImGuiQueue.front();

                Invocable<ContextFunc>(context)(iContext);
                Invocable<AllocFunc>(alloc)(global::iAllocFunc, global::iFreeFunc, global::iBackend);

                global::ImGuiQueue.pop();
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            for (auto& func : HostQueue) {
                Invocable<CallbackFunc>(func)(a_this, a_syncInterval, a_flags);
            }

            ImGui::EndFrame();
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            D3D11.DeviceContext->OMSetRenderTargets(1, &D3D11.RenderView, nullptr);

            return Invocable<PresentFunc>(_Present)(a_this, a_syncInterval, a_flags);
        }


        inline bool TryAcquireD3DData() noexcept
        {
            DEBUG("DKU_G: Acquiring D3D11 dll info..."sv);

            DXGI_SWAP_CHAIN_DESC swapChainDesc{ };
            swapChainDesc.BufferCount = 1;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.OutputWindow = global::D3DHost.Window;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            swapChainDesc.Windowed = TRUE;

            D3D_FEATURE_LEVEL featureLevel{ };

            const auto result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, &global::D3D11.SwapChain, &global::D3D11.Device, &featureLevel, nullptr);

            DEBUG("DKU_G: D3D11 dll -> {}", result);

            return SUCCEEDED(result);
        }


        inline void LogResult(const LRESULT a_result, const char* a_action, const char* a_payload1, const std::uintptr_t a_payload2) noexcept
        {
            switch (a_result) {
            case LR_DKU_G_SUCCESS:
            {
                DEBUG("DKU_G: {} D3D11 callback succeeded -> {} @ {}.{:X}", a_action, a_payload1, global::LastSender, a_payload2);
                break;
            }
            case LR_DKU_G_FAILURE:
            {
                ERROR("DKU_G: {} D3D11 callback failed\n\nHost -> {}\n\nFunction -> {}", a_action, global::D3DHost.Name, a_payload1);
                break;
            }
            case LR_DKU_G_DISABLE:
            {
                DEBUG("DKU_G: {} D3D11 callback disabled -> {} @ {}.{:X}", a_action, a_payload1, global::LastSender, a_payload2);
                break;
            }
            default:
                ERROR("DKU_G: Unknown log result");
                break;
            }

            global::LastSender = Plugin::NAME;
        }
    } // namespace Impl


    /* API */

    // Init D3D11 hook or query a loaded D3D11 host
    inline void InitD3D() noexcept
    {
        using namespace detail;
        using namespace detail::global;

        DEBUG("DKU_G: Version {}", DKU_G_VERSION);

        if (IPC::TryInitHost<WM_DKU_G_QUERY_D, LR_DKU_G_SUCCESS>(D3DHost, "D3D11", HostWndProc, &D3D11, []()
            {// 2022/01/05 DISABLED IPC until further investigation
                if (TryAcquireD3DData()) {
                    auto _Hook_Present = Hook::AddVMTHook(D3D11.SwapChain, VMT_PRESENT_INDEX, FUNC_INFO(Hook_Present), (Hook::Patch*)nullptr);
                    _Present = Invocable<PresentFunc>(_Hook_Present->As<VMTHandle>()->OldAddress);
                    _Hook_Present->Enable();

                    // Init ImGui context & allocators
                    ImGui::CreateContext();
                    iContext = ImGui::GetCurrentContext();

                    return static_cast<bool>(iContext);
                } else {
                    return false;
                }
            })) {
            DEBUG("DKU_G: D3D11 init"sv);

        } else {
            ERROR("DKU_G: Failed initializing D3D11");
        }
    }


    inline LRESULT AddCallback(Hook::FuncInfo a_func, const bool a_remove) noexcept
    {
        using namespace detail;
        using namespace detail::global;

        D3DHost.Window = FindWindowExW(HWND_MESSAGE, nullptr, D3DHost.WndClass.c_str(), D3DHost.WndTitle.c_str());
        if (!D3DHost.Window) {
            char moduleName[MAX_PATH];
            GetModuleBaseNameA(GetCurrentProcess(), nullptr, moduleName, MAX_PATH);

            ERROR("DKU_G: It seems like the D3D11 host has been terminated!\nThis could be caused by thread switching, e.g. the host was initiated from a different thread.\n\nPlease contact the plugin author to synchronize call of [InitD3D11] and [AddCallback].\n\nProcess name -> {}", moduleName);
        }

        if (!a_func.Address) {
            ERROR("DKU_G: Invalid function {}", a_func.Name.empty() ? "INVALID_NAME"sv : a_func.Name);
        }

        LRESULT result{ LR_DKU_G_SUCCESS };

        if (D3DHost.Type == IPC::HostType::kSelf) {
            const auto& iter = std::find(HostQueue.begin(), HostQueue.end(), a_func.Address);

            if (iter != HostQueue.end() && a_remove) {
                HostQueue.erase(iter);
            } else if (iter == HostQueue.end()) {
                HostQueue.push_back(a_func.Address);
            } else {
                result = LR_DKU_G_DISABLE;
            }
        } else {
            result = SendMessageA(D3DHost.Window, a_remove ? WM_DKU_G_REMOVE : WM_DKU_G_ADD, AsAddress(std::addressof(a_func)), AsAddress(PROJECT_NAME));
        }

        LogResult(result, a_remove ? "Remove" : "Register", a_func.Name.data(), a_func.Address);

        return result;
    }    
    
    
    inline LRESULT RemoveCallback(Hook::FuncInfo a_func) noexcept
    {
        return AddCallback(a_func, true);
    }    
    
    
    // no definitive termination if hosting
    inline void Terminate() noexcept
    {
        using namespace detail;

        for (auto func : global::HostQueue) {
            RemoveCallback(RT_INFO(func, "Termination"sv));
        }
    }
} // namespace DKUtil::GUI

