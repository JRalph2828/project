/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2012 Brazil

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef GRN_STRING_H
#define GRN_STRING_H

#ifndef GRN_H
# include "grn.h"
#endif /* GRN_H */

#ifndef GRN_CTX_H
# include "grn_ctx.h"
#endif /* GRN_CTX_H */

#ifndef GRN_DB_H
# include "grn_db.h"
#endif /* GRN_DB_H */

#ifndef GRN_STR_H
# include "grn_str.h"
#endif /* GRN_STR_H */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  grn_obj_header header;
  const char *original;
  unsigned int original_length_in_bytes;
  char *normalized;
  unsigned int normalized_length_in_bytes;
  unsigned int n_characters;
  short *checks;
  unsigned char *ctypes;
  grn_encoding encoding;
  int flags;
} grn_string;

grn_obj *grn_string_open_(grn_ctx *ctx, const char *str, unsigned int str_len,
                          grn_obj *normalizer, int flags, grn_encoding encoding);
grn_rc grn_string_close(grn_ctx *ctx, grn_obj *string);
grn_rc grn_string_inspect(grn_ctx *ctx, grn_obj *buffer, grn_obj *string);

#ifdef __cplusplus
}
#endif

#endif /* GRN_STRING_H */
