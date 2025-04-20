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

/* typedef can be changed if needed (e.g. unsigned short, unsigned long, etc.) */
typedef unsigned int hufLen;

/* return 0 on error else the length of out */
/* if length returned is more than length provided, allocate needed memory and retry */
hufLen rleEncode(
  unsigned char *out
 ,hufLen olen
 ,const unsigned char *in
 ,hufLen ilen
);

/* return 0 on error else the length of out */
/* if length returned is more than length provided, allocate needed memory and retry */
hufLen rleDecode(
  unsigned char *out
 ,hufLen olen
 ,const unsigned char *in
 ,hufLen ilen
);

/* return 0 on error else the length of out */
/* if length returned is more than length provided, allocate needed memory and retry */
hufLen hufEncode(
  unsigned char *out
 ,hufLen olen
 ,const unsigned char *in
 ,hufLen ilen
);

/* return 0 on error else the length of out */
/* if length returned is more than length provided, allocate needed memory and retry */
hufLen hufDecode(
  unsigned char *out
 ,hufLen olen
 ,const unsigned char *in
 ,hufLen ilen
);
