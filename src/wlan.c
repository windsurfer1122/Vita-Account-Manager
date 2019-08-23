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

#include <main.h>
#include <file.h>
#include <registry.h>
#include <wlan.h>

#include <debugScreen.h>
#define printf psvDebugScreenPrintf

const char *const wlan_folder = "wlans/";
const char *const reg_config_net_nn = "/CONFIG/NET/%02i";  // decimal 01-30
const char *const reg_config_net_eth = "/CONFIG/NET/ETH";
const char *const reg_config_net_app = "APP";
const char *const reg_config_net_common = "COMMON";
const char *const reg_config_net_ip = "IP";
const char *const reg_config_net_wifi = "WIFI";
const char *const file_reg_config_net = "registry/CONFIG/NET/";
const int reg_id_ssid = 217;

// values from os0:kd/registry.db0 and https://github.com/devnoname120/RegistryEditorMOD/blob/master/regs.c
struct Registry_Entry template_wlan_reg_entries[] = {
	{ reg_id_ssid, reg_config_net_nn, file_reg_config_net, reg_config_net_wifi, "ssid", KEY_TYPE_STR, 33, NULL, },
	{ 219, reg_config_net_nn, file_reg_config_net, reg_config_net_wifi, "wep_key", KEY_TYPE_STR, 27, NULL, },
	{ 218, reg_config_net_nn, file_reg_config_net, reg_config_net_wifi, "wifi_security", KEY_TYPE_INT, 4, NULL, },
	{ 220, reg_config_net_nn, file_reg_config_net, reg_config_net_wifi, "wpa_key", KEY_TYPE_STR, 65, NULL, },
	{ 233, reg_config_net_nn, file_reg_config_net, reg_config_net_app, "http_proxy_flag", KEY_TYPE_INT, 4, NULL, },
	{ 235, reg_config_net_nn, file_reg_config_net, reg_config_net_app, "http_proxy_port", KEY_TYPE_INT, 4, NULL, },
	{ 234, reg_config_net_nn, file_reg_config_net, reg_config_net_app, "http_proxy_server", KEY_TYPE_STR, 256, NULL, },
	{ 208, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "conf_flag", KEY_TYPE_INT, 4, NULL, },
	{ 210, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "conf_name", KEY_TYPE_STR, 65, NULL, },
	{ 211, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "conf_serial_no", KEY_TYPE_INT, 4, NULL, },
	{ 209, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "conf_type", KEY_TYPE_INT, 4, NULL, },
	{ 214, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "device", KEY_TYPE_INT, 4, NULL, },
	{ 212, reg_config_net_nn, file_reg_config_net, reg_config_net_common, "enable_auto_connect", KEY_TYPE_INT, 4, NULL, },
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
};


void free_wlan_data(struct Wlan_Data *wlan_data)
{
	int i;
	struct Registry_Data *reg_data;

	// free memory of each reg data entry
	for (i = 0; i < wlan_data->wlan_count; i++) {
		reg_data = &(wlan_data->wlan_reg_data[i]);
		free_reg_data(reg_data);
		free(reg_data);
	}

	// free memory of reg data array
	free(wlan_data->wlan_reg_data);

	return;
}

void main_wlan(void)
{
	int i;

	// determine special indexes of registry data
	template_wlan_reg_data.idx_username = -1;
	template_wlan_reg_data.idx_login_id = -1;
	template_wlan_reg_data.idx_ssid = -1;
	for (i = 0; i < template_wlan_reg_data.reg_count; i++) {
		// ssid
		if ((template_wlan_reg_data.idx_ssid < 0) && (template_wlan_reg_data.reg_entries[i].key_id == reg_id_ssid)) {
			template_wlan_reg_data.idx_ssid = i;
		}
		// all found?
		if (template_wlan_reg_data.idx_ssid >= 0) {
			break;
		}
	}
}
