/*
  VitaShell
  Copyright (C) 2015-2018, TheFloW

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __FILE_H__
#define __FILE_H__

#define MAX_PATH_LENGTH 1024
#define TRANSFER_SIZE (128 * 1024)

int allocate_read_file(const char *file, void **buffer_ptr);
int read_file(const char *file, void *buf, int size);
int write_file(const char *file, const void *buf, int size);
int get_file_size(const char *file);
int check_file_exists(const char *file);
int check_folder_exists(const char *folder);
int copy_file(const char *src_path, const char *dst_path);

#endif  /* __FILE_H__ */
