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
				if (targetRef && playerRef ) {
					auto GetDetectionLevel = [](RE::Actor* a_owner, RE::Actor* a_target) -> int32_t {
						int32_t detectionLevel = a_owner->RequestDetectionLevel(a_target, RE::DETECTION_PRIORITY::kNormal);
						if (detectionLevel < 0) {
							detectionLevel += 100;
							return detectionLevel = min(max(detectionLevel, 0), 100);
						}
						else
							return 100;
					};

					logger::debug("Get Detection Level is {}", GetDetectionLevel(targetRef, playerRef));
				}
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