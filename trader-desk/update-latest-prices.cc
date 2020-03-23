/*
 * Copyright (c) 2017, 2020  Dale Mellor
 *
 *  This file is part of the trader-desk package.
 *
 *  The trader-desk package is free software: you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  The trader-desk package is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see http://www.gnu.org/licenses/.
 */


#include  <trader-desk/update-latest-prices.h>
#include  <trader-desk/alpha-vantage--monitor.h>
#include  <algorithm>
#include  <numeric>
#include  <set>


/** \file
 *
 *  Implementation of the \c Update_Latest_Prices class. */


namespace DMBCS::Trader_Desk::Update_Latest_Prices {


      using  Data_Server  =  Alpha_Vantage__Monitor;


  void  sql_injector  (DB&  db,  const Data&  data)
  {
    time_t t = chrono::system_clock::to_time_t (data.time);
    tm tm;
    localtime_r (&t, &tm);

    db.quick () << "update company "
                <<    "set last_price=" << data.price << ','
                <<        "last_price_date=\"" << (tm.tm_year+1900) << '-'
                                               << (tm.tm_mon+1)
                                               << '-' << tm.tm_mday << ' '
                                               << tm.tm_hour << ':' << tm.tm_min
                                               << ':' << tm.tm_sec << "\" "
                << " where seqid=" << data.company_seqid;
  }



  static void  do_update  (Work *const  work,
                           Preferences&  P,
                           function <void (Data const &)> injector,
                           vector<Company> const &entries)
  try
  {
    for  (const Company&  C  :  entries)
      injector  (Data_Server::get_latest_data
                       (C,  work->market.world_data.component_extension,  P));
  }
  catch (exception const &e)
    {
      throw Update_Closing_Prices::No_Connection {e};
    }
    


  void  do_update  (Work *const  work,
                    DB&  db,
                    Preferences&  P,
                    function <void (Data const &)>  injector)
  {
     do_update (work,
                P,
                injector,
                Update_Closing_Prices::entries_from_database
                                              (db,  work->market.seqid));
  }



  void  do_update  (DB&  db,  Chart_Data&  data,  Preferences&  P)
  {
    auto  row  {db.row_query ()};
    row  <<  "select company.symbol, "
         <<         "unix_timestamp(company.last_close_date), "
         <<         "market.component_extension "
         <<    "from company, market "
         <<   "where company.seqid=" << data.company_seqid
         <<        " and market.seqid=company.market";
    row.execute ();

    Company  C;
    row  >>  C.symbol  >>  C.last_close_date;
    C.seqid  =  data.company_seqid;

    const string  market_symbol  {row.next_entry<string> ()};

    const Data  D  {Data_Server::get_latest_data  (C,  market_symbol,  P)};
    
    ++row;

    {unique_lock  L  {data.prices_mutex};
           data.last_fetch_time  =  D.time;
           data.prices.insert_event  (  {D.time,  D.price}  );
           data.extremes.end_time  =  D.time;       }
    
    data.changed_signal.emit  ();
  }
  
      
}  /* End of namespace DMBCS::Trader_Desk::Update_Latest_Prices. */
