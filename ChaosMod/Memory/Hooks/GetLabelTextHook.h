#pragma once

#include <functional>
#include <string>
#include <string_view>

namespace Hooks
{
	int AddTextProcessMethod(std::function<std::string(std::string_view)> foo);
	void RemoveTextProcessMethod(int id);

	void AddCustomLabel(std::string_view label, const std::string &text);
	void ClearCustomLabels();
}