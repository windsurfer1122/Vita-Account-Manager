/*
  Vita Account Manager - Switch between multiple PSN/SEN accounts on a PS Vita or PS TV.
  Copyright (C) 2019  "windsurfer1122"
  https://github.com/windsurfer1122

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#include <vitasdk.h>

enum {
	KEY_TYPE_INT=0,
	KEY_TYPE_STR=1,
	KEY_TYPE_BIN=2,
};

struct Registry_Entry {
	int key_id;
	const char *const key_path;
	const char *const key_save_path;
	const char *const key_path_extension;
	const char *const key_name;
	int key_type;
	int key_size;
	void *key_value;
};

struct Registry_Data {
	int count;
	size_t size;
	int idx_username;
	int idx_login_id;
	int idx_ssid;
	struct Registry_Entry *entries;
};

#define REG_BUFFER_DEFAULT_SIZE 256

const char *const file_ext_bin;
const char *const file_ext_txt;

#endif  /* __REGISTRY_H__ */
