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


#ifndef DMBCS__TRADER_DESK__UPDATE_CLOSING_PRICES__H
#define DMBCS__TRADER_DESK__UPDATE_CLOSING_PRICES__H


#include  <trader-desk/markets.h>
#include  <atomic>


/** \file
 *
 *  Declaration of the \c Update_Closing_Prices class. */


namespace DMBCS::Trader_Desk {


      struct  Alpha_Vantage__Monitor;
      using  Price_Server  =  Alpha_Vantage__Monitor;


  /** Really a function object, the implementation being the \c run
   *  method.  The purpose of the combined function is to scan the
   *  database for all companies in some market, use the Yahoo! API to get
   *  the latest prices for that company, and then ‘inject’ the results to
   *  some output destination, most likely back to the database itself,
   *  but also maybe to working \c Chart_Data objects. */

  namespace  Update_Closing_Prices 
  {
    /** We will throw this object any time we canʼt get service from
     *  Yahoo!. */
    struct No_Connection : runtime_error
        {
            explicit No_Connection (const exception&  E) 
              : runtime_error {pgettext ("Error",
                                         "Cannot get data from Internet")
                                   +  string {": "}  + E.what ()}
            {}
        };
    

    /** Vessel to hold the data which comes back from the Yahoo!
     *  server. */
    struct Data
        {
             /** Our own database sequence ID. */
             int  company_seqid;

             /** The date of the closing transaction. */
             int  year, month, day;

             /** The values of the daily amounts.  We put all of this into
              *  the database, though the application currently only takes
              *  any notice of the \c close prices.  */
             double  open, high, low, close;

             /** The volume of trade on this day--put into the database,
              *  but not currently used by the application. */
             int  volume;

             /** The ‘adjusted’ closing price--put into the database, but
              *  not currently used by the application. */
             double  adj_close;
        };

    
    /** Vessel to hold information about companies for which we need to
     *  get updated information. */
    struct Company 
        {
             /** Human-readable name of the company. */
             string  name;

             /** The companyʼs ticker symbol. */
             string  symbol;

             /** Our database sequence ID for the company. */
             int  seqid;

             /** The time of our most recent close price for this company. */
             time_t  last_close_date;
        };
    

    
    struct  Work
        {
             /** Our database sequence ID for the market in question. */
             const Market_Meta_Data&  market;

             /** A flag to tell us to stop processing (may be set from
              *  outside the class). */
             atomic<bool>  stop  {false};
        };
    

  /** Write the \a data to the database. */
  void  sql_injector  (DB&,  const Data&);


  /** Get the list of companies on which to get new data, from the
   *  database based on the market. */
  vector<Company>  entries_from_database  (DB&,  const size_t  market_seqid);
    

  /** Do the work: scan the \a db database for the appropriate \a
   *  companies and currently known closing prices, obtain new price
   *  information from data service and store these back to the database.  The
   *  return will list the symbols of companies of which we were unable to
   *  obtain the latest information.
   *
   *  If given (not \c nullptr), the \a progress_callback will be called
   *  at regular intervals with the percentage of work completed.
   *
   *  The \a injector will be called for every new datum that needs adding
   *  to a companyʼs price records.
   *
   *  If not \c nullptr, the \a company_done callback will be called after
   *  all data for a particular company have been processed as above. */
  void  do_update
      (Work&,
       DB&,
       Preferences&,
       const function <void (double, const Company&)>     progress_callback,
       const function <void (const Data&)>                injector,
       const function <void (const int&  company_seqid)>  company_done);
    

  };  /* End of namespace Update_Closing_Prices. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__UPDATE_CLOSING_PRICES__H. */
