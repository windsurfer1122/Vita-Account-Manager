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

#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__

#include <stdbool.h> // for bool
#include <registry.h> // for Registry_Data

struct File_Entry {
	const char *const file_path;
	const char *const file_save_path;
	const char *const file_name_path;
	bool file_available;
};

struct File_Data {
	int count;
	size_t size;
	struct File_Entry *entries;
};

void init_reg_data(struct Registry_Data *reg_data);
void get_initial_reg_data(struct Registry_Data *reg_data);
void get_current_reg_data(struct Registry_Data *reg_data);

void init_file_data(struct File_Data *file_data);
void get_current_file_data(struct File_Data *file_data);

void display_account_details_short(struct Registry_Data *reg_data, bool *no_user);
void display_account_details_full(struct Registry_Data *reg_data, struct File_Data *file_data, char *title);
void save_account_details(struct Registry_Data *reg_data, struct File_Data *file_data, char *title);
bool switch_account(struct Registry_Data *reg_data, struct Registry_Data *reg_init_data, struct File_Data *file_init_data, char *title);
bool remove_account(struct Registry_Data *reg_data, struct Registry_Data *reg_init_data, struct File_Data *file_init_data, char *title);

void main_account(void);

#endif  /* __ACCOUNT_H__ */
