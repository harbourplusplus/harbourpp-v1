//
// WINAPI for Harbour++ - Bindings libraries for Harbour++ and WINAPI
//
// Copyright (c) 2025 Marcos Antonio Gambeta <marcosgambeta AT outlook DOT com>
//

// MIT License
//
// Copyright (c) 2025 Marcos Antonio Gambeta
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// NOTE: source code generated with the help of a code generator

#include "hbclass.ch"

FUNCTION wasABC()
RETURN was_ABC():new()

CLASS WAS_ABC

   DATA ptr
   DATA self_destruction INIT .F.

   METHOD new
   METHOD delete

   // int abcA
   ASSIGN abcA(n) INLINE ::setabcA(n)
   ACCESS abcA INLINE ::getabcA()
   METHOD setabcA
   METHOD getabcA

   // UINT abcB
   ASSIGN abcB(n) INLINE ::setabcB(n)
   ACCESS abcB INLINE ::getabcB()
   METHOD setabcB
   METHOD getabcB

   // int abcC
   ASSIGN abcC(n) INLINE ::setabcC(n)
   ACCESS abcC INLINE ::getabcC()
   METHOD setabcC
   METHOD getabcC

   DESTRUCTOR destroyObject

END CLASS

PROCEDURE destroyObject() CLASS WAS_ABC
   IF ::self_destruction
      ::delete()
   ENDIF
RETURN

#pragma BEGINDUMP

#include <windows.h>
#include "hbapi.hpp"
#include "hbapiitm.hpp"
#include "hbapicls.hpp"
#include "winapi.hpp"

HB_FUNC_STATIC(WAS_ABC_NEW)
{
  auto self = hb_stackSelfItem();
  hb_objDataPutPtr(self, "_PTR", new ABC());
  hb_objDataPutL(self, "_SELF_DESTRUCTION", true);
  hb_itemReturn(self);
}

HB_FUNC_STATIC(WAS_ABC_DELETE)
{
  auto obj = static_cast<ABC *>(hb_objDataGetPtr(hb_stackSelfItem(), "PTR"));

  if (obj != nullptr)
  {
    delete obj;
    hb_objDataPutPtr(hb_stackSelfItem(), "_PTR", nullptr);
  }

  hb_itemReturn(hb_stackSelfItem());
}

// int abcA

HB_FUNC_STATIC(WAS_ABC_SETABCA)
{
  auto obj = static_cast<ABC *>(hb_objDataGetPtr(hb_stackSelfItem(), "PTR"));

  if (obj != nullptr)
  {
    obj->abcA = wa_par_int(1);
  }
}

HB_FUNC_STATIC(WAS_ABC_GETABCA)
{
  auto obj = static_cast<ABC *>(hb_objDataGetPtr(hb_stackSelfItem(), "PTR"));

  if (obj != nullptr)
  {
    wa_ret_int(obj->abcA);
  }
}

// UINT abcB

HB_FUNC_STATIC(WAS_ABC_SETABCB)
{
  auto obj = static_cast<ABC *>(hb_objDataGetPtr(hb_stackSelfItem(), "PTR"));

  if (obj != nullptr)
  {
    obj->abcB = wa_par_UINT(1);
  }
}

HB_FUNC_STATIC(WAS_ABC_GETABCB)
{
  auto obj = static_cast<ABC *>(hb_objDataGetPtr(hb_stackSelfItem(), "PTR"));

  if (obj != nullptr)
  {
    wa_ret_UINT(obj->abcB);
  }
}

// int abcC

HB_FUNC_STATIC(WAS_ABC_SETABCC)
{
  auto obj = static_cast<ABC *>(hb_objDataGetPtr(hb_stackSelfItem(), "PTR"));

  if (obj != nullptr)
  {
    obj->abcC = wa_par_int(1);
  }
}

HB_FUNC_STATIC(WAS_ABC_GETABCC)
{
  auto obj = static_cast<ABC *>(hb_objDataGetPtr(hb_stackSelfItem(), "PTR"));

  if (obj != nullptr)
  {
    wa_ret_int(obj->abcC);
  }
}

/*
typedef struct _ABC {
  int  abcA;
  UINT abcB;
  int  abcC;
} ABC, *PABC, *NPABC, *LPABC;
*/

#pragma ENDDUMP
