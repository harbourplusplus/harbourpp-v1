//
// At() function
//
// Copyright 2012 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
// Copyright 1999 Antonio Linares <alinares@fivetech.com>
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
#include "hbapicdp.hpp"
#include "hbapierr.hpp"

// locates a substring in a string

HB_FUNC(HB_AT)
{
  auto pSub = hb_param(1, Harbour::Item::STRING);
  auto pText = hb_param(2, Harbour::Item::STRING);

  if (pText && pSub)
  {
    auto cdp = hb_vmCDP();
    auto pszText = pText->getCPtr();
    auto nTextLength = pText->getCLen();
    HB_SIZE nStart = hb_parns(3);
    HB_SIZE nFrom, nPos = 0;

    if (nStart <= 1)
    {
      nStart = nFrom = 0;
    }
    else if (HB_CDP_ISCHARIDX(cdp))
    {
      nFrom = hb_cdpTextPos(cdp, pszText, nTextLength, --nStart);
    }
    else
    {
      nFrom = --nStart;
    }

    if (nFrom < nTextLength)
    {
      HB_SIZE nTo;

      pszText += nFrom;
      nTextLength -= nFrom;
      if (HB_ISNUM(4))
      {
        nTo = hb_parns(4);
        if (nTo <= nStart)
        {
          nTo = 0;
        }
        else
        {
          nTo -= nStart;
          if (HB_CDP_ISCHARIDX(cdp))
          {
            nTo = hb_cdpTextPos(cdp, pszText, nTextLength, nTo);
          }
          if (nTo > nTextLength)
          {
            nTo = nTextLength;
          }
        }
      }
      else
      {
        nTo = nTextLength;
      }

      if (nTo > 0)
      {
        nPos = hb_strAt(pSub->getCPtr(), pSub->getCLen(), pszText, nTo);
        if (nPos > 0)
        {
          if (HB_CDP_ISCHARIDX(cdp))
          {
            nPos = hb_cdpTextLen(cdp, pszText, nPos - 1) + 1 + nStart;
          }
          else
          {
            nPos += nFrom;
          }
        }
      }
    }
    hb_retns(nPos);
  }
  else
  {
    hb_errRT_BASE_SubstR(EG_ARG, 1108, nullptr, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
  }
}

HB_FUNC(AT)
{
  auto pSub = hb_param(1, Harbour::Item::STRING);
  auto pText = hb_param(2, Harbour::Item::STRING);

  if (pText && pSub)
  {
    HB_SIZE nPos = hb_strAt(pSub->getCPtr(), pSub->getCLen(), pText->getCPtr(), pText->getCLen());
    if (nPos)
    {
      auto cdp = hb_vmCDP();
      if (HB_CDP_ISCHARIDX(cdp))
      {
        nPos = hb_cdpTextLen(cdp, pText->getCPtr(), nPos - 1) + 1;
      }
    }
    hb_retns(nPos);
  }
  else
  {
    hb_errRT_BASE_SubstR(EG_ARG, 1108, nullptr, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
  }
}
