//
// StrTran() function
//
// Copyright 1999 Antonio Linares <alinares@fivetech.com>
// Copyright 2011 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
// rewritten to fix incompatibilities with Clipper and fatal performance
// of original code
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
#include "hbapiitm.hpp"
#include "hbapierr.hpp"

// FIXME: Check for string overflow, Clipper can crash if the resulting
//        string is too large. Example:
//        StrTran("...", ".", Replicate("A", 32000)) [vszakats]

HB_FUNC(STRTRAN)
{
  auto pText = hb_param(1, Harbour::Item::STRING);
  auto pSeek = hb_param(2, Harbour::Item::STRING);

  if (pText && pSeek)
  {
    HB_SIZE nStart, nCount;

    nStart = hb_parnsdef(4, 1);
    nCount = hb_parnsdef(5, -1);

    if (nStart && nCount)
    {
      auto nText = pText->getCLen();
      auto nSeek = pSeek->getCLen();

      if (nSeek && nSeek <= nText && nStart > 0)
      {
        auto pReplace = hb_param(3, Harbour::Item::STRING);
        auto nReplace = hb_itemGetCLen(pReplace);
        auto szReplace = hb_itemGetCPtr(pReplace);
        auto szText = pText->getCPtr();
        auto szSeek = pSeek->getCPtr();
        HB_SIZE nFound = 0;
        HB_SIZE nReplaced = 0;
        HB_SIZE nT = 0;
        HB_SIZE nS = 0;

        while (nT < nText && nText - nT >= nSeek - nS)
        {
          if (szText[nT] == szSeek[nS])
          {
            ++nT;
            if (++nS == nSeek)
            {
              if (++nFound >= nStart)
              {
                nReplaced++;
                if (--nCount == 0)
                {
                  nT = nText;
                }
              }
              nS = 0;
            }
          }
          else if (nS)
          {
            nT -= nS - 1;
            nS = 0;
          }
          else
          {
            ++nT;
          }
        }

        if (nReplaced)
        {
          HB_SIZE nLength = nText;

          if (nSeek > nReplace)
          {
            nLength -= (nSeek - nReplace) * nReplaced;
          }
          else
          {
            nLength += (nReplace - nSeek) * nReplaced;
          }

          if (nLength)
          {
            auto szResult = static_cast<char *>(hb_xgrab(nLength + 1));
            char *szPtr = szResult;

            nFound -= nReplaced;
            nT = nS = 0;
            do
            {
              if (nReplaced && szText[nT] == szSeek[nS])
              {
                ++nT;
                if (++nS == nSeek)
                {
                  const char *szCopy;

                  if (nFound)
                  {
                    nFound--;
                    szCopy = szSeek;
                  }
                  else
                  {
                    nReplaced--;
                    szCopy = szReplace;
                    nS = nReplace;
                  }
                  while (nS)
                  {
                    *szPtr++ = *szCopy++;
                    --nS;
                  }
                }
              }
              else
              {
                if (nS)
                {
                  nT -= nS;
                  nS = 0;
                }
                *szPtr++ = szText[nT++];
              }
            } while (nT < nText);

            hb_retclen_buffer(szResult, nLength);
          }
          else
          {
            hb_retc_null();
          }
        }
        else
        {
          hb_itemReturn(pText);
        }
      }
      else
      {
        hb_itemReturn(pText);
      }
    }
    else
    {
      hb_retc_null();
    }
  }
  else
  {
    // NOTE: Undocumented but existing Clipper Run-time error [vszakats]
#ifdef HB_CLP_STRICT
    hb_errRT_BASE_SubstR(EG_ARG, 1126, nullptr, HB_ERR_FUNCNAME, 0);
#else
    hb_errRT_BASE_SubstR(EG_ARG, 1126, nullptr, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
#endif
  }
}
