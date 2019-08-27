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
#include <vitasdk.h>

#include <dir.h>
#include <file.h>

#include <debugScreen.h>
#define printf psvDebugScreenPrintf

const char *const slash_folder = "/";


void create_path(char *check_path, int start_offset, int display) {
	char *value;

	if (check_path == NULL) {
		return;
	}

	// create path directories
	value = &check_path[start_offset];
	while ((value = strchr(value, '/')) != NULL) {
		*value = '\0';
		if (!check_folder_exists(check_path)) {
			if (display) {
				printf("\e[2mCreating folder %s/...\e[22m\e[0K\n", check_path);
			}
			sceIoMkdir(check_path, 0006);
		}
		*value++ = '/';
	}

	return;
}

int get_subdirs(const char *const base_path, struct Dir_Entry **dirs_ptr)
{
	int dir_count;
	int dir_count2;
	int do_count;
	SceUID dfd;
	SceIoDirent entry;
	struct Dir_Entry *dir;

	if (dirs_ptr == NULL) {
		return 0;
	}

	// read directories in base path
	*dirs_ptr = NULL;
	dir_count = 0;
	dir_count2 = 0;
	do_count = 2;
	do {
		do_count--;

		if ((dir_count > 0) && (*dirs_ptr == NULL)) {
			*dirs_ptr = (struct Dir_Entry *)malloc(dir_count * sizeof(struct Dir_Entry));
		}

		dfd = sceIoDopen(base_path);
		if (dfd >= 0) {
			sceClibMemset(&entry, 0, sizeof(SceIoDirent));
			while (sceIoDread(dfd, &entry) > 0) {
				if (!(SCE_S_ISDIR(entry.d_stat.st_mode))) {
					continue;
				}

				if (*dirs_ptr == NULL) {
					dir_count++;
				} else {
					dir = &((*dirs_ptr)[dir_count2]);
					dir->size = sceClibStrnlen(entry.d_name, sizeof(entry.d_name));
					dir->name = (char *)malloc(dir->size + 1);
					sceClibStrncpy(dir->name , entry.d_name, dir->size);
					dir->name[dir->size] = '\0';
					dir_count2++;
				}
			}
			sceIoDclose(dfd);
		}
	} while ((do_count > 0) && (dir_count > 0));

	return dir_count;
}

void free_subdirs(struct Dir_Entry *dirs, int dir_count)
{
	int i;

	for (i = 0; i < dir_count; i++) {
		free(dirs[i].name);
	}
	free(dirs);

	return;
}
