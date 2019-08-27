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

#include <dir.h>
#include <file.h>
#include <main.h>

#include <debugScreen.h>
#define printf psvDebugScreenPrintf

const char *const console_folder = "console/";
const char *const idps_bin = "idps.bin";
const char *const psid_bin = "psid.bin";


void save_console_details(char *title)
{
	int size_base_path;
	int size_target_path;
	char base_path[(MAX_PATH_LENGTH)+1];
	char target_path[(MAX_PATH_LENGTH)+1];
	char idps[16];
	struct SceKernelOpenPsId psid;

	base_path[(MAX_PATH_LENGTH)] = '\0';
	target_path[(MAX_PATH_LENGTH)] = '\0';

	// draw title line
	draw_title_line(title);

	// draw pixel line
	draw_pixel_line(NULL, NULL);

	// build target base path
	sceClibStrncpy(base_path, app_base_path, (MAX_PATH_LENGTH));
	sceClibStrncat(base_path, console_folder, (MAX_PATH_LENGTH));
	size_base_path = sceClibStrnlen(base_path, (MAX_PATH_LENGTH));
	printf("Saving console details to %s...\e[0K\n", base_path);
	// create target base path directories
	create_path(base_path, 0, 0);
	//
	sceClibStrncpy(target_path, base_path, (MAX_PATH_LENGTH));
	target_path[size_base_path] = '\0';
	size_target_path = size_base_path;

	// save console data
	// IDPS
	target_path[size_target_path] = '\0';
	sceClibStrncat(target_path, idps_bin, (MAX_PATH_LENGTH));
	sceClibMemset(&idps, 0x00, sizeof(idps));
	_vshSblAimgrGetConsoleId(idps);
	printf("\e[2mWriting %s...\e[22m\e[0K\n", idps_bin);
	write_file(target_path, (void *)(idps), sizeof(idps));
	// PSID
	target_path[size_target_path] = '\0';
	sceClibStrncat(target_path, psid_bin, (MAX_PATH_LENGTH));
	sceClibMemset(&psid, 0x00, sizeof(psid));
	sceKernelGetOpenPsId(&psid);
	printf("\e[2mWriting %s...\e[22m\e[0K\n", psid_bin);
	write_file(target_path, (void *)(&(psid.id)), sizeof(psid.id));

	printf("Console details saved!\e[0K\n");

	wait_for_cancel_button();

	return;
}
