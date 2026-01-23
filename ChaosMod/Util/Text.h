#pragma once

inline std::string StringTrim(std::string str)
{
	if (str.find_first_not_of(' ') == str.npos)
		return "";

	str = str.substr(str.find_first_not_of(' '));
	str = str.substr(0, str.find_first_of('\0'));
	str = str.substr(0, str.find_last_not_of(' ') == str.npos ? str.npos : str.find_last_not_of(' ') + 1);
	return str;
};

inline int CompareCaseInsensitive(std::string_view a, std::string_view b)
{
	for (size_t i = 0;; i++)
	{
		if (i == a.size())
			return i - b.size();
		else if (i == b.size())
			return a.size() - i;

		auto ai = std::toupper(a[i]);
		auto bi = std::toupper(b[i]);
		if (ai != bi)
			return bi - ai;
	}
}

inline void RemoveSpaces(std::string &data)
{
	std::string buffer;
	buffer.reserve(data.size());
	for (size_t pos = 0; pos != data.size(); ++pos)
		if ((data[pos] >= 'a' && data[pos] <= 'z') || (data[pos] >= 'A' && data[pos] <= 'Z'))
			buffer.append(&data[pos], 1);
	data.swap(buffer);
}