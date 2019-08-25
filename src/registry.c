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

#include <stdlib.h>  // for malloc(), free()

#include <registry.h>

const char *const file_ext_bin = ".bin";
const char *const file_ext_txt = ".txt";


void free_reg_data(struct Registry_Data *reg_data)
{
	int i;

	if (reg_data == NULL) {
		return;
	}

	// free memory of each key value
	for (i = 0; i < reg_data->reg_count; i++) {
		if (reg_data->reg_entries[i].key_value != NULL) {
			free(reg_data->reg_entries[i].key_value);
		}
	}

	// free memory of reg_entries array
	free(reg_data->reg_entries);
	reg_data->reg_entries = NULL;
	reg_data->reg_count = 0;

	return;
}
