#ifndef U32_TO_U8_H
#define U32_TO_U8_H

/* Convert UTF-32 string to UTF-8 string.
   Copyright (C) 2002, 2006-2007, 2009-2026 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2002.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include "gnu_def.h"
#include "u8_uctomb.h"

/* Specification.  */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

uint8_t *
u32_to_u8 (const uint32_t *s, size_t n, uint8_t *resultbuf, size_t *lengthp)
{
  const uint32_t *s_end = s + n;

  /* Output string accumulator.  */
  uint8_t *result;
  size_t allocated;
  if (resultbuf != NULL)
    {
      result = resultbuf;
      allocated = *lengthp;
    }
  else
    {
      result = NULL;
      allocated = 0;
    }
  size_t length = 0;
  /* Invariants:
     result is either == resultbuf or == NULL or malloc-allocated.
     If length > 0, then result != NULL.  */

  while (s < s_end)
    {
      /* Fetch a Unicode character from the input string.  */
      ucs4_t uc = *s++;
      /* No need to call the safe variant u32_mbtouc, because
         u8_uctomb will verify uc anyway.  */

      /* Store it in the output string.  */
      int count = u8_uctomb (result + length, uc, allocated - length);
      if (count == -1)
        {
          if (!(result == resultbuf || result == NULL))
            free (result);
          errno = EILSEQ;
          return NULL;
        }
      if (count == -2)
        {
          allocated = (allocated > 0 ? 2 * allocated : 12);
          if (length + 6 > allocated)
            allocated = length + 6;

          uint8_t *memory;
          if (result == resultbuf || result == NULL)
            memory = (uint8_t *) malloc (allocated * sizeof (uint8_t));
          else
            memory =
              (uint8_t *) realloc (result, allocated * sizeof (uint8_t));

          if (memory == NULL)
            {
              if (!(result == resultbuf || result == NULL))
                free (result);
              errno = ENOMEM;
              return NULL;
            }
          if (result == resultbuf && length > 0)
            memcpy ((char *) memory, (char *) result,
                    length * sizeof (uint8_t));
          result = memory;
          count = u8_uctomb (result + length, uc, allocated - length);
          if (count < 0)
            abort ();
        }
      length += count;
    }

  if (length == 0)
    {
      if (result == NULL)
        {
          /* Return a non-NULL value.  NULL means error.  */
          result = (uint8_t *) malloc (1);
          if (result == NULL)
            {
              errno = ENOMEM;
              return NULL;
            }
        }
    }
  else if (result != resultbuf && length < allocated)
    {
      /* Shrink the allocated memory if possible.  */
      uint8_t *memory =
        (uint8_t *) realloc (result, length * sizeof (uint8_t));
      if (memory != NULL)
        result = memory;
    }

  *lengthp = length;
  return result;
}
#endif