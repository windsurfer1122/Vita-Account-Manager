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

#ifndef __WLAN_H__
#define __WLAN_H__

#include <registry.h>

#define MAX_WLAN 30

struct Wlan_Data {
	int wlan_found;
	struct Registry_Data *wlan_reg_data[(MAX_WLAN)];
};

void init_wlan_data(struct Wlan_Data *wlan_data);
void get_current_wlan_data(struct Wlan_Data *wlan_data);
void save_wlan_details(struct Wlan_Data *wlan_data, char *title);
void load_wlan_details(struct Wlan_Data *wlan_data, char *title);
void main_wlan(void);

#endif  /* __WLAN_H__ */
