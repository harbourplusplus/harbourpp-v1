//
// AtAdjust() CT3 string function
//
// Copyright 2001 IntTec GmbH, Neunlindenstr 32, 79106 Freiburg, Germany
//        Author: Martin Vogel <vogel@inttec.de>
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

#include "ct.h"

HB_FUNC(ATADJUST)
{
  if (HB_ISCHAR(1) && HB_ISCHAR(2) && HB_ISNUM(3))
  {
    auto pcStringToMatch = hb_parc(1);
    auto sStrToMatchLen = hb_parclen(1);
    auto pcString = hb_parc(2);
    auto sStrLen = hb_parclen(2);
    HB_SIZE sAdjustPosition = hb_parns(3);

    int iMultiPass = ct_getatmupa();
    int iAtLike = ct_getatlike();
    char cAtLike = ct_getatlikechar();
    HB_SIZE sIgnore, sMatchStrLen = 0;
    HB_SIZE nCounter;
    const char *pc = nullptr;

    char cFillChar;
    char *pcRetStr;
    HB_SIZE sRetStrLen;

    // eventually ignore some characters
    sIgnore = hb_parnsdef(5, 0);

    if (sIgnore >= sStrLen)
    {
      int iArgErrorMode = ct_getargerrormode();

      if (iArgErrorMode != CT_ARGERR_IGNORE)
      {
        ct_error(static_cast<HB_USHORT>(iArgErrorMode), EG_ARG, CT_ERROR_ATADJUST, nullptr, HB_ERR_FUNCNAME, 0,
                 EF_CANDEFAULT, HB_ERR_ARGS_BASEPARAMS);
      }

      hb_retclen(pcString, sStrLen);
      return;
    }
    else
    {
      pcString += sIgnore;
      sStrLen -= sIgnore;
    }

    // check for wrong adjust position
    if (sAdjustPosition == 0)
    {
      int iArgErrorMode = ct_getargerrormode();

      if (iArgErrorMode != CT_ARGERR_IGNORE)
      {
        ct_error(static_cast<HB_USHORT>(iArgErrorMode), EG_ARG, CT_ERROR_ATADJUST, nullptr, HB_ERR_FUNCNAME, 0,
                 EF_CANDEFAULT, HB_ERR_ARGS_BASEPARAMS);
      }

      hb_retclen(pcString, sStrLen);
      return;
    }
    else
    {
      sAdjustPosition--; // makes live easier since C indices start at zero !
    }

    // nth match or last match ?
    if (HB_ISNUM(4) && (nCounter = hb_parns(4)) != 0)
    {
      // find the <nCounter>th match
      const char *pcSubStr;
      HB_SIZE sSubStrLen;
      HB_SIZE nMatchCounter = 0;

      pcSubStr = pcString;
      sSubStrLen = sStrLen;

      while (nMatchCounter < nCounter)
      {
        switch (iAtLike)
        {
        case CT_SETATLIKE_EXACT:
          pc = ct_at_exact_forward(pcSubStr, sSubStrLen, pcStringToMatch, sStrToMatchLen, &sMatchStrLen);
          break;

        case CT_SETATLIKE_WILDCARD:
          pc = ct_at_wildcard_forward(pcSubStr, sSubStrLen, pcStringToMatch, sStrToMatchLen, cAtLike, &sMatchStrLen);
          break;

        default:
          pc = nullptr;
        }

        if (pc == nullptr)
        {
          // no match found; if this happens at this point,
          // there are no <nCounter> matches, so return
          hb_retclen(pcString, sStrLen);
          return;
        }

        nMatchCounter++;
        if (iMultiPass)
        {
          pcSubStr = pc + 1;
        }
        else
        {
          pcSubStr = pc + sMatchStrLen;
        }
        sSubStrLen = sStrLen - (pcSubStr - pcString);
      }
    }
    else
    {
      // we have to find the last match
      switch (iAtLike)
      {
      case CT_SETATLIKE_EXACT:
        pc = ct_at_exact_backward(pcString, sStrLen, pcStringToMatch, sStrToMatchLen, &sMatchStrLen);
        break;

      case CT_SETATLIKE_WILDCARD:
        pc = ct_at_wildcard_backward(pcString, sStrLen, pcStringToMatch, sStrToMatchLen, cAtLike, &sMatchStrLen);
        break;

      default:
        pc = nullptr;
      }

      if (pc == nullptr)
      {
        // no matches found
        hb_retclen(pcString, sStrLen);
        return;
      }
    }

    // adjust string
    if (HB_ISCHAR(6))
    {
      if (hb_parclen(6) > 0)
      {
        cFillChar = *(hb_parc(6));
      }
      else
      {
        cFillChar = 0x20;
      }
    }
    else if (HB_ISNUM(6))
    {
      cFillChar = static_cast<char>(hb_parnl(6) % 256);
    }
    else
    {
      cFillChar = 0x20;
    }

    // position of pc == adjust position ?
    if (pc == pcString + sAdjustPosition)
    {
      // do nothing
      hb_retclen(pcString, sStrLen);
    }
    else
    {
      if (pc > pcString + sAdjustPosition)
      {
        // adjust to left
        // check if we only delete cFillChar characters
        for (const char *pcCheckFill = pcString + sAdjustPosition; pcCheckFill < pc; pcCheckFill++)
        {
          if (*pcCheckFill != cFillChar)
          {
            // no -> return string unchanged
            hb_retclen(pcString, sStrLen);
            return;
          }
        }

        // ok -> calculate new string size
        sRetStrLen = sStrLen - (pc - (pcString + sAdjustPosition));
        pcRetStr = static_cast<char *>(hb_xgrab(sRetStrLen + 1));

        // copy first portion of string
        if (sAdjustPosition > 0)
        {
          hb_xmemcpy(pcRetStr, pcString, sAdjustPosition);
        }

        // copy second portion of string
        if (sRetStrLen > sAdjustPosition)
        {
          hb_xmemcpy(pcRetStr + sAdjustPosition, pc, sRetStrLen - sAdjustPosition);
        }

        hb_retclen_buffer(pcRetStr, sRetStrLen);
      }
      else
      {
        // adjust to right
        sRetStrLen = sStrLen + (pcString + sAdjustPosition) - pc;
        pcRetStr = static_cast<char *>(hb_xgrab(sRetStrLen + 1));

        // copy first portion of string
        if (pc > pcString)
        {
          hb_xmemcpy(pcRetStr, pcString, pc - pcString);
        }

        // fill characters
        hb_xmemset(pcRetStr + (pc - pcString), cFillChar, sAdjustPosition - (pc - pcString));

        // copy second portion of string
        if (sRetStrLen > sAdjustPosition)
        {
          hb_xmemcpy(pcRetStr + sAdjustPosition, pc, sRetStrLen - sAdjustPosition);
        }

        hb_retclen_buffer(pcRetStr, sRetStrLen);
      }
    }
  }
  else
  {
    PHB_ITEM pSubst = nullptr;
    int iArgErrorMode = ct_getargerrormode();

    if (iArgErrorMode != CT_ARGERR_IGNORE)
    {
      pSubst = ct_error_subst(static_cast<HB_USHORT>(iArgErrorMode), EG_ARG, CT_ERROR_ATADJUST, nullptr,
                              HB_ERR_FUNCNAME, 0, EF_CANSUBSTITUTE, HB_ERR_ARGS_BASEPARAMS);
    }

    if (pSubst != nullptr)
    {
      hb_itemReturnRelease(pSubst);
    }
    else if (HB_ISCHAR(2))
    {
      hb_retclen(hb_parc(2), hb_parclen(2));
    }
    else
    {
      hb_retc_null();
    }
  }
}
