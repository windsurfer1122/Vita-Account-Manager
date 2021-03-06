# Vita Account Manager (c) 2019 by "windsurfer1122"
Switch between multiple PSN/SEN accounts on a PS Vita or PS TV.
Handle execution history.
Dump console IDs.
Save and restore WLAN settings.

## Features
* Save the current account for later restore.
* Restore a saved account or remove the current account.
  * Removes trophies of current account to reduce ban risk on an accidental trophy sync.
  * Unlink memory card from current account.
  * Delete execution history of current account to discard Homebrew title IDs in it.
    A reboot is still needed to clear execution history in memory.
  * Remove obligatory "save data" of Vita Account Manager to avoid trophy message after account switch.
* Delete execution history of current account to discard Homebrew title IDs in it before logging into PSN.
  A reboot is still needed to clear execution history in memory.
* Write-protect execution history files by replacing files with not overwritable directories.
  A reboot is still needed to clear execution history in memory.
* Save the IDPS and PSID of the console.
* Save WLAN settings and restore them.
* Uses an extandable coding framework for easy enhancements and/or extensions. Go fork and send [pull requests][11] and/or patches.

## Details
* Account data.
  * Data is stored at `ux0:data/ACTM00003/accounts/<username>/`.
  * Saved files.
    * `tm0:npdrm/act.dat` - PSV game activation data, stored under `tm0/npdrm/act.dat`
    * `tm0:psmdrm/act.dat` - PSM activation data, stored under `tm0/psmdrm/act.dat`
    * `ur0:user/00/np/myprofile.dat` - PSN avatar data, stored under `ur0/np/myprofile.dat`
  * Removed files.
    * `ur0:user/00/trophy/data/sce_trop/TRPUSER.DAT` - trophy data
    * `ur0:user/00/trophy/data/sce_trop/sce_pfs/files.db` - trophy data
    * `ux0:id.dat` - Link of memory card to account and firmware version
    * `imc0:id.dat` - Link of internal memory card to account and firmware version
    * `uma0:id.dat` - Link of additional memory card to account and firmware version
  * Saved registry entries.
    * Stored under `registry/` with their full entry path.
    * `/CONFIG/SYSTEM/username` - PSN Online ID, used as folder name to store data
    * `/CONFIG/NP/login_id` - E-mail address of account
    * `/CONFIG/NP/account_id` - Account ID (binary)
    * `/CONFIG/NP/password` - Password of account
    * `/CONFIG/NP/lang` - Language of account
    * `/CONFIG/NP/country` - Language of account
    * `/CONFIG/NP/yob` - Year of Birth of account
    * `/CONFIG/NP/mob` - Month of Birth of account
    * `/CONFIG/NP/dob` - Day of Birth of account
    * `/CONFIG/NP/has_subaccount`
    * `/CONFIG/NP/enable_np`
    * `/CONFIG/NP/download_confirmed`
    * `/CONFIG/NP/env`
* Execution history.
  1. Title IDs from *all* launched games/apps.
     * `ur0:ci/file/m.log` (pointer is *not* in `ur0:ci/file/m.cnt`)
     * `ur0:user/00/shell/playlog/playlod.dat`
     * `ur0:user/00/shell/playlog/playlog.dat`
     * `ur0:user/00/shell/playlog/playlog.dat.tmp`
  2. Error codes and title IDs from crashed games/apps.
     * `vd0:history/data.bak`
     * `vd0:history/data.bin`
  3. Reboot to also clear execution history in memory.
* Console data.
  * Data is stored at `ux0:data/ACTM00003/console/`.
    * `idps.bin` - IDPS of console
    * `psid.bin` - PSID of console
* WLAN data.
  * Data is stored at `ux0:data/ACTM00003/wlans/<ssid>/`.
  * Saved registry entries.
    * Stored under `registry/` with their full entry path, but excluding the NET count (01-30 decimal).
    * `/CONFIG/NET/WIFI/ssid` - WLAN SSID, used as folder name to store data
    * `/CONFIG/NET/WIFI/wep_key`
    * `/CONFIG/NET/WIFI/wifi_security`
    * `/CONFIG/NET/WIFI/wpa_key`
    * `/CONFIG/NET/APP/http_proxy_flag`
    * `/CONFIG/NET/APP/http_proxy_port`
    * `/CONFIG/NET/APP/http_proxy_server`
    * `/CONFIG/NET/COMMON/conf_flag`
    * `/CONFIG/NET/COMMON/conf_name` - same as WLAN SSID
    * `/CONFIG/NET/COMMON/conf_serial_no`
    * `/CONFIG/NET/COMMON/conf_type`
    * `/CONFIG/NET/COMMON/device`
    * `/CONFIG/NET/COMMON/enable_auto_connect`
    * `/CONFIG/NET/COMMON/ether_mode`
    * `/CONFIG/NET/COMMON/mtu`
    * `/CONFIG/NET/IP/auth_key`
    * `/CONFIG/NET/IP/auth_name`
    * `/CONFIG/NET/IP/default_route`
    * `/CONFIG/NET/IP/dhcp_hostname`
    * `/CONFIG/NET/IP/dns_flag`
    * `/CONFIG/NET/IP/ip_address`
    * `/CONFIG/NET/IP/ip_config`
    * `/CONFIG/NET/IP/netmask`
    * `/CONFIG/NET/IP/primary_dns`
    * `/CONFIG/NET/IP/secondary_dns`

## Converting Backups of Other Account Managers
* AccountManager by ONElua Team ([link][1])
  * Copy `ux0:pspemu/PSP/SAVEDATA/ACTM00001/<username>` to `ux0:data/ACTM00003/accounts/<username>`.
  * If `act.dat` exists then create account subdirectories and move file to `tm0/npdrm/act.dat`.
  * If `myprofile.dat` exists then create account subdirectories and move file to `ur0/np/myprofile.dat`.
  * Create account subdirectories `registry/CONFIG/NP` and create the following files:
    * *No* newlines in files.
    * `registry/CONFIG/NP/login_id.txt` with e-mail address of PSN/SEN account.
    * `registry/CONFIG/NP/password.txt` with password address of PSN/SEN account.
* Advanced-Account-Switcher by SiliciaAndPina ([link][2])
  * Copy `ux0:AdvancedAccountSwitcher/accounts/<username>` to `ux0:data/ACTM00003/accounts/<username>`
  * If `npdrm.dat` exists then create account subdirectories, move and rename file to `tm0/npdrm/act.dat`.
  * If `psmdrm.dat` exists then create account subdirectories, move and rename file to `tm0/psmdrm/act.dat`.
  * Create account subdirectories `registry/CONFIG/NP`, move and rename the following files:
    * `aid.bin` to `registry/CONFIG/NP/account_id.bin`.
    * `username.txt` to `registry/CONFIG/NP/login_id.txt`.
    * `password.txt` to `registry/CONFIG/NP/password.txt`.
  * If you have a separate backup of `myprofile.dat` then create account subdirectories and move file to `ur0/np/myprofile.dat`.

## Editing Data
* **!!! DO AT YOUR OWN RISK !!!**
* **!!! DO A BACKUP FIRST !!!**
* The saved registry data can be edited by text editors for integer and string values (e.g. [Notepad++][3]) and by hex editors for binary values (e.g. [HxD][4]).
  *No* newlines!
* For editing the registry directly the tool [Registry Editor MOD by devnoname120][5] is recommended as it also allows to edit binary registry entries.
* Console will retrieve/refresh account details from PSN when logging into PSN.

## Registry States during Activation on Vita 3.68
|Reg Path         |Reg Key                 |No user                    |User Added Only     |User Logged in    |Activated|DRM-Free Demo DL *1|
|-----------------|------------------------|---------------------------|--------------------|------------------|---------|-------------------|
|`/CONFIG/NP/`    |`account_id`            |(all hex zeroes)           |**(set from PSN)**  |-                 |-        |-                  |
|`/CONFIG/NP/`    |`country`               |**(set from intial setup)**|-                   |**(set from PSN)**|-        |-                  |
|`/CONFIG/NP/`    |`debug`                 |0                          |-                   |-                 |-        |-                  |
|`/CONFIG/NP/`    |`debug_ingame_commerce2`|0                          |-                   |-                 |-        |-                  |
|`/CONFIG/NP/`    |`dob`                   |**(set from intial setup)**|-                   |**(set from PSN)**|-        |-                  |
|`/CONFIG/NP/`    |`download_confirmed`    |0                          |-                   |-                 |-        |**1**              |
|`/CONFIG/NP/`    |`enable_np`             |0                          |-                   |-                 |-        |**1**              |
|`/CONFIG/NP/`    |`env`                   |**`np`**                   |-                   |-                 |-        |-                  |
|`/CONFIG/NP/`    |`has_subaccount`        |0                          |-                   |**(set from PSN)**|-        |-                  |
|`/CONFIG/NP/`    |`lang`                  |**(set from intial setup)**|-                   |**(set from PSN)**|-        |-                  |
|`/CONFIG/NP/`    |`login_id`              |(empty string)             |**(set in sign-up)**|-                 |-        |-                  |
|`/CONFIG/NP/`    |`mob`                   |**(set from intial setup)**|-                   |**(set from PSN)**|-        |-                  |
|`/CONFIG/NP/`    |`nav_only`              |0                          |-                   |-                 |-        |-                  |
|`/CONFIG/NP/`    |`np_ad_clock_diff`      |0                          |-                   |-                 |-        |-                  |
|`/CONFIG/NP/`    |`np_geo_filtering`      |0                          |-                   |-                 |-        |-                  |
|`/CONFIG/NP/`    |`password`              |(empty string)             |**(set in sign-up)**|-                 |-        |-                  |
|`/CONFIG/NP/`    |`yob`                   |**(set from intial setup)**|-                   |**(set from PSN)**|-        |-                  |
|`/CONFIG/SYSTEM/`|`username`              |**`user<NNN>`**            |-                   |-                 |-        |-                  |

*1: DRM-Free demo was EU PCSF00095 "ModNation Racers: Road Trip", just downloaded, not started.

## Credits for Ideas and Features
* ONElua Team for [AccountManager][1]
* SiliciaAndPina for [Advanced-Account-Switcher][2] and [SimpleAccountSwitcher][6]
* Chihiro/KanadeEngel for [History Deleter][7]
* Orion for [Vita Toolbox][8]
* NamelessGhoul0 for [History Wipe Plugin][9]
* TheFlow for [VitaShell][10]
* devnoname120 for [RegistryEditor MOD][5]

[1]: https://github.com/ONElua/AccountManager "AccountManager by ONElua Team"
[2]: https://bitbucket.org/SilicaAndPina/advanced-account-switcher/ "Advanced-Account-Switcher by SiliciaAndPina"
[3]: https://notepad-plus-plus.org/ "Notepad++ Text Editor"
[4]: https://mh-nexus.de/en/hxd/ "HxD Hex Editor"
[5]: https://github.com/devnoname120/RegistryEditorMOD "Registry Editor MOD by devnoname120"
[6]: https://bitbucket.org/SilicaAndPina/simpleaccountswitcher/ "SimpleAccountSwitcher by SiliciaAndPina"
[7]: https://github.com/KanadeEngel/PS-Vita/tree/master/history_deleter "History Deleter by Chihiro/KanadeEngel"
[8]: http://www.psx-place.com/threads/vita-toolbox-v-0-2-new-functions-added-by-orion.10457/ "Vita Toolbox by Orion"
[9]: https://github.com/NamelessGhoul0/history_wipe "History Wipe Plugin by NamelessGhoul0"
[10]: https://github.com/TheOfficialFloW/VitaShell "VitaShell by TheFlow"
[11]: https://guides.github.com/activities/hello-world/ "GitHub Guide"
