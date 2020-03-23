/*
 * Copyright (c) 2020  Dale Mellor
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


#ifndef DMBCS__TRADER_DESK__ALPHA_VANTAGE__H
#define DMBCS__TRADER_DESK__ALPHA_VANTAGE__H


#include  <trader-desk/update-closing-prices.h>
#include  <trader-desk/update-latest-prices.h>


namespace DMBCS::Trader_Desk {


    /* This is a struct rather than a namespace because we want to be able
     * to derive from it a version with a proper API key, and want to
     * alias it as a type in ‘update_*_prices.cc’.
     *
     * The reason for this seemingly unnecessary abstraction is that in
     * the past we use Yahoo! Finance for this purpose, but they pulled
     * their API.  We now want to keep things flexible so that other data
     * sources might be used in future. */

struct  Alpha_Vantage
  {
      struct  Error  :  runtime_error  { using  runtime_error::runtime_error; };
      struct  Throttled    :  Error    {   using  Error::Error;   };
      struct  Bad_API_Key  :  Error    {   using  Error::Error;   };


      enum  TO_DO  :  bool  {FINISHED = 0,  MORE_WORK = 1};
      static  TO_DO  get_closing_prices   /* override */
         (const Update_Closing_Prices::Company&,
          const string&  market_component_extension,
          const Preferences&  P,
          const function <void (const Update_Closing_Prices::Data&)>  injector);


      static  Update_Latest_Prices::Data  get_latest_data  /* override */
                              (const Update_Latest_Prices::Company&,
                               const string&  market_component_extension,
                               const Preferences&);
      

  } ;  /* End of class Alpha_Vantage. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__ALPHA_VANTAGE__H. */
