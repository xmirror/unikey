// -*- coding:unix -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2004 Pham Kim Long
 * Contact:
 *   longcz@yahoo.com
 *   UniKey project: http://unikey.sf.net
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __UNIKEY_MACRO_H
#define __UNIKEY_MACRO_H

#if defined(__cplusplus)
extern "C" {
#endif

  int UkSetupMacro();
  void UkCleanupMacro();

  int UkLoadMacroTable(const char *fileName);
  void UkUpdateMacroTable(int charsetInUse);

  
#if defined(__cplusplus)
}
#endif

#endif
