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


#ifndef DMBCS_TRADER_DESK__UPDATE_LATEST_PRICES__H
#define DMBCS_TRADER_DESK__UPDATE_LATEST_PRICES__H


#include <trader-desk/chart-data.h>
#include <trader-desk/update-closing-prices.h>


/** \file
 *
 *  Declaration of the \c Update_Latest_Prices class. */


namespace DMBCS::Trader_Desk {


  /** Function object which reads a list of companies in a market from the
   *  database, fetches the latest prices for those companies from the
   *  Yahoo! service, and then writes the new information back to the
   *  database and gives them back to the calling application.
   *
   *  Similarities with the \c Update_Closing_Prices class are actually
   *  quite superficial, but we do inherit that class mainly to take
   *  advantage of the functionality for the initial fetch of the company
   *  list from the database. */

  namespace  Update_Latest_Prices
  {
    /** Vessel to carry the information that comes back from the data
     *  service. */
    struct Data   {
                      /** Our database sequence ID. */
                      int company_seqid;

                      /** The time at which the price holds. */
                      Time_Point time;

                      /** The current price of this commodity. */
                      double price;
                   };

    
    using  Company  =  Update_Closing_Prices::Company;

    struct  Work  :  Update_Closing_Prices::Work  {};

    /** Update the database with the new \a data. */
    void  sql_injector  (DB&,  const Data&);


    /** Get the companies for the registered market, fetch their latest
     *  data from the Yahoo! server, and pass the results, one at a time,
     *  to \a injector. */
    void  do_update  (Work *const  work,
                      DB&,
                      Preferences&,
                      function <void (const Data&)>  injector);


    void  do_update  (DB&,  Chart_Data&,  Preferences&);
    

} }  /* End of namespace DMBCS::Trader_Desk::Update_Latest_Prices. */


#endif  /* Undefined DMBCS_TRADER_DESK__UPDATE_LATEST_PRICES__H. */
