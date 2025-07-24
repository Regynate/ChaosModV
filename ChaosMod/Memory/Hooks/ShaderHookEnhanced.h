#pragma once

#include <string_view>


namespace Hooks
{
	void AddPixelShader(std::string_view shaderSrc);
	void RemovePixelShader(std::string_view shaderSrc);
}