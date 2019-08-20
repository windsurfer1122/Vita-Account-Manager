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

#include <vitasdk.h>
#include <stdlib.h> // for malloc(), free()
#include <malloc.h> // for memalign()

#include <file.h>

int allocate_read_file(const char *file, void **buffer) {
  SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
  if (fd < 0)
    return fd;

  int size = sceIoLseek32(fd, 0, SCE_SEEK_END);
  sceIoLseek32(fd, 0, SCE_SEEK_SET);

  *buffer = malloc(size);
  if (!*buffer) {
    sceIoClose(fd);
    return -1;
  }

  int read = sceIoRead(fd, *buffer, size);
  sceIoClose(fd);

  return read;
}

int read_file(const char *file, void *buf, int size) {
  SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
  if (fd < 0)
    return fd;

  int read = sceIoRead(fd, buf, size);

  sceIoClose(fd);
  return read;
}

int write_file(const char *file, const void *buf, int size) {
  SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
  if (fd < 0)
    return fd;

  int written = sceIoWrite(fd, buf, size);

  sceIoClose(fd);
  return written;
}

int get_file_size(const char *file) {
  SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
  if (fd < 0)
    return fd;

  int fileSize = sceIoLseek(fd, 0, SCE_SEEK_END);

  sceIoClose(fd);
  return fileSize;
}

int check_file_exists(const char *file) {
  SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
  if (fd < 0)
    return 0;

  sceIoClose(fd);
  return 1;
}

int check_folder_exists(const char *folder) {
  SceUID dfd = sceIoDopen(folder);
  if (dfd < 0)
    return 0;

  sceIoDclose(dfd);
  return 1;
}

int copy_file(const char *src_path, const char *dst_path) {
  // The source and destination paths are identical
  if (strcasecmp(src_path, dst_path) == 0) {
    return -1;
  }

  // The destination is a subfolder of the source folder
  int len = strlen(src_path);
  if (strncasecmp(src_path, dst_path, len) == 0 && (dst_path[len] == '/' || dst_path[len - 1] == '/')) {
    return -2;
  }

  SceUID fdsrc = sceIoOpen(src_path, SCE_O_RDONLY, 0);
  if (fdsrc < 0)
    return fdsrc;

  SceUID fddst = sceIoOpen(dst_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
  if (fddst < 0) {
    sceIoClose(fdsrc);
    return fddst;
  }

  void *buf = memalign(4096, TRANSFER_SIZE);

  while (1) {
    int read = sceIoRead(fdsrc, buf, TRANSFER_SIZE);

    if (read < 0) {
      free(buf);

      sceIoClose(fddst);
      sceIoClose(fdsrc);

      sceIoRemove(dst_path);

      return read;
    }

    if (read == 0)
      break;

    int written = sceIoWrite(fddst, buf, read);

    if (written < 0) {
      free(buf);

      sceIoClose(fddst);
      sceIoClose(fdsrc);

      sceIoRemove(dst_path);

      return written;
    }
  }

  free(buf);

  // Inherit file stat
  SceIoStat stat;
  memset(&stat, 0, sizeof(SceIoStat));
  sceIoGetstatByFd(fdsrc, &stat);
  sceIoChstatByFd(fddst, &stat, 0x3B);

  sceIoClose(fddst);
  sceIoClose(fdsrc);

  return 1;
}
