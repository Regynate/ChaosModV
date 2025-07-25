#include <stdafx.h>

#include <cwctype>

#include "Memory/Hooks/GetLabelTextHook.h"

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR int function_id;

static std::string &Buttify(std::string &word, float chance)
{
	if (word.empty())
		return word;

	// 10% chance of buttification
	if (g_Random.GetRandomFloat(0, 1) < chance)
	{
		bool allCaps     = true;
		bool capitalized = std::iswupper(word[0]);

		for (const auto &ch : word)
		{
			if (!std::iswupper(ch))
			{
				allCaps = false;
				break;
			}
		}

		const char *replacement;
		if (allCaps)
			replacement = "BUTT";
		else if (capitalized)
			replacement = "Butt";
		else
			replacement = "butt";

		if (word.size() >= 8 && g_Random.GetRandomFloat(0, 1) > 0.5)
			word = replacement + word.substr(g_Random.GetRandomInt(3, 5));
		else
			word = replacement;
	}

	return word;
}

static std::string Process(std::string_view sv)
{
	std::string result;

	std::string word("");
	bool ignoreWord    = false;
	bool multipleWords = false;

	for (const wchar_t &ch : sv)
	{
		if (ch == '~')
		{
			ignoreWord = !ignoreWord;
			result += Buttify(word, 0.2f);
			word = "";
			result += ch;
			continue;
		}

		if (ignoreWord)
		{
			result += ch;
			continue;
		}

		if (ch == ' ' || std::iswpunct(ch))
		{
			result += Buttify(word, 0.2f);
			word = "";
			result += ch;
			multipleWords = true;
		}
		else
		{
			word += ch;
		}
	}

	result += Buttify(word, multipleWords ? 0.2f : 0.05f);

	return result;
}

static void OnStart()
{
	function_id = Hooks::AddTextProcessMethod(Process);
}

static void OnStop()
{
	Hooks::RemoveTextProcessMethod(function_id);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, nullptr, 
    {
        .Name = "Buttsbot", 
        .Id = "misc_buttsbot", 
        .IsTimed = true
    }
);
