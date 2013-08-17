/*
Copyright (C) 2008-2013 Vana Development Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#pragma once

extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "ExternalIp.h"
#include "IConfig.h"
#include "Types.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

struct DbConfig;
struct LogConfig;

class ConfigFile {
public:
	ConfigFile(const string &filename, bool executeFile = true);
	ConfigFile();
	~ConfigFile();
	void loadFile(const string &filename);
	bool keyExists(const string &value);
	bool execute();
	void handleError();
	void printError(const string &error);
	void setVariable(const string &name, const string &value);
	void setVariable(const string &name, int32_t value);
	template <typename T> T get(const string &value);
	template <> bool get<bool>(const string &value);
	template <typename T> T getClass(const string &prefix = "");

	string getString(const string &value);
	IpMatrix getIpMatrix(const string &value);
	vector<int8_t> getBossChannels(const string &value, size_t maxChannels);
private:
	void keyMustExist(const string &value);

	lua_State * getLuaState() const { return m_luaVm; }
	lua_State *m_luaVm;
	string m_file;
};

template <typename T>
T ConfigFile::get(const string &value) {
	keyMustExist(value);
	lua_getglobal(getLuaState(), value.c_str());
	T val = static_cast<T>(lua_tointeger(getLuaState(), -1));
	lua_pop(getLuaState(), 1);
	return val;
}

template <>
bool ConfigFile::get<bool>(const string &value) {
	keyMustExist(value);
	lua_getglobal(getLuaState(), value.c_str());
	bool val = lua_toboolean(getLuaState(), -1) != 0;
	lua_pop(getLuaState(), 1);
	return val;
}

template <typename T>
T ConfigFile::getClass(const string &prefix) {
	T obj;
	obj.read(*this, prefix);
	return obj;
}