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

#include "huf.h"

#define HUFCHARBITS 8 /* bits per unsigned char */

/*
 * https://en.wikipedia.org/wiki/Huffman_coding
 * https://en.wikipedia.org/wiki/Canonical_Huffman_code
 * a symbol is an unsigned char, 1 << HUFCHARBITS of them
 * the longest encoded value can be, up to, (1 << HUFCHARBITS) - 1 bits long
 */

/* the header of an encoded buffer is:
 *   octet of the number of octets of big endian original input length
 *     octet msb of original length
 *     ...
 *     octet lsb of original length
 *   octet number of bits/symbols sections (the maximum number of encoded bits)
 *     when 0, the remaining bytes are not encoded
 *   octet number of symbols for 1 bit
 *     when 1, the symbol is repeated for length (run length encoded)
 *   octet symbols for 1 bit...
 *   octet number of symbols for 2 bit
 *   octet symbols for 2 bit...
 *   ...
 * the body
 *   encoded data
 */

hufLen
hufEncode(
  unsigned char *out
 ,hufLen olen
 ,const unsigned char *in
 ,hufLen ilen
){
  hufLen l; /* output length */
  hufLen s[1 << HUFCHARBITS]; /* symbol count/code */
  hufLen k; /* working hufLen */
  unsigned int i; /* working unsigned int */
  unsigned int j; /* working unsigned int */
  struct node {
    hufLen c;        /* count */
    unsigned char f; /* flag 0=bothNode 1=leftSym 2=rightSym 3=bothSym */
    unsigned char p; /* parent node */
    unsigned char l; /* left */
    unsigned char r; /* right */
  } n[1 << HUFCHARBITS]; /* circular list of nodes */
  unsigned char b[1 << HUFCHARBITS]; /* symbol bits */
  unsigned char h; /* head node */
  unsigned char t; /* tail node */
  unsigned char m; /* min bits */
  unsigned char x; /* max bits */

  l = 0;
  if (!ilen)
    return (l);

  /* output length */
  for (i = sizeof (ilen); i && !((ilen >> ((i - 1) * HUFCHARBITS)) & 0xff); --i);
  ++l;
  if (olen)
    --olen, *out++ = i;
  while (i--) {
    ++l;
    if (olen)
      --olen, *out++ = (ilen >> (i * HUFCHARBITS)) & 0xff;
  }

  /* zero counts */
  for (i = 0; i < 1 << HUFCHARBITS; ++i)
    s[i] = b[i] = 0;

  /* count symbols */
  for (k = 0; k < ilen; ++k)
    ++s[*(in + k)];

  /* count non-zero symbols */
  for (i = j = 0; i < sizeof (s) / sizeof (s[0]); ++i)
    if (s[i]) {
      ++j;
      h = i;
    }
  /* if only one symbol, output one and the symbol */
  if (j == 1) {
    ++l;
    if (olen)
      --olen, *out++ = 1;
    ++l;
    if (olen)
      --olen, *out++ = h;
    return (l);
  }
  /* if input smaller than the smallest possible encoding (one bit per CHAR), output zero and the input */
  if (ilen <= l + 2 + j + ilen / HUFCHARBITS) {
noEncode:
    ++l;
    if (olen)
      --olen, *out++ = 0;
    for (; ilen; --ilen) {
      ++l;
      if (olen)
        --olen, *out++ = *in++;
    }
    return (l);
  }

  /* build huffman tree */
  h = t = 0; /* initialize node list head and tail */
  for (;;) {

    /* find least count */
    for (k = ilen, i = 0; i < sizeof (s) / sizeof (s[0]); ++i)
      if (s[i] && s[i] < k) {
        k = s[i];
        j = i;
      }
    /* "first" queue is smallest breaking ties in favor of first queue */
    if (k < ilen && (h == t || k <= n[h].c)) {
      n[t].c = k;
      n[t].f = 1; /* leftSym */
      n[t].l = j;
      s[j] = 0; /* take off "first" queue */
    } else if (h == t) /* one node left */
      break;
    else {
      n[t].c = n[h].c;
      n[t].f = 0; /* node */
      n[t].l = h;
      n[h].p = t;
      if (h == sizeof (n) / sizeof (n[0]) - 1)
        h = 0;
      else
        ++h;
    }

    /* if the previous least count was used */
    if (n[t].f) {
      /* find new least count */
      for (k = ilen, i = 0; i < sizeof (s) / sizeof (s[0]); ++i)
        if (s[i] && s[i] < k) {
          k = s[i];
          j = i;
        }
    }
    /* "first" queue is smallest breaking ties in favor of first queue */
    if (k < ilen && (h == t || k <= n[h].c)) {
      n[t].c += k;
      n[t].f |= 2; /* rightSym */
      n[t].r = j;
      s[j] = 0; /* take off "first" queue */
    } else if (h == t) /* one node left */
      break;
    else {
      n[t].c += n[h].c;
      n[t].r = h;
      n[h].p = t;
      if (h == sizeof (n) / sizeof (n[0]) - 1)
        h = 0;
      else
        ++h;
    }

    /* to next tail */
    if (t == sizeof (n) / sizeof (n[0]) - 1)
      t = 0;
    else
      ++t;
  }

  /* root is the previous tail */
  if (!t)
    t = sizeof (n) / sizeof (n[0]) - 1;
  else
    --t;
  n[t].p = t; /* root's parent is itself */

  /* generate symbol encoded bit lengths */
  i = t; /* start with root */
  j = 1; /* at least one bit */
  m = ~0; /* maximum minimum */
  x = 0; /* minimum maximum */
  for (;;) {
    struct node *p;

    p = n + i;
    if (p->c > 1) { /* haven't visited left */
      p->c = 0; /* visited left */
      if (p->f & 1) {
        if (j < m) m = j; /* min */
        if (j > x) x = j; /* max */
        b[p->l] = j;
      } else {
        i = p->l;
        ++j;
        continue;
      }
    }
    if (!p->c) { /* visited left not right */
      p->c = 1; /* visited right */
      if (p->f & 2) {
        if (j < m) m = j; /* min */
        if (j > x) x = j; /* max */
        b[p->r] = j;
      } else {
        i = p->r;
        ++j;
        continue;
      }
    }
    if (!--j)
      break;
    i = p->p;
  }
  /* if minimum bits is HUFCHARBITS, don't encode */
  if (m >= HUFCHARBITS)
    goto noEncode;

  /* compute canonical huffman code */
  k = 0;
  for (i = m; i <= x; ++i) {
    for (j = 0; j < sizeof (b) / sizeof (b[0]); ++j)
      if (b[j] && b[j] == i)
        s[j] = k++;
    k <<= 1;
  }

  { /* if encoding output becomes larger than the input, don't encode */
  unsigned char *p;
  hufLen o;

  /* for rewinding output */
  h = l;
  p = out;
  o = olen;

  /* output max bits */
  ++l;
  if (o)
    --o, *p++ = x;

  /* output bit counts */
  for (i = 1; i <= x; ++i) {
    for (j = m = 0; j < sizeof (b) / sizeof (b[0]); ++j)
      if (b[j] && b[j] == i)
        ++m;
    if (++l >= ilen) {
      l = h;
      goto noEncode;
    }
    if (o)
      --o, *p++ = m;

    /* output symbols */
    for (j = 0; j < sizeof (b) / sizeof (b[0]); ++j)
      if (b[j] && b[j] == i) {
        if (++l >= ilen) {
          l = h;
          goto noEncode;
        }
        if (o)
          --o, *p++ = j;
      }
  }

  /* encode */
  for (k = i = 0; k < ilen; ++k) {
    if (i) {
      i += b[*(in + k)];
      if (i < HUFCHARBITS) {
        if (o)
          *p |= s[*(in + k)] << (HUFCHARBITS - i) & ((1 << HUFCHARBITS) - 1);
        continue;
      }
      i -= HUFCHARBITS;
      if (o)
        --o, *p++ |= s[*(in + k)] >> i & ((1 << HUFCHARBITS) - 1);
    } else
      i = b[*(in + k)];
    for (; i >= HUFCHARBITS; i -= HUFCHARBITS) {
      if (++l >= ilen) {
        l = h;
        goto noEncode;
      }
      if (o)
        --o, *p++ = s[*(in + k)] >> (i - HUFCHARBITS) & ((1 << HUFCHARBITS) - 1);
    }
    if (i) {
      if (++l >= ilen) {
        l = h;
        goto noEncode;
      }
      if (o)
        *p = s[*(in + k)] << (HUFCHARBITS - i) & ((1 << HUFCHARBITS) - 1);
    }
  }
  }

  return (l);
}

hufLen
hufDecode(
  unsigned char *out
 ,hufLen olen
 ,const unsigned char *in
 ,hufLen ilen
){
  hufLen l;
  hufLen o;
  hufLen k;
  unsigned int i;
  unsigned int j;
  unsigned char b[1 << HUFCHARBITS]; /* symbol bits */
  struct table {
    hufLen f;        /* first code */
    hufLen o;        /* offset code */
    unsigned char b; /* bits */
  } t[1 << HUFCHARBITS];
  unsigned char m;
  unsigned char s[1 << HUFCHARBITS]; /* symbol */
  unsigned char n;
  unsigned char c;

  l = 0;
  if (!ilen-- || !(c = *in++))
    return (l);

  /* get original buffer length */
  for (o = 0; ilen && c; --ilen, --c) {
    o <<= HUFCHARBITS;
    o |= *in++;
  }
  if (!ilen--)
    return (l);

  /* get number of bits/symbols */
  if (!(c = *in++)) { /* noEncode */
    while (ilen-- && o--) {
      ++l;
      if (olen)
        --olen, *out++ = *in++;
    }
    return (l);
  }
  if (c == 1) { /* single symbol */
    if (!ilen--)
      return (l);
    c = *in;
    while (o--) {
      ++l;
      if (olen)
        --olen, *out++ = c;
    }
    return (l);
  }

  /* zero bits */
  for (i = 0; i < sizeof (b) / sizeof (b[0]); ++i)
    b[i] = 0;

  /* get bits for symbols */
  for (i = 1; i <= c; ++i) {
    if (!ilen--)
      return (l);
    for (j = *in++; j; --j) {
      /* get symbol */
      if (!ilen--)
        return (l);
      b[*in++] = i;
    }
  }

  /* zero table */
  for (i = 0; i < sizeof (t) / sizeof (t[0]); ++i)
    t[i].b = 0;

  /* compute canonical code, decompression table and symbol table */
  m = n = 0;
  k = 0;
  for (i = 1; i <= c; ++i) {
    for (j = 0; j < sizeof (b) / sizeof (b[0]); ++j)
      if (b[j] && b[j] == i) {
        if (!t[m].b) {
          t[m].f = k;
          t[m].o = k - n;
          t[m].b = i;
        }
        ++k;
        s[n++] = j;
      }
    k <<= 1;
    if (t[m].b)
      ++m;
  }

  /* decode */
  c = m; /* number of t */
  m = n = 0; /* bits of k from msb, bits of *in from lsb */
  k = 0;
  while (o--) {
    while (m < sizeof (k) * HUFCHARBITS) { /* fill the table search value */
      if (HUFCHARBITS <= sizeof (k) * HUFCHARBITS - m) { /* left shift */
        if (n) {
          k |= (*in++ << (HUFCHARBITS - n)) << ((sizeof (k) - 1) * HUFCHARBITS - m);
          m += n;
          n = 0;
        } else if (!ilen)
          break;
        else {
          m += HUFCHARBITS;
          k |= *in++ << (sizeof (k) * HUFCHARBITS - m);
          --ilen;
        }
      } else { /* right shift */
        if (n > (i = sizeof (k) * HUFCHARBITS - m)) {
          k |= (*in << (HUFCHARBITS - n)) >> (HUFCHARBITS - i);
          m += i;
          n -= i;
        } else if (n) {
          i = sizeof (k) * HUFCHARBITS - m;
          k |= (*in++ << (HUFCHARBITS - n)) >> (HUFCHARBITS - i);
          m += n;
          n = 0;
        } else if (!ilen)
          break;
        else {
          n = HUFCHARBITS - (sizeof (k) * HUFCHARBITS - m);
          k |= *in >> n;
          m = sizeof (k) * HUFCHARBITS;
          --ilen;
        }
      }
    }
    /* search the table */
    for (i = 0; i < c && t[i].f <= k >> (sizeof (k) * HUFCHARBITS - t[i].b); ++i);
    /* output the decoded symbol */
    --i;
    ++l;
    if (olen)
      --olen, *out++ = s[(k >> (sizeof (k) * HUFCHARBITS - t[i].b)) - t[i].o];
    k <<= t[i].b;
    m -= t[i].b;
  }
  return (l);
}
