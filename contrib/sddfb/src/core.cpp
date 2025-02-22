//
// Firebird SQL Database Driver
//
// Copyright 2007 Mindaugas Kavaliauskas <dbtopas at dbtopas.lt>
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

/* NOTE: Ugly hack to avoid this error when compiler with BCC 5.8.2 and above:
         Error E2238 C:\...\Firebird-2.1.1\include\ibase.h 82: Multiple declaration for 'intptr_t' */
#if (defined(__BORLANDC__) && __BORLANDC__ >= 0x582)
/* Prevent inclusion of <stdint.h> from hbdefs.h */
   #define __STDINT_H
#endif

#include "hbrddsql.hpp"
#include <hbapiitm.hpp>
#include <hbvm.hpp>
#include <ibase.h>

struct SDDCONN
{
   isc_db_handle hDb;
};

struct SDDDATA
{
   isc_tr_handle    hTrans;
   isc_stmt_handle  hStmt;
   XSQLDA ISC_FAR * pSqlda;
};

static HB_ERRCODE fbConnect(SQLDDCONNECTION * pConnection, PHB_ITEM pItem);
static HB_ERRCODE fbDisconnect(SQLDDCONNECTION * pConnection);
static HB_ERRCODE fbExecute(SQLDDCONNECTION * pConnection, PHB_ITEM pItem);
static HB_ERRCODE fbOpen(SQLBASEAREAP pArea);
static HB_ERRCODE fbClose(SQLBASEAREAP pArea);
static HB_ERRCODE fbGoTo(SQLBASEAREAP pArea, HB_ULONG ulRecNo);

static SDDNODE s_firebirddd = {
   nullptr,
   "FIREBIRD",
   static_cast<SDDFUNC_CONNECT>(fbConnect),
   static_cast<SDDFUNC_DISCONNECT>(fbDisconnect),
   static_cast<SDDFUNC_EXECUTE>(fbExecute),
   static_cast<SDDFUNC_OPEN>(fbOpen),
   static_cast<SDDFUNC_CLOSE>(fbClose),
   static_cast<SDDFUNC_GOTO>(fbGoTo),
   static_cast<SDDFUNC_GETVALUE>(nullptr),
   static_cast<SDDFUNC_GETVARLEN>(nullptr)
};

static void hb_firebirddd_init(void * cargo)
{
   HB_SYMBOL_UNUSED(cargo);

   if( !hb_sddRegister(&s_firebirddd) || (sizeof(isc_db_handle) != sizeof(void*)) ) {
      hb_errInternal(HB_EI_RDDINVALID, nullptr, nullptr, nullptr);
   }
}

HB_FUNC(HB_SDDFB_REGISTER)
{
   hb_firebirddd_init(nullptr);
}

/* force SQLBASE linking */
HB_FUNC_TRANSLATE(SDDFB, SQLBASE)

HB_INIT_SYMBOLS_BEGIN(firebirddd__InitSymbols)
{
   "SDDFB", {HB_FS_PUBLIC | HB_FS_LOCAL}, {HB_FUNCNAME(SDDFB)}, nullptr
},
HB_INIT_SYMBOLS_END(firebirddd__InitSymbols)

HB_CALL_ON_STARTUP_BEGIN(_hb_firebirddd_init_)
hb_vmAtInit(hb_firebirddd_init, nullptr);
HB_CALL_ON_STARTUP_END(_hb_firebirddd_init_)

#if defined(HB_PRAGMA_STARTUP)
   #pragma startup firebirddd__InitSymbols
   #pragma startup _hb_firebirddd_init_
#elif defined(HB_DATASEG_STARTUP)
   #define HB_DATASEG_BODY  HB_DATASEG_FUNC(firebirddd__InitSymbols) \
   HB_DATASEG_FUNC(_hb_firebirddd_init_)
   #include "hbiniseg.hpp"
#endif

/* --- */
static HB_USHORT hb_errRT_FirebirdDD(HB_ERRCODE errGenCode, HB_ERRCODE errSubCode, const char * szDescription, const char * szOperation, HB_ERRCODE errOsCode)
{
   auto pError = hb_errRT_New(ES_ERROR, "SDDFB", errGenCode, errSubCode, szDescription, szOperation, errOsCode, EF_NONE);
   HB_USHORT uiAction = hb_errLaunch(pError);
   hb_itemRelease(pError);
   return uiAction;
}

/* --- SDD METHODS --- */
static HB_ERRCODE fbConnect(SQLDDCONNECTION * pConnection, PHB_ITEM pItem)
{
   ISC_STATUS_ARRAY status;
   auto hDb = static_cast<isc_db_handle>(0);
   char parambuf[520];
   int i;

   i = 0;
   parambuf[i++] = isc_dpb_version1;

   parambuf[i++] = isc_dpb_user_name;
   auto ul = static_cast<unsigned int>(hb_arrayGetCLen(pItem, 3));
   if( ul > 255 ) {
      ul = 255;
   }
   parambuf[i++] = static_cast<char>(ul);
   memcpy(parambuf + i, hb_arrayGetCPtr(pItem, 3), ul);
   i += ul;

   parambuf[i++] = isc_dpb_password;
   ul = static_cast<unsigned int>(hb_arrayGetCLen(pItem, 4));
   if( ul > 255 ) {
      ul = 255;
   }
   parambuf[i++] = static_cast<char>(ul);
   memcpy(parambuf + i, hb_arrayGetCPtr(pItem, 4), ul);
   i += ul;

   if( isc_attach_database(status, static_cast<short>(hb_arrayGetCLen(pItem, 5)), hb_arrayGetCPtr(pItem, 5), &hDb, static_cast<short>(i), parambuf) ) {
      /* TODO: error code in status[1] */
      return Harbour::FAILURE;
   }
   pConnection->pSDDConn = hb_xgrab(sizeof(SDDCONN));
   (static_cast<SDDCONN*>(pConnection->pSDDConn))->hDb = hDb;
#if 0
   HB_TRACE(HB_TR_ALWAYS, ("hDb=%d", hDb));
#endif
   return Harbour::SUCCESS;
}

static HB_ERRCODE fbDisconnect(SQLDDCONNECTION * pConnection)
{
   ISC_STATUS_ARRAY status;
   isc_detach_database(status, &(static_cast<SDDCONN*>(pConnection->pSDDConn))->hDb);
   hb_xfree(pConnection->pSDDConn);
   return Harbour::SUCCESS;
}

static HB_ERRCODE fbExecute(SQLDDCONNECTION * pConnection, PHB_ITEM pItem)
{
   HB_SYMBOL_UNUSED(pConnection);
   HB_SYMBOL_UNUSED(pItem);
   return Harbour::SUCCESS;
}

static HB_ERRCODE fbOpen(SQLBASEAREAP pArea)
{
   isc_db_handle * phDb = &(static_cast<SDDCONN*>(pArea->pConnection->pSDDConn))->hDb;
   auto hTrans = static_cast<isc_tr_handle>(0);
   auto hStmt = static_cast<isc_stmt_handle>(0);

   pArea->pSDDData = memset(hb_xgrab(sizeof(SDDDATA)), 0, sizeof(SDDDATA));
   auto pSDDData = static_cast<SDDDATA*>(pArea->pSDDData);

   ISC_STATUS_ARRAY status{};

#if 0
   HB_TRACE(HB_TR_ALWAYS, ("db=%d", hDb));
#endif
   if( isc_start_transaction(status, &hTrans, 1, phDb, 0, nullptr) ) {
#if 0
      HB_TRACE(HB_TR_ALWAYS, ("hTrans=%d status=%ld %ld %ld %ld", static_cast<int>(hTrans), static_cast<long>(status[0]), static_cast<long>(status[1]), static_cast<long>(status[2]), static_cast<long>(status[3])));
#endif
      hb_errRT_FirebirdDD(EG_OPEN, ESQLDD_START, "Start transaction failed", nullptr, static_cast<HB_ERRCODE>(isc_sqlcode(status)));
      return Harbour::FAILURE;
   }

   if( isc_dsql_allocate_statement(status, phDb, &hStmt) ) {
      hb_errRT_FirebirdDD(EG_OPEN, ESQLDD_STMTALLOC, "Allocate statement failed", nullptr, static_cast<HB_ERRCODE>(isc_sqlcode(status)));
      isc_rollback_transaction(status, &hTrans);
      return Harbour::FAILURE;
   }

   auto pSqlda = static_cast<XSQLDA*>(hb_xgrab(XSQLDA_LENGTH(1)));
   pSqlda->sqln = 1;
   pSqlda->version = 1;

   if( isc_dsql_prepare(status, &hTrans, &hStmt, 0, pArea->szQuery, SQL_DIALECT_V5, pSqlda) ) {
      hb_errRT_FirebirdDD(EG_OPEN, ESQLDD_INVALIDQUERY, "Prepare statement failed", pArea->szQuery, static_cast<HB_ERRCODE>(isc_sqlcode(status)));
      isc_dsql_free_statement(status, &hStmt, DSQL_drop);
      isc_rollback_transaction(status, &hTrans);
      hb_xfree(pSqlda);
      return Harbour::FAILURE;
   }

   if( isc_dsql_execute(status, &hTrans, &hStmt, 1, nullptr) ) {
      hb_errRT_FirebirdDD(EG_OPEN, ESQLDD_EXECUTE, "Execute statement failed", pArea->szQuery, static_cast<HB_ERRCODE>(isc_sqlcode(status)));
      isc_dsql_free_statement(status, &hStmt, DSQL_drop);
      isc_rollback_transaction(status, &hTrans);
      hb_xfree(pSqlda);
      return Harbour::FAILURE;
   }

   HB_USHORT uiFields;

   if( pSqlda->sqld > pSqlda->sqln ) {
      uiFields = pSqlda->sqld;
      hb_xfree(pSqlda);
      pSqlda = static_cast<XSQLDA*>(hb_xgrab(XSQLDA_LENGTH(uiFields)));
      pSqlda->sqln = uiFields;
      pSqlda->version = 1;

      if( isc_dsql_describe(status, &hStmt, SQL_DIALECT_V5, pSqlda) ) {
         hb_errRT_FirebirdDD(EG_OPEN, ESQLDD_STMTDESCR, "Describe statement failed", nullptr, static_cast<HB_ERRCODE>(isc_sqlcode(status)));
         isc_dsql_free_statement(status, &hStmt, DSQL_drop);
         isc_rollback_transaction(status, &hTrans);
         hb_xfree(pSqlda);
         return Harbour::FAILURE;
      }
   }

   pSDDData->hTrans = hTrans;
   pSDDData->hStmt = hStmt;
   pSDDData->pSqlda = pSqlda;

   uiFields = pSqlda->sqld;
   SELF_SETFIELDEXTENT(&pArea->area, uiFields);

   auto pItemEof = hb_itemArrayNew(uiFields);

   HB_USHORT uiCount;
   XSQLVAR * pVar;
   bool bError = false;
   PHB_ITEM pItem;

   for( uiCount = 0, pVar = pSqlda->sqlvar; uiCount < uiFields; uiCount++, pVar++ ) {
      /* FIXME: if pVar->sqlname is ended with 0 byte then this hb_strndup()
       *        and hb_xfree() below is redundant and
       *          dbFieldInfo.atomName = pVar->sqlname;
       *        is enough.
       */
      char * szOurName = hb_strndup(pVar->sqlname, pVar->sqlname_length);

      DBFIELDINFO dbFieldInfo{};
      dbFieldInfo.atomName = szOurName;

      int iType = pVar->sqltype & ~1;
      switch( iType ) {
         case SQL_TEXT:
         {
            dbFieldInfo.uiType = Harbour::DB::Field::STRING;
            dbFieldInfo.uiLen = pVar->sqllen;
            pVar->sqldata = static_cast<char*>(hb_xgrab(sizeof(char) * pVar->sqllen + 2));
            auto pStr  = static_cast<char*>(memset(hb_xgrab(dbFieldInfo.uiLen), ' ', dbFieldInfo.uiLen));
            pItem = hb_itemPutCL(nullptr, pStr, dbFieldInfo.uiLen);
            hb_xfree(pStr);
            break;
         }

         case SQL_VARYING:
         {
            dbFieldInfo.uiType = Harbour::DB::Field::VARLENGTH;
            dbFieldInfo.uiLen = pVar->sqllen;
            /* pVar->sqltype = SQL_TEXT;  Coercing */
            pVar->sqldata = static_cast<char*>(hb_xgrab(sizeof(char) * pVar->sqllen + 2));
            auto pStr = static_cast<char*>(memset(hb_xgrab(dbFieldInfo.uiLen), ' ', dbFieldInfo.uiLen));
            pItem = hb_itemPutCL(nullptr, pStr, dbFieldInfo.uiLen);
            hb_xfree(pStr);
            break;
         }

         case SQL_SHORT:
            if( pVar->sqlscale < 0 ) {
               dbFieldInfo.uiType = Harbour::DB::Field::LONG;
               dbFieldInfo.uiLen = 7;
               dbFieldInfo.uiDec = -pVar->sqlscale;
               pItem = hb_itemPutNDLen(nullptr, 0.0, 6 - dbFieldInfo.uiDec, static_cast<int>(dbFieldInfo.uiDec));
            } else {
               dbFieldInfo.uiType = Harbour::DB::Field::INTEGER;
               dbFieldInfo.uiLen = 2;
               pItem = hb_itemPutNILen(nullptr, 0, 6);
            }
            pVar->sqldata = static_cast<char*>(hb_xgrab(sizeof(short)));
            break;

         case SQL_LONG:
            if( pVar->sqlscale < 0 ) {
               dbFieldInfo.uiType = Harbour::DB::Field::LONG;
               dbFieldInfo.uiLen = 12;
               dbFieldInfo.uiDec = -pVar->sqlscale;
               pItem = hb_itemPutNDLen(nullptr, 0.0, 11 - dbFieldInfo.uiDec, static_cast<int>(dbFieldInfo.uiDec));
            } else {
               dbFieldInfo.uiType = Harbour::DB::Field::INTEGER;
               dbFieldInfo.uiLen = 4;
               pItem = hb_itemPutNLLen(nullptr, 0, 11);
            }
            pVar->sqldata = static_cast<char*>(hb_xgrab(sizeof(long)));
            break;

         case SQL_FLOAT:
            dbFieldInfo.uiType = Harbour::DB::Field::DOUBLE;
            dbFieldInfo.uiLen = 8;
            dbFieldInfo.uiDec = -pVar->sqlscale;
            pVar->sqldata = static_cast<char*>(hb_xgrab(sizeof(float)));
            pItem = hb_itemPutNDLen(nullptr, 0.0, 20 - dbFieldInfo.uiDec, dbFieldInfo.uiDec);
            break;

         case SQL_DOUBLE:
            dbFieldInfo.uiType = Harbour::DB::Field::DOUBLE;
            dbFieldInfo.uiLen = 8;
            dbFieldInfo.uiDec = -pVar->sqlscale;
            pVar->sqldata = static_cast<char*>(hb_xgrab(sizeof(double)));
            pItem = hb_itemPutNDLen(nullptr, 0.0, 20 - dbFieldInfo.uiDec, dbFieldInfo.uiDec);
            break;

         case SQL_TIMESTAMP:
            dbFieldInfo.uiType = Harbour::DB::Field::TIMESTAMP;
            dbFieldInfo.uiLen = 8;
            pVar->sqldata = static_cast<char*>(hb_xgrab(sizeof(ISC_TIMESTAMP)));
            pItem = hb_itemPutTDT(nullptr, 0, 0);
            break;

         default: /* other fields as binary string */
            pVar->sqldata = static_cast<char*>(hb_xgrab(sizeof(char) * pVar->sqllen));
            pItem = hb_itemNew(nullptr);
            bError = true;
            break;
      }

      if( pVar->sqltype & 1 ) {
         pVar->sqlind = static_cast<short*>(hb_xgrab(sizeof(short)));
      }

      hb_arraySetForward(pItemEof, uiCount + 1, pItem);
      hb_itemRelease(pItem);

      if( !bError ) {
         bError = (SELF_ADDFIELD(&pArea->area, &dbFieldInfo) == Harbour::FAILURE);
      }

      hb_xfree(szOurName);

      if( bError ) {
         break;
      }
   }

   if( bError ) {
      hb_itemClear(pItemEof);
      hb_itemRelease(pItemEof);
      hb_errRT_FirebirdDD(EG_CORRUPTION, ESQLDD_INVALIDFIELD, "Invalid field type", pArea->szQuery, 0);
      return Harbour::FAILURE;
   }

   pArea->ulRecCount = 0;

   pArea->pRow = static_cast<void**>(hb_xgrab(SQLDD_ROWSET_INIT * sizeof(void*)));
   pArea->pRowFlags = static_cast<HB_BYTE*>(hb_xgrab(SQLDD_ROWSET_INIT * sizeof(HB_BYTE)));
   pArea->ulRecMax = SQLDD_ROWSET_INIT;

   pArea->pRow[0] = pItemEof;
   pArea->pRowFlags[0] = SQLDD_FLAG_CACHED;

   return Harbour::SUCCESS;
}


static HB_ERRCODE fbClose(SQLBASEAREAP pArea)
{
   auto pSDDData = static_cast<SDDDATA*>(pArea->pSDDData);
   ISC_STATUS_ARRAY status;

   if( pSDDData ) {
      if( pSDDData->pSqlda ) {
         hb_xfree(pSDDData->pSqlda);
      }
      if( pSDDData->hStmt ) {
         isc_dsql_free_statement(status, &pSDDData->hStmt, DSQL_drop);
      }
      if( pSDDData->hTrans ) {
         isc_rollback_transaction(status, &pSDDData->hTrans);
      }
      hb_xfree(pSDDData);
      pArea->pSDDData = nullptr;
   }

   return Harbour::SUCCESS;
}


static HB_ERRCODE fbGoTo(SQLBASEAREAP pArea, HB_ULONG ulRecNo)
{
   auto pSDDData = static_cast<SDDDATA*>(pArea->pSDDData);

   ISC_STATUS lErr;
   ISC_STATUS_ARRAY status;
   HB_USHORT ui;
   XSQLVAR * pVar;
   short iType;

   while( ulRecNo > pArea->ulRecCount && !pArea->fFetched ) {
      isc_stmt_handle * phStmt = &pSDDData->hStmt;
      isc_tr_handle * phTr = &pSDDData->hTrans;

      lErr = isc_dsql_fetch(status, phStmt, SQL_DIALECT_V5, pSDDData->pSqlda);

      if( lErr == 0 ) {
         PHB_ITEM pItem = nullptr;

         auto pArray = hb_itemArrayNew(pArea->area.uiFieldCount);
         for( ui = 0; ui < pArea->area.uiFieldCount; ui++ ) {
            pVar = pSDDData->pSqlda->sqlvar + ui;

            if( (pVar->sqltype & 1) && (*pVar->sqlind < 0) ) {
               continue;  /* NIL value */
            }

            LPFIELD pField = pArea->area.lpFields + ui;
            iType  = pVar->sqltype & ~1;
            switch( iType ) {
               case SQL_TEXT:
                  pItem = hb_itemPutCL(pItem, pVar->sqldata, pVar->sqllen);
                  break;

               case SQL_VARYING:
                  pItem = hb_itemPutCL(pItem, pVar->sqldata + 2, *reinterpret_cast<short*>(pVar->sqldata));
                  break;

               case SQL_SHORT:
                  if( pField->uiDec == 0 ) {
                     pItem = hb_itemPutNILen(pItem, *reinterpret_cast<short*>(pVar->sqldata), 6);
                  } else {
                     pItem = hb_itemPutNDLen(pItem, hb_numDecConv(*reinterpret_cast<short*>(pVar->sqldata), static_cast<int>(pField->uiDec)), 6 - pField->uiDec, static_cast<int>(pField->uiDec));
                  }
                  break;

               case SQL_LONG:
                  if( pField->uiDec == 0 ) {
                     pItem = hb_itemPutNLLen(pItem, *reinterpret_cast<short*>(pVar->sqldata), 11);
                  } else {
                     pItem = hb_itemPutNDLen(pItem, hb_numDecConv(*reinterpret_cast<long*>(pVar->sqldata), static_cast<int>(pField->uiDec)), 11 - pField->uiDec, static_cast<int>(pField->uiDec));
                  }
                  break;

               case SQL_FLOAT:
                  pItem = hb_itemPutNDLen(pItem, *reinterpret_cast<float*>(pVar->sqldata), 20 - pField->uiDec, pField->uiDec);
                  break;

               case SQL_DOUBLE:
                  pItem = hb_itemPutNDLen(pItem, *reinterpret_cast<double*>(pVar->sqldata), 20 - pField->uiDec, pField->uiDec);
                  break;

               case SQL_TIMESTAMP:
                  pItem = hb_itemPutTDT(pItem, (reinterpret_cast<ISC_TIMESTAMP*>(pVar->sqldata) )->timestamp_date + 2400001, (reinterpret_cast<ISC_TIMESTAMP*>(pVar->sqldata))->timestamp_time / 10);
                  break;

               default:
                  /* default value is NIL */
                  break;
            }
            if( pItem != nullptr ) {
               hb_arraySetForward(pArray, ui + 1, pItem);
            }
         }

         if( pArea->ulRecCount + 1 >= pArea->ulRecMax ) {
            pArea->pRow = static_cast<void**>(hb_xrealloc(pArea->pRow, (pArea->ulRecMax + SQLDD_ROWSET_RESIZE) * sizeof(void*)));
            pArea->pRowFlags = static_cast<HB_BYTE*>(hb_xrealloc(pArea->pRowFlags, (pArea->ulRecMax + SQLDD_ROWSET_RESIZE) * sizeof(HB_BYTE)));
            pArea->ulRecMax += SQLDD_ROWSET_RESIZE;
         }

         pArea->ulRecCount++;
         pArea->pRow[pArea->ulRecCount] = pArray;
         pArea->pRowFlags[pArea->ulRecCount] = SQLDD_FLAG_CACHED;
         hb_itemRelease(pItem);
      } else if( lErr == 100 ) {
         pArea->fFetched = true;
         if( isc_dsql_free_statement(status, phStmt, DSQL_drop) ) {
            hb_errRT_FirebirdDD(EG_OPEN, ESQLDD_STMTFREE, "Statement free error", nullptr, static_cast<HB_ERRCODE>(isc_sqlcode(status)));
            return Harbour::FAILURE;
         }
         pSDDData->hStmt = static_cast<isc_stmt_handle>(0);

         if( isc_commit_transaction(status, phTr) ) {
            hb_errRT_FirebirdDD(EG_OPEN, ESQLDD_COMMIT, "Transaction commit error", nullptr, static_cast<HB_ERRCODE>(isc_sqlcode(status)));
            return Harbour::FAILURE;
         }
         pSDDData->hTrans = static_cast<isc_tr_handle>(0);

         hb_xfree(pSDDData->pSqlda);  /* TODO: free is more complex */
         pSDDData->pSqlda = nullptr;
      } else {
         hb_errRT_FirebirdDD(EG_OPEN, ESQLDD_FETCH, "Fetch error", nullptr, static_cast<HB_ERRCODE>(lErr));
         return Harbour::FAILURE;
      }
   }

   if( ulRecNo == 0 || ulRecNo > pArea->ulRecCount ) {
      pArea->pRecord = pArea->pRow[0];
      pArea->bRecordFlags = pArea->pRowFlags[0];
      pArea->fPositioned = false;
   } else {
      pArea->pRecord = pArea->pRow[ulRecNo];
      pArea->bRecordFlags = pArea->pRowFlags[ulRecNo];
      pArea->fPositioned = true;
   }

   return Harbour::SUCCESS;
}
