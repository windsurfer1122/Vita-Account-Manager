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

#ifndef __DIR_H__
#define __DIR_H__

struct Dir_Entry {
	size_t size;
	char *name;
};

extern const char *const slash_folder;

void create_path(char *check_path, int start_offset, int display);
int get_subdirs(const char *const base_path, struct Dir_Entry **dirs_ptr);
void free_subdirs(struct Dir_Entry *dirs, int dir_count);

#endif  /* __DIR_H__ */
