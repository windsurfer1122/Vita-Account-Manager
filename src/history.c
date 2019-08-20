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

#include <vitasdk.h>

#include <main.h>
#include <file.h>
#include <history.h>

#include <debugScreen.h>
#define printf psvDebugScreenPrintf


struct History_Entry execution_history_entries[] = {
	{ "ur0:ci/file/", "m.log", false, false, },
	{ "ur0:user/00/shell/playlog/", "playlod.dat", false, false, },
	{ "ur0:user/00/shell/playlog/", "playlog.dat", false, false, },
	{ "ur0:user/00/shell/playlog/", "playlog.dat.tmp", false, false, },
	{ "vd0:history/", "data.bak", false, false, },
	{ "vd0:history/", "data.bin", false, false, },
};

struct History_Data execution_history_data = {
	.count = sizeof(execution_history_entries) / sizeof(execution_history_entries[0]),
	.count_protected = 0,
	.entries = execution_history_entries,
};


void get_current_execution_history_data(struct History_Data *hist_data)
{
	int i;
	char source_path[(MAX_PATH_LENGTH)];

	hist_data->count_protected = 0;
	for (i = 0; i < hist_data->count; i++) {
		hist_data->entries[i].file_available = false;
		hist_data->entries[i].file_protected = false;

		if ((hist_data->entries[i].file_path == NULL) || (hist_data->entries[i].file_name_path == NULL)) {
			continue;
		}

		sceClibStrncpy(source_path, hist_data->entries[i].file_path, (MAX_PATH_LENGTH));
		sceClibStrncat(source_path, hist_data->entries[i].file_name_path, (MAX_PATH_LENGTH));
		hist_data->entries[i].file_available = check_file_exists(source_path);
		hist_data->entries[i].file_protected = check_folder_exists(source_path);
		if (hist_data->entries[i].file_protected) {
			hist_data->count_protected++;
		}
	}

	return;
}

void display_execution_history_details(struct History_Data *hist_data, char *title)
{
	int i;

	if (title != NULL) {
		// draw title line
		draw_title_line(title);

		// draw pixel line
		draw_pixel_line(NULL, NULL);
	}

	// execution history data
	printf("Execution History Data:\e[0K\n");
	for (i = 0; i < hist_data->count; i++) {
		if ((hist_data->entries[i].file_path == NULL) || (hist_data->entries[i].file_name_path == NULL)) {
			continue;
		}

		printf("\e[2m%s\e[22m%s: ", hist_data->entries[i].file_path, hist_data->entries[i].file_name_path);

		if (hist_data->entries[i].file_protected) {
			printf("protected\e[0K\n");
		} else if (hist_data->entries[i].file_available) {
			printf("available\e[0K\n");
		} else {
			printf("missing\e[0K\n");
		}
	}

	if (title != NULL) {
		wait_for_cancel_button();
	}

	return;
}

void delete_execution_history(struct History_Data *hist_data, char *title)
{
	int i;
	char target_path[(MAX_PATH_LENGTH)];

	if (title != NULL) {
		// draw title line
		draw_title_line(title);

		// draw pixel line
		draw_pixel_line(NULL, NULL);
	}

	// execution history data
	for (i = 0; i < hist_data->count; i++) {
		if ((hist_data->entries[i].file_path == NULL) || (hist_data->entries[i].file_name_path == NULL)) {
			continue;
		}

		sceClibStrncpy(target_path, hist_data->entries[i].file_path, (MAX_PATH_LENGTH));
		sceClibStrncat(target_path, hist_data->entries[i].file_name_path, (MAX_PATH_LENGTH));

		if (check_folder_exists(target_path)) {
			printf("\e[2mSkip protected %s...\e[22m\e[0K\n", target_path);
		} else if (!check_file_exists(target_path)) {
			printf("\e[2mSkip missing %s...\e[22m\e[0K\n", target_path);
		} else {
			printf("\e[2mDeleting %s...\e[22m\e[0K\n", target_path);
			sceIoRemove(target_path);
		}
	}

	if (title != NULL) {
		printf("Execution history deleted!\nReboot to also clear list in memory!\e[0K\n");
		wait_for_cancel_button();
	}

	return;
}

void protect_execution_history_files(struct History_Data *hist_data, char *title)
{
	int i;
	char target_path[(MAX_PATH_LENGTH)];

	// draw title line
	draw_title_line(title);

	// draw pixel line
	draw_pixel_line(NULL, NULL);

	// protect execution history files
	for (i = 0; i < hist_data->count; i++) {
		if ((hist_data->entries[i].file_path == NULL) || (hist_data->entries[i].file_name_path == NULL)) {
			continue;
		}

		sceClibStrncpy(target_path, hist_data->entries[i].file_path, (MAX_PATH_LENGTH));
		sceClibStrncat(target_path, hist_data->entries[i].file_name_path, (MAX_PATH_LENGTH));

		if (check_folder_exists(target_path)) {
			printf("\e[2mSkip protected %s...\e[22m\e[0K\n", target_path);
		} else {
			if (check_file_exists(target_path)) {
				printf("\e[2mDeleting %s...\e[22m\e[0K\n", target_path);
				sceIoRemove(target_path);
			}
			printf("\e[2mCreating directory %s...\e[22m\e[0K\n", target_path);
			sceIoMkdir(target_path, 0006);
		}
	}

	printf("Execution history files deleted and protected!\nReboot to also clear list in memory!\e[0K\n");
	wait_for_cancel_button();

	return;
}

void unprotect_execution_history_files(struct History_Data *hist_data, char *title)
{
	int i;
	char target_path[(MAX_PATH_LENGTH)];

	// draw title line
	draw_title_line(title);

	// draw pixel line
	draw_pixel_line(NULL, NULL);

	// unprotect execution history files
	for (i = 0; i < hist_data->count; i++) {
		if ((hist_data->entries[i].file_path == NULL) || (hist_data->entries[i].file_name_path == NULL)) {
			continue;
		}

		sceClibStrncpy(target_path, hist_data->entries[i].file_path, (MAX_PATH_LENGTH));
		sceClibStrncat(target_path, hist_data->entries[i].file_name_path, (MAX_PATH_LENGTH));

		if (check_folder_exists(target_path)) {
			printf("\e[2mDeleting directory %s...\e[22m\e[0K\n", target_path);
			sceIoRmdir(target_path);
		} else {
			printf("\e[2mSkip unprotected %s...\e[22m\e[0K\n", target_path);
		}
	}

	printf("Execution history files unprotected!\e[0K\n");
	wait_for_cancel_button();

	return;
}
