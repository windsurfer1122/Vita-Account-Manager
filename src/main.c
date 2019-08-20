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
#include <account.h>
#include <console.h>
#include <file.h>
#include <history.h>

#include <debugScreen.h>
#define printf psvDebugScreenPrintf

const char *const app_base_path = "ux0:data/" VITA_TITLEID "/";
const char *const slash_folder = "/";

// DebugScreen Font Handling
PsvDebugScreenFont *psv_font_default_1x;
PsvDebugScreenFont *psv_font_default_2x;
PsvDebugScreenFont *psv_font_current;

// Control Handling
int button_enter;
int button_cancel;


void determine_enter_cancel_button(int *ctrl_enter, int *ctrl_cancel)
{
	SceAppUtilInitParam init_param;
	SceAppUtilBootParam boot_param;
	int enter_button;

	// init sceapputil
	sceClibMemset(&init_param, 0, sizeof(SceAppUtilInitParam));
	sceClibMemset(&boot_param, 0, sizeof(SceAppUtilBootParam));
	sceAppUtilInit(&init_param, &boot_param);

	// get enter definition
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &enter_button);
	if (enter_button == SCE_SYSTEM_PARAM_ENTER_BUTTON_CROSS) {
		if (ctrl_enter != NULL) {
			*ctrl_enter = SCE_CTRL_CROSS;
		}
		if (ctrl_cancel != NULL) {
			*ctrl_cancel = SCE_CTRL_CIRCLE;
		}
	} else {
		if (ctrl_enter != NULL) {
			*ctrl_enter = SCE_CTRL_CIRCLE;
		}
		if (ctrl_cancel != NULL) {
			*ctrl_cancel = SCE_CTRL_CROSS;
		}
	}

	return;
}

int get_key()
{
	static unsigned buttons[] = {
		SCE_CTRL_SELECT,
		SCE_CTRL_START,
		SCE_CTRL_UP,
		SCE_CTRL_RIGHT,
		SCE_CTRL_DOWN,
		SCE_CTRL_LEFT,
		SCE_CTRL_LTRIGGER,
		SCE_CTRL_RTRIGGER,
		SCE_CTRL_TRIANGLE,
		SCE_CTRL_CIRCLE,
		SCE_CTRL_CROSS,
		SCE_CTRL_SQUARE,
	};

	static unsigned prev = 0;
	SceCtrlData pad;
	while (1) {
		sceClibMemset(&pad, 0, sizeof(pad));
		sceCtrlPeekBufferPositive(0, &pad, 1);
		unsigned newb = prev ^ (pad.buttons & prev);
		prev = pad.buttons;
		for (int i = 0; i < sizeof(buttons)/sizeof(*buttons); i++) {
			if (newb & buttons[i]) {
				return buttons[i];
			}
		}

		sceKernelDelayThread(1000);  // 1ms
	}
}

void draw_title_line(const char *menu_title)
{
	printf("\e[0m\e[H\e[2J");
	printf("\e[37;44m\e[2K" VITA_APP_NAME " v" VITA_VERSION " - %s" "\e[0m\n", menu_title);

	return;
}

void draw_pixel_line(int *return_x, int *return_y)
{
	int x, y;

	// draw pixel line
	psvDebugScreenGetCoordsXY(NULL, &y);
	x = 0; y += 3;
	psvDebugScreenSetCoordsXY(&x, &y);
	//
	x = psv_font_current->size_h;
	psv_font_current->size_h = 1;
	printf("\e[7m\e[2K\e[27m");
	psv_font_current->size_h = x;
	//
	x = 0; y += 3;
	psvDebugScreenSetCoordsXY(&x, &y);

	if (return_x != NULL) {
		*return_x = x;
	}
	if (return_y != NULL) {
		*return_y = y;
	}
	return;
}

void delete_app_save_data(void)
{
	//remove the save data created at application start
	sceIoRemove("savedata0:/sce_sys/keystone");
	sceIoRemove("savedata0:/sce_sys/param.sfo");
	sceIoRemove("savedata0:/sce_sys/safemem.dat");
	sceIoRemove("savedata0:/sce_sys/_safemem.dat");
	sceIoRemove("savedata0:/sce_sys/sdslot.dat");
	sceIoRemove("savedata0:/sce_sys/sealedkey");
}

void wait_for_cancel_button(void)
{
	bool menu_run;
	int button_pressed;

	menu_run = true;
	do {
		button_pressed = get_key();
		if (button_pressed == button_cancel) {
			menu_run = false;
		}
	} while (menu_run);
}

int main(void)
{
	bool is_safe_mode;
	bool menu_redraw;
	bool menu_run;
	int menu_items;
	int menu_item;
	bool no_user;
	int x, y;
	int button_pressed;
	int i;
	bool reboot;
	//bool extra = false;

	struct Registry_Data initial_reg_user_data;
	struct Registry_Data current_reg_user_data;

	struct File_Data initial_file_user_data;
	struct File_Data current_file_user_data;

	reboot = false;

	// initialize DebugScreen
	psvDebugScreenInit();
	// use a scaled by 2 version of the default font
	psv_font_default_1x = psvDebugScreenGetFont();
	psv_font_default_2x = psvDebugScreenScaleFont2x(psv_font_default_1x);
	if (psv_font_default_2x == NULL) {
		// scaled font was not created
		// TODO: error message check font data
		psv_font_default_2x = psv_font_default_1x;
	} else {
		// set scaled default font
		psv_font_current = psvDebugScreenSetFont(psv_font_default_2x);
		if (psv_font_current != psv_font_default_2x) {
			// font was not set
			// TODO: error message check font data
		}
	}

	// remove save data for this app
	delete_app_save_data();

	// determine enter button
	determine_enter_cancel_button(&button_enter, &button_cancel);

	// Check for homebrew safe mode (adapted from VitaShell)
	is_safe_mode = false;
	if (sceIoDevctl("ux0:", 0x3001, NULL, 0, NULL, 0) == 0x80010030) {
		is_safe_mode = true;
	}

	// initialize account data variables and structures
	if (!is_safe_mode) {
		main_account();

		// initialize registry initial data
		init_reg_data(&initial_reg_user_data);
		get_initial_reg_data(&initial_reg_user_data);

		// initialize registry user data
		init_reg_data(&current_reg_user_data);

		// initialize file initial data
		init_file_data(&initial_file_user_data);

		// initialize file user data
		init_file_data(&current_file_user_data);
	}

	// run main menu
	menu_redraw = true;
	menu_run = true;
	menu_items = 0;
	menu_item = 0;
	no_user = true;
	do {
		// redraw complete screen
		if (menu_redraw) {
			// draw title line
			draw_title_line("Main Menu");

			// draw pixel line
			draw_pixel_line(NULL, NULL);

			if (is_safe_mode) {
				printf("\e[1mPlease enable unsafe homebrew in Henkaku settings.\e[22m\e[0K\n");
			} else {
				get_current_reg_data(&current_reg_user_data);
				get_current_file_data(&current_file_user_data);
				get_current_execution_history_data(&execution_history_data);

				// draw current account data
				display_account_details_short(&current_reg_user_data, &no_user);

				// draw pixel line
				draw_pixel_line(NULL, NULL);
			}

			// draw menu
			printf("Menu:\e[0K\n");
			psvDebugScreenGetCoordsXY(NULL, &y);
			x = 0;
			menu_items = 0;
			if (!is_safe_mode) {
				// account menu points
				printf(" Display current account details.\e[0K\n"); menu_items++;
				printf(" Display initial account details.\e[0K\n"); menu_items++;
				if (no_user) { printf("\e[2m"); }
				printf(" Save current account.\e[0K\n"); menu_items++;
				if (no_user) { printf("\e[22m"); }
				printf(" Switch to a saved account.\e[0K\n"); menu_items++;
				printf(" Remove current account.\e[0K\n"); menu_items++;
				// execution history menu points
				printf(" Display current execution history details.\e[0K\n"); menu_items++;
				printf(" Delete current execution history.\e[0K\n"); menu_items++;
				if (execution_history_data.count_protected == execution_history_data.count) { printf("\e[2m"); }
				printf(" Write-protect execution history files.\e[0K\n"); menu_items++;
				if (execution_history_data.count_protected == execution_history_data.count) { printf("\e[22m"); }
				if (execution_history_data.count_protected == 0) { printf("\e[2m"); }
				printf(" Unprotect execution history files.\e[0K\n"); menu_items++;
				if (execution_history_data.count_protected == 0) { printf("\e[22m"); }
				printf(" Save console details.\e[0K\n"); menu_items++;
			}
			// last menu item is always exit
			if (reboot) {
				printf(" Exit and reboot, so changes take effect.");
			} else {
				printf(" Exit.");
			}
			printf("\e[0K\n");

			menu_redraw = false;
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
		if ((menu_item == 2) && no_user) {
			menu_item--;
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
			if ((menu_item == 2) && no_user) {
				menu_item++;
			}
		} else if (button_pressed == SCE_CTRL_UP) {
			menu_item--;
			if ((menu_item == 2) && no_user) {
				menu_item--;
			}
		//} else if (button_pressed == button_cancel) {
		//	extra = !extra;
		//	menu_redraw = true;
		//}
		} else if (button_pressed == button_enter) {
			if (menu_item == menu_items) {  // last menu item is always exit
				menu_run = false;
			} else if (menu_item == 0) {  // display account details
				display_account_details_full(&current_reg_user_data, &current_file_user_data, "Current Account Details");
				menu_redraw = true;
			} else if (menu_item == 1) {  // display initial account details
				display_account_details_full(&initial_reg_user_data, &initial_file_user_data, "Initial Account Details");
				menu_redraw = true;
			} else if (menu_item == 2) {  // save current account data
				if (!no_user) {
					save_account_details(&current_reg_user_data, &current_file_user_data, "Saving Current Account");
					menu_redraw = true;
				}
			} else if (menu_item == 3) {  // switch account
				if (switch_account(&current_reg_user_data, &initial_reg_user_data, &initial_file_user_data, "Switch Account")) {
					reboot = true;
				}
				menu_redraw = true;
			} else if(menu_item == 4) {  // remove current account
				if (remove_account(&current_reg_user_data, &initial_reg_user_data, &initial_file_user_data, "Removing Current Account")) {
					reboot = true;
				}
				menu_redraw = true;
			} else if (menu_item == 5) {  // display execution history details
				display_execution_history_details(&execution_history_data, "Current Execution History");
				menu_redraw = true;
			} else if (menu_item == 6) {  // delete execution history files
				delete_execution_history(&execution_history_data, "Delete Execution History");
				reboot = true;
				menu_redraw = true;
			} else if (menu_item == 7) {  // write-protect execution history files
				if (execution_history_data.count_protected < execution_history_data.count) {
					protect_execution_history_files(&execution_history_data, "Protect Execution History");
					menu_redraw = true;
				}
			} else if (menu_item == 8) {  // unprotect execution history files
				if (execution_history_data.count_protected > 0) {
					unprotect_execution_history_files(&execution_history_data, "Unprotect Execution History");
					menu_redraw = true;
				}
			} else if (menu_item == 9) {  // save console details
				save_console_details("Saving console details");
				menu_redraw = true;
			}
		}
	} while (menu_run);

	delete_app_save_data();

	if (reboot) {
		scePowerRequestColdReset();  // reboot
		//scePowerRequestStandby();  // shutdown
	}

	return 0;
}
