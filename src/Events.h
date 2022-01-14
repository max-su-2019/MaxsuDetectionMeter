#pragma once

namespace MaxsuDetectionMeter
{
	class MenuOpenCloseEvent : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:

		virtual RE::BSEventNotifyControl ProcessEvent(RE::MenuOpenCloseEvent const* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source)
		{
			if (a_event->opening && a_event->menuName == RE::InterfaceStrings::GetSingleton()->console) {
				auto targetRef = RE::Console::GetSelectedRef() ? RE::Console::GetSelectedRef()->As<RE::Actor>() : nullptr;
				auto playerRef = RE::PlayerCharacter::GetSingleton();
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		static void Sink()
		{
		
			static MenuOpenCloseEvent singleton;
			auto ui = RE::UI::GetSingleton();
			ui->AddEventSink(&singleton);

			logger::debug("Register Menu Open Close Change Event!");
		}
	};
}