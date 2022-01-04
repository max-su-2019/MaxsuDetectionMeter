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
					auto group = targetRef->GetCombatGroup();
					if (group) {
						for (auto target : group->targets) {
							if (target.targetHandle && target.targetHandle.get() && target.targetHandle.get()->IsPlayerRef()) {
								auto searchState = group->searchState;
								auto detectionLevel = target.detectLevel;
								auto stealthPoint = target.attackedStealthPoints;
								logger::debug("Searach State is {}, detectionLevel is {}, stealthPoint is {}", searchState, detectionLevel, stealthPoint);
							}
						}
					}
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