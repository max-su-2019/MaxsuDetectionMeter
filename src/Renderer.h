#pragma once

namespace MaxsuDetectionMeter
{
	class Renderer
	{
	private:
		Renderer() = delete;

		static void DrawMeters();	//Rendering Meters.
		static void AddCallback(SKSE::MessagingInterface::Message* msg);

	public:
		static bool Install();
	};

}