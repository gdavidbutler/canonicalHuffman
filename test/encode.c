/*
 * canonicalHuffman - A C language implementation of a Canonical Huffman Encoder and Decoder pair
 * Copyright (C) 2015-2023 G. David Butler <gdb@dbSystems.com>
 *
 * This file is part of canonicalHuffman
 *
 * canonicalHuffman is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * canonicalHuffman is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "huf.h"

int
main(
 int argc
,char *argv[]
){
  int fd;
  unsigned long oz;
  unsigned long sz;
  unsigned char *ib;
  unsigned char *ob;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s file\n", argv[0]);
    return (1);
  }
  if ((fd = open(argv[1], O_RDONLY)) < 0) {
    fprintf(stderr, "%s: Can't open %s\n", argv[0], argv[1]);
    return (1);
  }
  oz = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  if (!(ib = malloc(oz))
   || !(ob = malloc(oz * 2))) {
    fprintf(stderr, "%s: malloc\n", argv[0]);
    return (1);
  }
  if (read(fd, ib, oz) != (ssize_t)oz) {
    fprintf(stderr, "%s: read fail on %s\n", argv[0], argv[1]);
    return (1);
  }
  close(fd);
  sz = hufEncode(ob, oz * 2, ib, oz);
  free(ib);
  if (fwrite(ob, 1, sz, stdout) != sz) {
    fprintf(stderr, "%s: write fail\n", argv[0]);
    return (1);
  }
  free(ob);
  free(ib);
  return (0);
}
