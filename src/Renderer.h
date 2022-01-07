#pragma once

namespace MaxsuDetectionMeter
{
	class Renderer
	{
	private:
		Renderer() = delete;

		static void DrawMeters();	//Rendering Meters.
		static void MessageCallback(SKSE::MessagingInterface::Message* msg);

		static bool ShowMeters;
	public:
		static bool Install();
	};
}