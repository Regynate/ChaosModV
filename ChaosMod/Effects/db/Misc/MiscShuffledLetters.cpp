#include <stdafx.h>

#include <cwctype>

#include "Memory/Hooks/GetLabelTextHook.h"

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR int function_id;

static std::string &Shuffle(std::string &str)
{
	for (int i = 0; i < str.size(); ++i)
	{
		int j  = g_Random.GetRandomInt(0, str.size() - 1);
		char t = str[j];
		str[j] = str[i];
		str[i] = t;
	}

	return str;
}

static std::string Process(std::string_view sv)
{
	std::string result;

	std::string word("");
	bool ignoreWord = false;

	for (const wchar_t &ch : sv)
	{
		if (ch == '~')
		{
			ignoreWord = !ignoreWord;
			result += Shuffle(word);
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
			result += Shuffle(word);
			word = "";
			result += ch;
		}
		else
		{
			word += ch;
		}
	}

	result += Shuffle(word);

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
        .Name = "Shuffled Letters", 
        .Id = "misc_shuffled_letters", 
        .IsTimed = true
    }
);
