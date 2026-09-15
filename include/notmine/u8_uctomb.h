#ifndef U8_UCTOMB_H
#define U8_UCTOMB_H
/* Store Unicode character in UTF-8 string.
   Copyright (C) 1999-2002, 2006-2007, 2009-2026 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2001.

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include "gnu_def.h"
/* Specification.  */

int
u8_uctomb (uint8_t *s, ucs4_t uc, int n)
{
  if (uc < 0x80)
    {
      if (n >= 1)
        {
          s[0] = uc;
          return 1;
        }
      else
        return -2;
    }
  else if (uc < 0x800)
    {
      if (n >= 2)
        {
          s[0] = 0xc0 | (uc >> 6);
          s[1] = 0x80 | (uc & 0x3f);
          return 2;
        }
      else
        return -2;
    }
  else if (uc < 0x10000)
    {
      if (n >= 3)
        {
          s[0] = 0xe0 | (uc >> 12);
          s[1] = 0x80 | ((uc >> 6) & 0x3f);
          s[2] = 0x80 | (uc & 0x3f);
          return 3;
        }
      else
        return -2;
    }
  else if (uc < 0x110000)
    {
      if (n >= 4)
        {
          s[0] = 0xf0 | (uc >> 18);
          s[1] = 0x80 | ((uc >> 12) & 0x3f);
          s[2] = 0x80 | ((uc >> 6) & 0x3f);
          s[3] = 0x80 | (uc & 0x3f);
          return 4;
        }
      else
        return -2;
    }
  else
    return -1;
}
#endif