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

#ifndef __HISTORY_H__
#define __HISTORY_H__

struct History_Entry {
	const char *const file_path;
	const char *const file_name_path;
	int file_available;
	int file_protected;
};

struct History_Data {
	int count;
	int count_protected;
	struct History_Entry *entries;
};

extern struct History_Data execution_history_data;

void get_current_execution_history_data(struct History_Data *hist_data);
void display_execution_history_details(struct History_Data *hist_data, char *title);
void delete_execution_history(struct History_Data *hist_data, char *title);
void protect_execution_history_files(struct History_Data *hist_data, char *title);
void unprotect_execution_history_files(struct History_Data *hist_data, char *title);

#endif  /* __HISTORY_H__ */
