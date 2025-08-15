#pragma once

#include "Color.h"

#include <cstdlib>
#include <string_view>
#include <type_traits>

namespace Util
{
	template <typename T> bool TryParse(std::string_view text, T &result, int radix = 10);

	inline Color ParseConfigColorString(const std::string &colorText, bool &success)
	{
		// Format: #ARGB or #RGB

		success           = false;

		const auto length = colorText.length();

		if (length != 9 && length != 7)
			return Color();

		std::array<BYTE, 4> colors;

		int j = 0;
		for (size_t i = 1; i < length; i += 2)
			if (!Util::TryParse<BYTE>(colorText.substr(i, 2), colors[j++], 16))
				return Color();

		if (length == 7)
		{
			for (size_t i = 3; i > 0; i--)
				colors[i] = colors[i - 1];

			colors[0] = 0xFF;
		}

		success = true;
		return Color(colors[1], colors[2], colors[3], colors[0]);
	}

	template <typename T> inline bool TryParse(std::string_view text, T &result, int radix)
	{
		bool success = false;

		if constexpr (std::is_same<T, float>())
		{
			char *end;
			float parseResult = std::strtof(text.data(), &end);
			if (*end == '\0')
			{
				result  = parseResult;
				success = true;
			}
		}
		else if constexpr (std::is_same<T, Color>())
		{
			result = ParseConfigColorString(text.data(), success);
		}
		else
		{
			char *end;
			long parseResult;
			parseResult = std::strtol(text.data(), &end, radix);
			if (*end == '\0')
			{
				success = true;
				result  = static_cast<T>(parseResult);
			}
		}

		return success;
	}
}