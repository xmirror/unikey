/*------------------------------------------------------------------------------
VnConv: Vietnamese Encoding Converter Library
UniKey Project: http://unikey.sourceforge.net
Copyleft (C) 1998-2002 Pham Kim Long
Contact: longcz@yahoo.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
--------------------------------------------------------------------------------*/

#ifndef VIETNAMESE_CHARSET_DATA_H
#define VIETNAMESE_CHARSET_DATA_H

// This header defines some special characters
const StdVnChar StdStartQuote = (VnStdCharOffset + 201);  // 0x93 in the Western charset
// 201 is the offset of character 0x93 (start quote) in Vn charsets
const StdVnChar StdEndQuote = (VnStdCharOffset + 202); // 0x94 in the Western charset
const StdVnChar StdEllipsis = (VnStdCharOffset + 190); // 0x85 in Western charet.

#endif
