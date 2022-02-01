#include "ConsoleCommands.h"
#include "DataHandler.h"

namespace MaxsuDetectionMeter
{
	bool ConsoleCommands::Exec(const RE::SCRIPT_PARAMETER*, RE::SCRIPT_FUNCTION::ScriptData* a_scriptData, RE::TESObjectREFR*, RE::TESObjectREFR*, RE::Script*, RE::ScriptLocals*, double&, std::uint32_t&)
	{
		auto strChunk = a_scriptData->GetStringChunk();
		auto command = strChunk->GetString();
		std::transform(command.begin(), command.end(), command.begin(), ::tolower);

		static std::map<std::string, func_ptr> funcMap = {std::make_pair("reload", &ReloadConfig)};
		auto it = funcMap.find(command);
		if (it != funcMap.end() && it->second()) {
			CPrint("Execute Command %s Successfully", it->first.c_str());
			return true;
		}

		CPrint("Not Valid Command Execute!");
		return false;
	}

	void ConsoleCommands::Register()
	{
		using Type = RE::SCRIPT_PARAM_TYPE;

		auto info = RE::SCRIPT_FUNCTION::LocateConsoleCommand("ShowWhoDetectsPlayer");  // Unused
		if (info) {
			static RE::SCRIPT_PARAMETER params[] = {
				{ "String", Type::kChar, 0 }
			};

			info->functionName = LONG_NAME;
			info->shortName = SHORT_NAME;
			info->helpString = HelpStr();
			info->referenceFunction = false;
			info->SetParameters(params);
			info->executeFunction = Exec;
			info->conditionFunction = nullptr;
			info->editorFilter = false;
			info->invalidatesCellList = false;

			logger::info(FMT_STRING("Registered console command: {} ({})"), LONG_NAME, SHORT_NAME);
		}
		else {
			logger::error(FMT_STRING("Failed to register console command: {} ({})"), LONG_NAME, SHORT_NAME);
		}
	}

	void ConsoleCommands::CPrint(const char* a_fmt, ...)
	{
		auto console = RE::ConsoleLog::GetSingleton();
		if (console && console->IsConsoleMode()) {
			std::va_list args;
			va_start(args, a_fmt);
			console->VPrint(a_fmt, args);
			va_end(args);
		}
	}

	const char* ConsoleCommands::HelpStr()
	{
		static const std::string help = []() {
			std::string str;
			str += "<MaxsuDetectionMeter> \" \" <comment>";
			str += "\n\t<MaxsuDetectionMeter> ::= \"MaxsuDetectionMeter\" | \"MaxsuDM\"";
			str += "\n\t<comment> ::= <string> ; The comment string";
			return str;
		}();

		return help.c_str();
	}


	bool ConsoleCommands::ReloadConfig()
	{
		MeterHandler::GetSingleton()->meterSettings->Reload();
		CPrint("Reload Configuration Settings!");
		return true;
	}
}


