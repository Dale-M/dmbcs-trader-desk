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


#include  <trader-desk/update-closing-prices.h>
#include  <trader-desk/alpha-vantage--monitor.h>


/** \file
 *
 *  Implementation of the \c Update_Closing_Prices class. */


namespace DMBCS::Trader_Desk { namespace Update_Closing_Prices {


  void  sql_injector  (DB&  db,  const Data&  data)
  {
    db.quick () << "replace into prices set date=\""
                << data.year << "-" << data.month << "-" << data.day
                << "\", open="         << data.open
                << ", high="           << data.high
                << ", low="            << data.low
                << ", close="          << data.close
                << ", volume="         << data.volume
                << ", adjusted_close=" << data.adj_close
                << ", company="        << data.company_seqid;

    db.quick () << "update company set last_close_date=greatest(\""
                << data.year << "-" << data.month << "-" << data.day
                << "\", last_close_date) where seqid=" << data.company_seqid;
  }


    
  vector<Company>  entries_from_database  (DB&  db,  const size_t  market_seqid)
  {
    auto  sql  {db.row_query ()};

    sql << "select rtrim(name), "
        << "       symbol, "
        << "       seqid, "
        << "       unix_timestamp(greatest(date_add(current_date(),"
                         << " interval -" << 10/* TIME_HORIZON */ << " year),"
                         <<               "last_close_date)) "
        << "  from company "
        << " where market=" << market_seqid;

    sql.execute ();

    vector <Company>  entries;
    entries.reserve  (sql.number_rows ());
    
    for (; sql; ++sql)
       {
           Company  e;
           sql >> e.name >> e.symbol >> e.seqid >> e.last_close_date;
           entries.push_back (move (e));
       }

    return entries;
  }



    static  tm  current_tm  ()
          {
                  using  C  =  chrono::system_clock;
                  const time_t  current_t   {C::to_time_t  (C::now ())};
                  return  *localtime (&current_t);
          }

    static  tm  tm_from  (const time_t  date)
          {
                  tm  start  {*localtime  (&date)};
                  /* ++ start.tm_mday; */
                  /* mktime  (&start); */
                  return  start;
          }

    static  bool  same_day  (const tm&  A,  const tm&  B)
          {
                return  A.tm_year == B.tm_year  &&  A.tm_mon == B.tm_mon
                                                &&  A.tm_mday == B.tm_mday;
          }

    static  tm  not_weekend  (tm&&  T)
          {
                mktime (&T);
                if  (T.tm_wday == 0)  T.tm_mday -= 2;
                else if (T.tm_wday == 6)  T.tm_mday -= 1;
                else  return T;
                mktime (&T);
                return  T;
          }

    static  tm  day_before  (tm&  T)
          {
                mktime (&T);
                T.tm_mday -= 1;
                mktime (&T);
                return  T;
          }


static  void  do_update
        (Work&  ucp,
         Preferences&  user_prefs,
         const function <void (double, const Company&)>     progress_callback,
         const function <void (const Data&)>                injector,
         const function <void (const int&  company_seqid)>  done_processing,
         const vector<Company>&  entries)
  {
    ucp.stop  =  false;

    for (size_t  i  {0};  i < entries.size ();  ++i)
      {
        if (ucp.stop)   break;

        const Company&  company  {entries [i]};

        if (progress_callback)
              progress_callback  (i / double (entries.size ()),  company);

        tm  now  {not_weekend (current_tm ())};
        const auto  close_hour  {chrono::duration_cast<chrono::hours>
                                   (ucp.market.world_data.close_time).count ()};
        const auto  close_min  {chrono::duration_cast<chrono::minutes>
                                   (ucp.market.world_data.close_time).count ()
                                 %  60};
        if  (now.tm_hour < close_hour
             ||  (now.tm_hour == close_hour  &&  now.tm_min < close_min))
          now  =  day_before (now);

        if  (!same_day  (tm_from  (company.last_close_date),  now))
          try   {  while (Price_Server::get_closing_prices
                                 (company,
                                  ucp.market.world_data.component_extension,
                                  user_prefs,
                                  injector)
                            ==  Price_Server::TO_DO::MORE_WORK)
                        if (ucp.stop)   break;   }

          catch (const Price_Server::Bad_API_Key&)    {  throw;  }

          catch (const Price_Server::Error&)
            {   cerr << "Skipping company " << company.name << ".\n";   }

          /* This will be thrown by curlpp if there is any serious problem
           * with the networking. */
          catch (const exception&  e)   {   throw No_Connection {e};   }
          
        if (ucp.stop)  break;
        if (done_processing)    done_processing (company.seqid);
      }
  }



  void  do_update
        (Work&  ucp,
         DB&  db,
         Preferences&  user_prefs,
         const function <void (double, const Company&)>     progress_callback,
         const function <void (const Data&)>                injector,
         const function <void (const int&  company_seqid)>  company_done)
    {
       return  do_update  (ucp,
                           user_prefs,
                           progress_callback,
                           injector,
                           company_done,
                           entries_from_database (db, ucp.market.seqid));
    }


} }  /*  End of namespace DMBCS::Trader_Desk::Update_Closing_Prices. */
