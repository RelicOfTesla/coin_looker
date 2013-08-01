#pragma once

#include <json/json.h>
#include <fstream>

static Json::Value json_from_string(const std::string& s)
{
	Json::Value val;
	Json::Reader rd;
	if (s.size())
	{
		rd.parse(s, val);
	}
	if (val.empty())
	{
		rd.parse("{}", val);
	}
	return val;
}
static Json::Value json_from_file(const std::string& filepath)
{
	Json::Value val;
	Json::Reader rd;
	std::ifstream file(filepath);
	if (!file.eof())
	{
		rd.parse(file, val);
#if _DEBUG
		if (val.empty())
		{
			std::string s = rd.getFormatedErrorMessages();
			printf("%s\n", s.c_str());
		}
#endif
	}
	return val;
}

static void json_to_file(Json::Value& v, const std::string& filepath)
{
	Json::StyledWriter writer;
	std::string data = writer.write(v);
	std::ofstream file(filepath);
	file << data;
}
