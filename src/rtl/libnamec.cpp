//
// hb_libExt(), hb_libPrefix()
//
// Copyright 2010 Viktor Szakats (vszakats.net/harbour)
//

// $HB_BEGIN_LICENSE$
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file LICENSE.txt.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301 USA (or visit https://www.gnu.org/licenses/).
//
// As a special exception, the Harbour Project gives permission for
// additional uses of the text contained in its release of Harbour.
//
// The exception is that, if you link the Harbour libraries with other
// files to produce an executable, this does not by itself cause the
// resulting executable to be covered by the GNU General Public License.
// Your use of that executable is in no way restricted on account of
// linking the Harbour library code into it.
//
// This exception does not however invalidate any other reasons why
// the executable file might be covered by the GNU General Public License.
//
// This exception applies only to the code released by the Harbour
// Project under the name Harbour.  If you copy code from other
// Harbour Project or Free Software Foundation releases into a copy of
// Harbour, as the General Public License permits, the exception does
// not apply to the code that you add in this way.  To avoid misleading
// anyone as to the status of such modified files, you must delete
// this exception notice from them.
//
// If you write modifications of your own for Harbour, it is your choice
// whether to permit this exception to apply to your modifications.
// If you do not wish that, delete this exception notice.
// $HB_END_LICENSE$

#include "hbapi.hpp"

HB_FUNC(HB_LIBEXT)
{
#if defined(HB_OS_WIN)
  hb_retc_const(".dll");
#elif defined(HB_OS_DARWIN)
  hb_retc_const(".dylib");
#elif defined(HB_OS_HPUX)
  hb_retc_const(".sl");
#else
  hb_retc_const(".so");
#endif
}

HB_FUNC(HB_LIBPREFIX)
{
#if !defined(HB_OS_UNIX)
  hb_retc_null();
#else
  hb_retc_const("lib");
#endif
}

#if defined(HB_OS_WIN)
#define HB_DLL_VER "-" HB_MACRO2STRING(HB_VER_MAJOR) HB_MACRO2STRING(HB_VER_MINOR)
#else
#define HB_DLL_VER ""
#endif

#if defined(__BORLANDC__)
#define HB_DLL_SUFFIX HB_DLL_VER "-bcc"
#elif defined(HB_OS_WIN_64) && defined(HB_CPU_X86_64)
#define HB_DLL_SUFFIX HB_DLL_VER "-x64"
#elif defined(HB_OS_WIN_64) && defined(HB_CPU_IA_64)
#define HB_DLL_SUFFIX HB_DLL_VER "-ia64"
#else
#define HB_DLL_SUFFIX HB_DLL_VER
#endif

HB_FUNC(HB_LIBSUFFIX)
{
  hb_retc_const(HB_DLL_SUFFIX);
}

HB_FUNC_TRANSLATE(HB_LIBPOSTFIX, HB_LIBSUFFIX) // Deprecated
