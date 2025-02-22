/*
 * Author....: Ted Means
 * CIS ID....: 73067,3332
 *
 * This function is an original work by Ted Means and is placed in the
 * public domain.
 *
 * Modification history:
 * ---------------------
 *
 *    Rev 1.1   01 May 1995 03:05:00   TED
 * Added typecast to tame compiler warning
 *
 *    Rev 1.0   01 Feb 1995 03:02:00   TED
 * Initial release
 *
 */

#include <hbapi.hpp>
#include <hbapiitm.hpp>

HB_FUNC(FT_DESCEND)
{
   PHB_ITEM iP     = hb_itemParam(1);
   HB_TYPE  uiType = hb_itemType(iP);

   PHB_ITEM iR = nullptr;

   if( (uiType & Harbour::Item::NUMERIC) && (uiType & Harbour::Item::DOUBLE) )
   {
      iR = hb_itemPutND(nullptr, 0 - hb_itemGetND(iP));
   }
   else if( uiType & Harbour::Item::NUMERIC )
   {
      iR = hb_itemPutNL(nullptr, 0 - hb_itemGetNL(iP));
   }
   else if( uiType & Harbour::Item::DATE )
   {
      iR = hb_itemPutNL(nullptr, 0x4FD4C0L - hb_itemGetNL(iP));
   }
   else if( uiType & Harbour::Item::TIMESTAMP )
   {
      iR = hb_itemPutND(nullptr, 0x4FD4C0L - hb_itemGetTD(iP));
   }
   else if( uiType & Harbour::Item::LOGICAL )
   {
      iR = hb_itemPutL(0, (hb_itemGetL(iP) > 0) ? 0 : 1);
   }
   else if( uiType & Harbour::Item::STRING )
   {
      HB_SIZE uiLen = hb_itemSize(iP);

      auto pDescend = static_cast<char*>(hb_xgrab(uiLen));

      hb_itemCopyC(iP, pDescend, uiLen);

      for( HB_SIZE n = 0; n < uiLen; n++ )
      {
         pDescend[n] = static_cast<char>(0) - pDescend[n];
      }

      iR = hb_itemPutCL(nullptr, pDescend, uiLen);

      hb_xfree(pDescend);
   }

   hb_itemReturn(iR);

   hb_itemRelease(iP);
   hb_itemRelease(iR);
}
