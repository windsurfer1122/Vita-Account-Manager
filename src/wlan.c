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

#include <common.h>
#include <dir.h>
#include <file.h>
#include <main.h>
#include <registry.h>
#include <wlan.h>

#include <debugScreen.h>
#define printf psvDebugScreenPrintf

const char *const wlans_folder = "wlans/";
const char *const reg_config_net_nn = "/CONFIG/NET/%02i";  // decimal 01-30
const char *const reg_config_net_eth = "/CONFIG/NET/ETH";
const char *const reg_config_net_app = "APP";
const char *const reg_config_net_common = "COMMON";
const char *const reg_config_net_ip = "IP";
const char *const reg_config_net_wifi = "WIFI";
const char *const file_reg_config_net = "registry/CONFIG/NET/";
const int reg_id_ssid = 217;
const int reg_id_conf_name = 210;
const int reg_id_http_proxy_port = 235;
const int reg_id_enable_auto_connect = 212;

// values from os0:kd/registry.db0 and https://github.com/devnoname120/RegistryEditorMOD/blob/master/regs.c
struct Registry_Entry template_wlan_reg_entries[] = {
	{ reg_id_ssid, reg_config_net_nn, NULL, reg_config_net_wifi, "ssid", KEY_TYPE_STR, 33, NULL, },
	{ 219, reg_config_net_nn, file_reg_config_net, reg_config_net_wifi, "wep_key", KEY_TYPE_STR, 27, NULL, },
	{ 218, reg_config_net_nn, file_reg_config_net, reg_config_net_wifi, "wifi_security", KEY_TYPE_INT, 4, NULL, },
	{ 220, reg_config_net_nn, file_reg_config_net, reg_config_net_wifi, "wpa_key", KEY_TYPE_STR, 65, NULL, },
	{ 233, reg_config_net_nn, file_reg_config_net, reg_config_net_app, "http_proxy_flag", KEY_TYPE_INT, 4, NULL, },
	{ reg_id_http_proxy_port, reg_config_net_nn, file_reg_config_net, reg_config_net_app, "http_proxy_port", KEY_TYPE_INT, 4, NULL, },
	{ 234, reg_config_net_nn, file_reg_config_net, reg_config_net_app, "http_proxy_server", KEY_TYPE_STR, 256, NULL, },
	{ 208, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "conf_flag", KEY_TYPE_INT, 4, NULL, },
	{ reg_id_conf_name, reg_config_net_nn, NULL, reg_config_net_common, "conf_name", KEY_TYPE_STR, 65, NULL, },
	{ 211, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "conf_serial_no", KEY_TYPE_INT, 4, NULL, },
	{ 209, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "conf_type", KEY_TYPE_INT, 4, NULL, },
	{ 214, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "device", KEY_TYPE_INT, 4, NULL, },
	{ reg_id_enable_auto_connect, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "enable_auto_connect", KEY_TYPE_INT, 4, NULL, },
	{ 215, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "ether_mode", KEY_TYPE_INT, 4, NULL, },
	{ 213, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "mtu", KEY_TYPE_INT, 4, NULL, },
	{ 225, reg_config_net_nn, file_reg_config_net, reg_config_net_ip, "auth_key", KEY_TYPE_STR, 128, NULL, },
	{ 224, reg_config_net_nn, file_reg_config_net, reg_config_net_ip, "auth_name", KEY_TYPE_STR, 128, NULL, },
	{ 228, reg_config_net_nn, file_reg_config_net, reg_config_net_ip, "default_route", KEY_TYPE_STR, 16, NULL, },
	{ 223, reg_config_net_nn, file_reg_config_net, reg_config_net_ip, "dhcp_hostname", KEY_TYPE_STR, 256, NULL, },
	{ 229, reg_config_net_nn, file_reg_config_net, reg_config_net_ip, "dns_flag", KEY_TYPE_INT, 4, NULL, },
	{ 226, reg_config_net_nn, file_reg_config_net, reg_config_net_ip, "ip_address", KEY_TYPE_STR, 16, NULL, },
	{ 222, reg_config_net_nn, file_reg_config_net, reg_config_net_ip, "ip_config", KEY_TYPE_INT, 4, NULL, },
	{ 227, reg_config_net_nn, file_reg_config_net, reg_config_net_ip, "netmask", KEY_TYPE_STR, 16, NULL, },
	{ 230, reg_config_net_nn, file_reg_config_net, reg_config_net_ip, "primary_dns", KEY_TYPE_STR, 16, NULL, },
	{ 231, reg_config_net_nn, file_reg_config_net, reg_config_net_ip, "secondary_dns", KEY_TYPE_STR, 16, NULL, },
};

struct Registry_Data template_wlan_reg_data = {
	.reg_count = sizeof(template_wlan_reg_entries) / sizeof(template_wlan_reg_entries[0]),
	.reg_size = sizeof(template_wlan_reg_entries),
	.reg_entries = template_wlan_reg_entries,
	.idx_username = -1,
	.idx_login_id = -1,
	.idx_ssid = -1,
	.idx_conf_name = -1,
};

struct Registry_Data *initial_wlan_reg_data;


void init_wlan_data(struct Wlan_Data *wlan_data)
{
	int j;

	wlan_data->wlan_found = 0;
	for (j = 0; j < (MAX_WLAN); j++) {
		wlan_data->wlan_reg_data[j] = NULL;
	}
}

void free_wlan_data(struct Wlan_Data *wlan_data)
{
	int j;
	struct Registry_Data *reg_data;

	// free memory of each reg data entry
	for (j = 0; j < (MAX_WLAN); j++) {
		reg_data = wlan_data->wlan_reg_data[j];
		if (reg_data == NULL) {
			continue;
		}

		free_reg_data(reg_data);
		free(reg_data);
		wlan_data->wlan_reg_data[j] = NULL;
	}
	wlan_data->wlan_found = 0;

	return;
}

void get_current_wlan_data(struct Wlan_Data *wlan_data)
{
	int i, j;
	char reg_path[(STRING_BUFFER_DEFAULT_SIZE)+1];
	char value[(STRING_BUFFER_DEFAULT_SIZE)+1];
	struct Registry_Data *reg_data;

	reg_path[(STRING_BUFFER_DEFAULT_SIZE)] = '\0';

	free_wlan_data(wlan_data);

	wlan_data->wlan_found = 0;
	for (j = 0; j < (MAX_WLAN); j++) {
		// build registry path
		sceClibSnprintf(reg_path, (STRING_BUFFER_DEFAULT_SIZE), template_wlan_reg_entries[template_wlan_reg_data.idx_ssid].key_path, j+1);
		sceClibStrncat(reg_path, slash_folder, (STRING_BUFFER_DEFAULT_SIZE));
		sceClibStrncat(reg_path, template_wlan_reg_entries[template_wlan_reg_data.idx_ssid].key_path_extension, (STRING_BUFFER_DEFAULT_SIZE));

		// get ssid
		sceClibMemset(value, 0x00, (STRING_BUFFER_DEFAULT_SIZE)+1);
		sceRegMgrGetKeyStr(reg_path, template_wlan_reg_entries[template_wlan_reg_data.idx_ssid].key_name, value, template_wlan_reg_entries[template_wlan_reg_data.idx_ssid].key_size);
		value[(STRING_BUFFER_DEFAULT_SIZE)] = '\0';

		// check ssid
		if (sceClibStrnlen(value, (STRING_BUFFER_DEFAULT_SIZE)) <= 0) {
			continue;
		}

		wlan_data->wlan_found++;
		init_reg_data(&(wlan_data->wlan_reg_data[j]), &template_wlan_reg_data);
		reg_data = wlan_data->wlan_reg_data[j];

		// fill each key value
		for (i = 0; i < reg_data->reg_count; i++) {
			if (reg_data->reg_entries[i].key_value == NULL) {
				continue;
			}
			sceClibMemset(reg_data->reg_entries[i].key_value, 0x00, reg_data->reg_entries[i].key_size);

			if (i == reg_data->idx_ssid) {  // special case: ssid already retrieved
				sceClibStrncpy((char *)(reg_data->reg_entries[i].key_value), value, reg_data->reg_entries[i].key_size);
				((char *)(reg_data->reg_entries[i].key_value))[reg_data->reg_entries[i].key_size] = '\0';
			} else {
				// build registry path
				sceClibSnprintf(reg_path, (STRING_BUFFER_DEFAULT_SIZE), reg_data->reg_entries[i].key_path, j+1);
				sceClibStrncat(reg_path, slash_folder, (STRING_BUFFER_DEFAULT_SIZE));
				sceClibStrncat(reg_path, reg_data->reg_entries[i].key_path_extension, (STRING_BUFFER_DEFAULT_SIZE));

				switch(reg_data->reg_entries[i].key_type) {
					case KEY_TYPE_INT:
						sceRegMgrGetKeyInt(reg_path, reg_data->reg_entries[i].key_name, (int *)(reg_data->reg_entries[i].key_value));
						break;
					case KEY_TYPE_STR:
						sceRegMgrGetKeyStr(reg_path, reg_data->reg_entries[i].key_name, (char *)(reg_data->reg_entries[i].key_value), reg_data->reg_entries[i].key_size);
						((char *)(reg_data->reg_entries[i].key_value))[reg_data->reg_entries[i].key_size] = '\0';
						break;
					case KEY_TYPE_BIN:
						sceRegMgrGetKeyBin(reg_path, reg_data->reg_entries[i].key_name, reg_data->reg_entries[i].key_value, reg_data->reg_entries[i].key_size);
						break;
				}
			}
		}  // for i < reg_data->reg_count
	}  // for j < (MAX_WLAN)
}

void save_wlan_details(struct Wlan_Data *wlan_data, char *title)
{
	int menu_redraw_screen;
	int menu_redraw;
	int menu_run;
	int menu_items;
	int menu_item;
	int x, y;
	int x2, y2;
	int x3, y3;
	int button_pressed;
	int i, j;
	struct Registry_Data *reg_data;
	int wlan_count2;
	int entries_per_screen;
	int count;
	char base_path[(MAX_PATH_LENGTH)+1];

	if (wlan_data == NULL) {
		return;
	}

	if (wlan_data->wlan_found <= 0) {
		return;
	}

	// run save menu
	menu_redraw_screen = 1;
	menu_redraw = 1;
	menu_run = 1;
	menu_items = 0;
	menu_item = 0;
	wlan_count2 = 0;
	count = wlan_count2;
	do {
		// redraw screen
		if (menu_redraw_screen) {
			// draw title line
			draw_title_line(title);

			// draw pixel line
			draw_pixel_line(NULL, NULL);
			psvDebugScreenGetCoordsXY(NULL, &y3);  // start of data
			x3 = 0;

			// draw info
			printf("The following %i WLANs are available: (L/R to page)\e[0K\n", wlan_data->wlan_found);

			// draw first part of menu
			psvDebugScreenGetCoordsXY(NULL, &y);  // start of menu
			x = 0;
			printf(" Cancel.\e[0K\n");
			psvDebugScreenGetCoordsXY(NULL, &y2);  // start of wlan list
			x2 = 0;

			entries_per_screen = ((SCREEN_HEIGHT) - y2 + 1) / psv_font_current->size_h;

			menu_redraw = 1;
			menu_redraw_screen = 0;
		}

		// redraw wlan list
		if (menu_redraw) {
			psvDebugScreenSetCoordsXY(&x2, &y2);
			printf("\e[0J");

			count = wlan_count2;
			menu_items = 0;
			for (i = 0, j = 0; (i < entries_per_screen) && (count < wlan_data->wlan_found) && (j < (MAX_WLAN)); j++) {
				reg_data = wlan_data->wlan_reg_data[j];
				if (reg_data == NULL) {
					continue;
				}
				if (reg_data->reg_entries == NULL) {
					continue;
				}
				if (reg_data->idx_ssid < 0) {
					continue;
				}

				menu_items++;
				i++;
				count++;
				printf(" %s (reg #%02i)\e[0K\n", (char *)(reg_data->reg_entries[reg_data->idx_ssid].key_value), j+1);
			}

			menu_redraw = 0;
		}

		// draw menu marker
		psvDebugScreenSetCoordsXY(&x, &y);
		//
		if (menu_item < 0) {
			menu_item = 0;
		}
		if (menu_item > menu_items) {
			menu_item = menu_items;
		}
		//
		for (i = 0; i <= menu_items; i++) {
			if (menu_item == i) {
				printf(">\n");
			} else {
				printf(" \n");
			}
		}

		// process key strokes
		button_pressed = get_key();
		if (button_pressed == SCE_CTRL_DOWN) {
			menu_item++;
		} else if (button_pressed == SCE_CTRL_UP) {
			menu_item--;
		} else if (button_pressed == SCE_CTRL_RTRIGGER) {
			if (count < wlan_data->wlan_found) {
				wlan_count2 += entries_per_screen;
				if (wlan_count2 >= wlan_data->wlan_found) {
					wlan_count2 = wlan_data->wlan_found - 1;
				}
				menu_redraw = 1;
			}
		} else if (button_pressed == SCE_CTRL_LTRIGGER) {
			if (wlan_count2 > 0) {
				wlan_count2 -= entries_per_screen;
				if (wlan_count2 < 0) {
					wlan_count2 = 0;
				}
				menu_redraw = 1;
			}
		} else if (button_pressed == button_enter) {
			if (menu_item == 0) {  // cancel
				menu_run = 0;
			} else if (menu_item > 0) {  // save wlan
				// find corresponding wlan
				i = wlan_count2 + menu_item;
				for (j = 0; j < (MAX_WLAN); j++) {
					reg_data = wlan_data->wlan_reg_data[j];
					if (reg_data == NULL) {
						continue;
					}
					if (reg_data->reg_entries == NULL) {
						continue;
					}
					if (reg_data->idx_ssid < 0) {
						continue;
					}
					if (--i > 0) {
						continue;
					}

					// clear data part of screen
					psvDebugScreenSetCoordsXY(&x3, &y3);
					printf("\e[0J");

					// build target base path
					base_path[(MAX_PATH_LENGTH)] = '\0';
					sceClibStrncpy(base_path, app_base_path, (MAX_PATH_LENGTH));
					sceClibStrncat(base_path, wlans_folder, (MAX_PATH_LENGTH));
					sceClibStrncat(base_path, (char *)(reg_data->reg_entries[reg_data->idx_ssid].key_value), (MAX_PATH_LENGTH));
					sceClibStrncat(base_path, slash_folder, (MAX_PATH_LENGTH));
					printf("Saving WLAN details to %s...\e[0K\n", base_path);

					// save wlan registry data
					save_reg_data(base_path, reg_data);

					printf("WLAN %s (reg #%02i) saved!\e[0K\n", (char *)(reg_data->reg_entries[reg_data->idx_ssid].key_value), j+1);
					wait_for_cancel_button();
					menu_redraw_screen = 1;
					menu_redraw = 1;
					break;
				}
			}
		}
	} while (menu_run);

	return;
}

void read_wlan_details(struct Registry_Data *reg_data, struct Registry_Data *reg_init_data)
{
	char base_path[(MAX_PATH_LENGTH)+1];

	// build source base path
	base_path[(MAX_PATH_LENGTH)] = '\0';
	sceClibStrncpy(base_path, app_base_path, (MAX_PATH_LENGTH));
	sceClibStrncat(base_path, wlans_folder, (MAX_PATH_LENGTH));
	sceClibStrncat(base_path, (char *)(reg_data->reg_entries[reg_data->idx_ssid].key_value), (MAX_PATH_LENGTH));
	sceClibStrncat(base_path, slash_folder, (MAX_PATH_LENGTH));
	printf("Reading WLAN details from %s...\e[0K\n", base_path);

	// load wlan registry data
	load_reg_data(base_path, reg_data, reg_init_data, reg_id_ssid, reg_id_conf_name);

	return;
}

void load_wlan_details(struct Wlan_Data *wlan_data, char *title)
{
	int menu_redraw;
	int menu_redraw_screen;
	int menu_run;
	int menu_items;
	int menu_item;
	int x, y;
	int x2, y2;
	int x3, y3;
	int button_pressed;
	int i, j;
	struct Registry_Data *reg_data;
	int size_base_path;
	char base_path[(MAX_PATH_LENGTH)+1];
	struct Dir_Entry *dirs;
	int dir_count;
	int dir_count2;
	int entries_per_screen;
	int size;
	int count;
	int free_slot, update_slot;

	// build source base path
	base_path[(MAX_PATH_LENGTH)] = '\0';
	sceClibStrncpy(base_path, app_base_path, (MAX_PATH_LENGTH));
	sceClibStrncat(base_path, wlans_folder, (MAX_PATH_LENGTH));
	size_base_path = sceClibStrnlen(base_path, (MAX_PATH_LENGTH));

	// read directories in base path
	base_path[size_base_path - 1] = '\0';
	dirs = NULL;
	dir_count = get_subdirs(base_path, &dirs);
	base_path[size_base_path - 1] = '/';

	// run switch menu
	menu_redraw_screen = 1;
	menu_redraw = 1;
	menu_run = 1;
	menu_items = 0;
	menu_item = 0;
	dir_count2 = 0;
	count = dir_count2;
	free_slot = -1;
	do {
		// redraw screen
		if (menu_redraw_screen) {
			// draw title line
			draw_title_line(title);

			// draw pixel line
			draw_pixel_line(NULL, NULL);
			psvDebugScreenGetCoordsXY(NULL, &y3);  // start of data
			x3 = 0;

			// draw info
			printf("The following %i WLANs are available: (L/R to page)\e[0K\n", dir_count);

			// draw first part of menu
			psvDebugScreenGetCoordsXY(NULL, &y);  // start of menu
			x = 0;
			printf(" Cancel.\e[0K\n");
			psvDebugScreenGetCoordsXY(NULL, &y2);  // start of wlan list
			x2 = 0;

			entries_per_screen = ((SCREEN_HEIGHT) - y2 + 1) / psv_font_current->size_h;

			menu_redraw = 1;
			menu_redraw_screen = 0;
		}

		// redraw wlan list
		if (menu_redraw) {
			psvDebugScreenSetCoordsXY(&x2, &y2);
			printf("\e[0J");

			count = dir_count2;
			menu_items = 0;
			for (i = 0; (i < entries_per_screen) && (count < dir_count); i++, count++) {
				menu_items++;
				size = (dirs[count].size > (template_wlan_reg_data.reg_entries[template_wlan_reg_data.idx_ssid].key_size - 1));
				if (size) {
					printf("\e[2m");
				}
				printf(" %.*s", template_wlan_reg_data.reg_entries[template_wlan_reg_data.idx_ssid].key_size, dirs[count].name);
				if (size) {
					printf("... (name too long)\e[22m");
				} else {
					update_slot = -1;
					for (j = 0; j < (MAX_WLAN); j++) {
						reg_data = wlan_data->wlan_reg_data[j];
						if (reg_data == NULL) {
							if (free_slot < 0) {
								free_slot = j;
							}
							continue;
						}
						if (reg_data->reg_entries == NULL) {
							continue;
						}
						if (reg_data->idx_ssid < 0) {
							continue;
						}

						if (sceClibStrcmp(dirs[count].name, (char *)(reg_data->reg_entries[reg_data->idx_ssid].key_value)) == 0) {
							update_slot = j;
							break;
						}
					}
					if (update_slot >= 0) {
						printf(" (update #%02i)", update_slot+1);
					} else {
						printf(" (add #%02i)", free_slot+1);
					}
				}
				printf("\e[0K\n");
			}

			menu_redraw = 0;
		}

		// draw menu marker
		psvDebugScreenSetCoordsXY(&x, &y);
		//
		if (menu_item < 0) {
			menu_item = 0;
		}
		if (menu_item > menu_items) {
			menu_item = menu_items;
		}
		//
		for (i = 0; i <= menu_items; i++) {
			if (menu_item == i) {
				printf(">\n");
			} else {
				printf(" \n");
			}
		}

		// process key strokes
		button_pressed = get_key();
		if (button_pressed == SCE_CTRL_DOWN) {
			menu_item++;
		} else if (button_pressed == SCE_CTRL_UP) {
			menu_item--;
		} else if (button_pressed == SCE_CTRL_RTRIGGER) {
			if (count < dir_count) {
				dir_count2 += entries_per_screen;
				if (dir_count2 >= dir_count) {
					dir_count2 = dir_count - 1;
				}
				menu_redraw = 1;
			}
		} else if (button_pressed == SCE_CTRL_LTRIGGER) {
			if (dir_count2 > 0) {
				dir_count2 -= entries_per_screen;
				if (dir_count2 < 0) {
					dir_count2 = 0;
				}
				menu_redraw = 1;
			}
		} else if (button_pressed == button_enter) {
			if (menu_item == 0) {  // cancel
				menu_run = 0;
			} else if (menu_item > 0) {  // load wlan
				i = dir_count2 + menu_item - 1;
				size = (dirs[i].size > (template_wlan_reg_data.reg_entries[template_wlan_reg_data.idx_ssid].key_size - 1));
				if (!size) {
					struct Registry_Data *reg_new_data;

					update_slot = -1;
					for (j = 0; j < (MAX_WLAN); j++) {
						reg_data = wlan_data->wlan_reg_data[j];
						if (reg_data == NULL) {
							if (free_slot < 0) {
								free_slot = j;
							}
							continue;
						}
						if (reg_data->reg_entries == NULL) {
							continue;
						}
						if (reg_data->idx_ssid < 0) {
							continue;
						}

						if (sceClibStrcmp(dirs[i].name, (char *)(reg_data->reg_entries[reg_data->idx_ssid].key_value)) == 0) {
							update_slot = j;
							break;
						}
					}
					if (update_slot < 0) {
						update_slot = free_slot;
					}

					// clear data part of screen
					psvDebugScreenSetCoordsXY(&x3, &y3);
					printf("\e[0J");
					// initialize data for wlan to be added/updated
					reg_new_data = NULL;
					init_reg_data(&reg_new_data, &template_wlan_reg_data);
					// copy ssid plus conf_name
					sceClibStrncpy((char *)(reg_new_data->reg_entries[reg_new_data->idx_ssid].key_value), dirs[i].name, (reg_new_data->reg_entries[reg_new_data->idx_ssid].key_size - 1));
					sceClibStrncpy((char *)(reg_new_data->reg_entries[reg_new_data->idx_conf_name].key_value), dirs[i].name, (reg_new_data->reg_entries[reg_new_data->idx_conf_name].key_size - 1));
					// read wlan data
					read_wlan_details(reg_new_data, initial_wlan_reg_data);
					// set wlan registry data
					set_reg_data(reg_new_data, update_slot+1);
					//
					printf("WLAN %s restored!\e[0K\n", (char *)(reg_new_data->reg_entries[reg_new_data->idx_ssid].key_value));
					free_reg_data(reg_new_data);
					free(reg_new_data);
				}
				wait_for_cancel_button();
				menu_redraw_screen = 1;
				menu_redraw = 1;
			}
		}
	} while (menu_run);

	// free memory
	free_subdirs(dirs, dir_count);

	return;
}

void main_wlan(void)
{
	int i;
	char base_path[(MAX_PATH_LENGTH)+1];

	// create wlans base path
	base_path[(MAX_PATH_LENGTH)] = '\0';
	sceClibStrncpy(base_path, app_base_path, (MAX_PATH_LENGTH));
	sceClibStrncat(base_path, wlans_folder, (MAX_PATH_LENGTH));
	create_path(base_path, 0, 0);

	// determine special indexes of registry data
	for (i = 0; i < template_wlan_reg_data.reg_count; i++) {
		// ssid
		if ((template_wlan_reg_data.idx_ssid < 0) && (template_wlan_reg_data.reg_entries[i].key_id == reg_id_ssid)) {
			template_wlan_reg_data.idx_ssid = i;
		}
		// conf_name
		if ((template_wlan_reg_data.idx_conf_name < 0) && (template_wlan_reg_data.reg_entries[i].key_id == reg_id_conf_name)) {
			template_wlan_reg_data.idx_conf_name = i;
		}
		// all found?
		if ((template_wlan_reg_data.idx_ssid >= 0) && (template_wlan_reg_data.idx_conf_name >= 0)) {
			break;
		}
	}

	// create initial wlan reg data
	init_reg_data(&initial_wlan_reg_data, &template_wlan_reg_data);
	for (i = 0; i < initial_wlan_reg_data->reg_count; i++) {
		// http_proxy_port
		if (initial_wlan_reg_data->reg_entries[i].key_id == reg_id_http_proxy_port) {
			*((int *)(initial_wlan_reg_data->reg_entries[i].key_value)) = 8080;
		}
		// enable_auto_connect
		if (initial_wlan_reg_data->reg_entries[i].key_id == reg_id_enable_auto_connect) {
			*((int *)(initial_wlan_reg_data->reg_entries[i].key_value))  = 1;
		}
	}
}
