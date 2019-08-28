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

#include <common.h>
#include <dir.h>
#include <file.h>
#include <registry.h>

#include <debugScreen.h>
#define printf psvDebugScreenPrintf

const char *const file_ext_bin = ".bin";
const char *const file_ext_txt = ".txt";


void init_reg_data(struct Registry_Data **reg_data_ptr, const struct Registry_Data *const template_reg_data)
{
	struct Registry_Data *reg_data;
	int i;

	if (reg_data_ptr == NULL) {
		return;
	}

	if (*reg_data_ptr == NULL) {
		*reg_data_ptr = (struct Registry_Data *)malloc(sizeof(struct Registry_Data));
	}
	reg_data = *reg_data_ptr;

	// copy template to reg data plus new reg entries array
	if (template_reg_data == NULL) {
		sceClibMemset(reg_data, 0x00, sizeof(struct Registry_Data));
	} else {
		sceClibMemcpy((void *)reg_data, (void *)(template_reg_data), sizeof(struct Registry_Data));
		reg_data->reg_entries = (struct Registry_Entry *)malloc(template_reg_data->reg_size);
		sceClibMemcpy((void *)(reg_data->reg_entries), (void *)(template_reg_data->reg_entries), template_reg_data->reg_size);

		// alloc memory for each key
		for (i = 0; i < reg_data->reg_count; i++) {
			switch(reg_data->reg_entries[i].key_type) {
				case KEY_TYPE_INT:
					if (reg_data->reg_entries[i].key_size <= 0) {
						reg_data->reg_entries[i].key_size = sizeof(int);
					}
					break;
				case KEY_TYPE_STR:
				case KEY_TYPE_BIN:
					if (reg_data->reg_entries[i].key_size <= 0) {
						reg_data->reg_entries[i].key_size = (REG_BUFFER_DEFAULT_SIZE);
					}
					break;
				default:  // unknown type
					// TODO: error message unknown type
					continue;  // skip entry
					break;
			}
			reg_data->reg_entries[i].key_value = (void *)malloc(reg_data->reg_entries[i].key_size);
			sceClibMemset(reg_data->reg_entries[i].key_value, 0x00, reg_data->reg_entries[i].key_size);
		}
	}

	return;
}

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

void save_reg_data(const char *const base_path, const struct Registry_Data *const reg_data)
{
	int size_base_path;
	char target_path[(MAX_PATH_LENGTH)+1];
	int i;
	int size;
	char string[(STRING_BUFFER_DEFAULT_SIZE)+1];
	char *value;

	if ((base_path == NULL) || (reg_data == NULL)) {
		return;
	}

	string[(STRING_BUFFER_DEFAULT_SIZE)] = '\0';

	// prepare target path
	size_base_path = sceClibStrnlen(base_path, (MAX_PATH_LENGTH));
	sceClibStrncpy(target_path, base_path, (MAX_PATH_LENGTH));
	target_path[size_base_path] = '\0';
	target_path[(MAX_PATH_LENGTH)] = '\0';
	create_path(target_path, 0, 0);

	// save all registry data
	for (i = 0; i < reg_data->reg_count; i++) {
		if ((reg_data->reg_entries[i].key_save_path == NULL) || (reg_data->reg_entries[i].key_name == NULL)) {
			continue;
		}
		if (reg_data->reg_entries[i].key_value == NULL) {
			continue;
		}

		// build target path
		target_path[size_base_path] = '\0';
		sceClibStrncat(target_path, reg_data->reg_entries[i].key_save_path, (MAX_PATH_LENGTH));
		if (reg_data->reg_entries[i].key_path_extension != NULL) {
			sceClibStrncat(target_path, reg_data->reg_entries[i].key_path_extension, (MAX_PATH_LENGTH));
			sceClibStrncat(target_path, slash_folder, (MAX_PATH_LENGTH));
		}
		sceClibStrncat(target_path, reg_data->reg_entries[i].key_name, (MAX_PATH_LENGTH));

		// create target path directories
		create_path(target_path, size_base_path, 0);

		// save reg entry data as file
		switch(reg_data->reg_entries[i].key_type) {
			case KEY_TYPE_INT:
				sceClibStrncat(target_path, file_ext_txt, (MAX_PATH_LENGTH));
				sceClibSnprintf(string, (STRING_BUFFER_DEFAULT_SIZE), "%i", *((int *)(reg_data->reg_entries[i].key_value)));
				value = string;
				size = sceClibStrnlen(value, (reg_data->reg_entries[i].key_size));
				break;
			case KEY_TYPE_STR:
				sceClibStrncat(target_path, file_ext_txt, (MAX_PATH_LENGTH));
				value = (char *)(reg_data->reg_entries[i].key_value);
				size = sceClibStrnlen(value, (reg_data->reg_entries[i].key_size));
				break;
			case KEY_TYPE_BIN:
				sceClibStrncat(target_path, file_ext_bin, (MAX_PATH_LENGTH));
				value = (char *)(reg_data->reg_entries[i].key_value);
				size = reg_data->reg_entries[i].key_size;
				break;
			default:  // unknown type
				// TODO: error message unknown type
				continue;  // skip entry
				break;
		}
		printf("\e[2mWriting %s...\e[22m\e[0K\n", &(target_path[size_base_path]));  // reg_data->reg_entries[i].key_path_extension, reg_data->reg_entries[i].key_name  // TODO
		write_file(target_path, (void *)value, size);
	}
}

void set_reg_data(struct Registry_Data *reg_data, int slot)
{
	int i;
	char reg_path[(STRING_BUFFER_DEFAULT_SIZE)+1];
	int size;

	reg_path[(STRING_BUFFER_DEFAULT_SIZE)] = '\0';

	for (i = 0; i < reg_data->reg_count; i++) {
		if (reg_data->reg_entries[i].key_value == NULL) {
			continue;
		}

		// build registry path
		sceClibSnprintf(reg_path, (STRING_BUFFER_DEFAULT_SIZE), reg_data->reg_entries[i].key_path, slot);
		size = sceClibStrnlen(reg_path, (STRING_BUFFER_DEFAULT_SIZE));
		sceClibStrncat(reg_path, slash_folder, (STRING_BUFFER_DEFAULT_SIZE));
		if (reg_data->reg_entries[i].key_path_extension != NULL) {
			sceClibStrncat(reg_path, reg_data->reg_entries[i].key_path_extension, (STRING_BUFFER_DEFAULT_SIZE));
			size = sceClibStrnlen(reg_path, (STRING_BUFFER_DEFAULT_SIZE));
			sceClibStrncat(reg_path, slash_folder, (STRING_BUFFER_DEFAULT_SIZE));
		}
		sceClibStrncat(reg_path, reg_data->reg_entries[i].key_name, (MAX_PATH_LENGTH));
		printf("\e[2mSetting registry %s...\e[22m\e[0K\n", reg_path);
		reg_path[size] = '\0';

		switch(reg_data->reg_entries[i].key_type) {
			case KEY_TYPE_INT:
				sceRegMgrSetKeyInt(reg_path, reg_data->reg_entries[i].key_name, *((int *)(reg_data->reg_entries[i].key_value)));
				break;
			case KEY_TYPE_STR:
				((char *)(reg_data->reg_entries[i].key_value))[reg_data->reg_entries[i].key_size] = '\0';
				sceRegMgrSetKeyStr(reg_path, reg_data->reg_entries[i].key_name, (char *)(reg_data->reg_entries[i].key_value), reg_data->reg_entries[i].key_size);
				break;
			case KEY_TYPE_BIN:
				sceRegMgrSetKeyBin(reg_path, reg_data->reg_entries[i].key_name, reg_data->reg_entries[i].key_value, reg_data->reg_entries[i].key_size);
				break;
		}
	}

	return;
}

void load_reg_data(const char *const base_path, struct Registry_Data *reg_data, const struct Registry_Data *const reg_init_data, const int skip_reg_id_1, const int skip_reg_id_2)
{
	int size_base_path;
	char source_path[(MAX_PATH_LENGTH)+1];
	int i;
	int use_initial;
	int size;
	char *value;
	char string[(STRING_BUFFER_DEFAULT_SIZE)+1];

	if ((base_path == NULL) || (reg_data == NULL)) {
		return;
	}

	string[(STRING_BUFFER_DEFAULT_SIZE)] = '\0';

	// prepare source path
	size_base_path = sceClibStrnlen(base_path, (MAX_PATH_LENGTH));
	sceClibStrncpy(source_path, base_path, (MAX_PATH_LENGTH));
	source_path[size_base_path] = '\0';
	source_path[(MAX_PATH_LENGTH)] = '\0';

	// load all registry data
	for (i = 0; i < reg_data->reg_count; i++) {
		if (reg_data->reg_entries[i].key_id == skip_reg_id_1) {  // do not read special id, already stored in reg data from folder name
			continue;
		}
		if (reg_data->reg_entries[i].key_id == skip_reg_id_2) {  // do not read special id, already stored in reg data from folder name
			continue;
		}

		use_initial = 1;
		if ((reg_data->reg_entries[i].key_save_path != NULL) && (reg_data->reg_entries[i].key_name != NULL)) {
			use_initial = 0;
			// build source path
			source_path[size_base_path] = '\0';
			sceClibStrncat(source_path, reg_data->reg_entries[i].key_save_path, (MAX_PATH_LENGTH));
			if (reg_data->reg_entries[i].key_path_extension != NULL) {
				sceClibStrncat(source_path, reg_data->reg_entries[i].key_path_extension, (MAX_PATH_LENGTH));
				sceClibStrncat(source_path, slash_folder, (MAX_PATH_LENGTH));
			}
			sceClibStrncat(source_path, reg_data->reg_entries[i].key_name, (MAX_PATH_LENGTH));
			switch(reg_data->reg_entries[i].key_type) {
				case KEY_TYPE_INT:
					sceClibStrncat(source_path, file_ext_txt, (MAX_PATH_LENGTH));
					break;
				case KEY_TYPE_STR:
					sceClibStrncat(source_path, file_ext_txt, (MAX_PATH_LENGTH));
					break;
				case KEY_TYPE_BIN:
					sceClibStrncat(source_path, file_ext_bin, (MAX_PATH_LENGTH));
					break;
				default:  // unknown type
					// TODO: error message unknown type
					continue;  // skip entry
					break;
			}
		}

		// check and read source path
		if ((use_initial) || (!check_file_exists(source_path))) {
			if (use_initial) {
				printf("\e[2mUse initial %s...\e[22m\e[0K\n", &(source_path[size_base_path]));
			} else {
				printf("\e[2mUse initial for missing %s...\e[22m\e[0K\n", &(source_path[size_base_path]));
			}
			switch(reg_data->reg_entries[i].key_type) {
				case KEY_TYPE_INT:
					*((int *)(reg_data->reg_entries[i].key_value)) = *((int *)(reg_init_data->reg_entries[i].key_value));
					break;
				case KEY_TYPE_STR:
					sceClibStrncpy((char *)(reg_data->reg_entries[i].key_value), (char *)(reg_init_data->reg_entries[i].key_value), (reg_data->reg_entries[i].key_size) - 1);
					((char *)(reg_data->reg_entries[i].key_value))[reg_data->reg_entries[i].key_size] = '\0';
					break;
				case KEY_TYPE_BIN:
					sceClibMemcpy(reg_data->reg_entries[i].key_value, reg_init_data->reg_entries[i].key_value, reg_data->reg_entries[i].key_size);
					break;
				default:  // unknown type
					// TODO: error message unknown type
					continue;  // skip entry
					break;
			}
		} else {
			size = allocate_read_file(source_path, ((void **)(&value)));
			printf("\e[2mReading %s... (%i)\e[22m\e[0K\n", &(source_path[size_base_path]), size);
			switch(reg_data->reg_entries[i].key_type) {
				case KEY_TYPE_INT:
					size = min(size, (STRING_BUFFER_DEFAULT_SIZE) - 1);
					sceClibMemcpy((void *)string, (void *)value, size);
					string[size] = '\0';
					*((int *)(reg_data->reg_entries[i].key_value)) = atoi(string);
					break;
				case KEY_TYPE_STR:
					size = min(size, (reg_data->reg_entries[i].key_size) - 1);
					sceClibStrncpy((char *)(reg_data->reg_entries[i].key_value), value, size);
					((char *)(reg_data->reg_entries[i].key_value))[size] = '\0';
					break;
				case KEY_TYPE_BIN:
					size = min(size, reg_data->reg_entries[i].key_size);
					sceClibMemcpy(reg_data->reg_entries[i].key_value, (void *)value, size);
					break;
				default:  // unknown type
					// TODO: error message unknown type
					continue;  // skip entry
					break;
			}
			free(value);
		}
	}
}
