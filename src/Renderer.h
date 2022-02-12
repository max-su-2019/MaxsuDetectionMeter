#pragma once

namespace MaxsuDetectionMeter
{
	class Renderer
	{
	private:
		Renderer() = delete;

		static void DrawMeters();	//Rendering Meters.
		static void MessageCallback(SKSE::MessagingInterface::Message* msg);

		//static inline RECT screenRect{};

		static inline bool ShowMeters = false;
	public:
		static bool Install();
		
		static float GetResolutionScaleWidth();  // { return  screenRect.right / 1920.f; }
		static float GetResolutionScaleHeight(); // { return  screenRect.bottom / 1080.f; }
	};
}