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


#ifndef DMBCS__TRADER_DESK__MARKETS__H
#define DMBCS__TRADER_DESK__MARKETS__H


#include <dmbcs-market-data-api.h>
#include "db.h"
#include <gtkmm.h>


/** \file
 *
 *  Declaration of the \c Market_Meta_Data class and the \c Markets
 *  class. */


namespace DMBCS::Trader_Desk {


  /** Really just a data-carrying structure (vessel) which fully describes
   *  a stock market.  It is divided into two parts: data which describe
   *  the markets in the eyes of the outside world, and data which
   *  describe the state of the markets from our own, private, point of
   *  view. */
  struct Market_Meta_Data
     {
        /** The subset of data which relate to the state of affairs in the
         *  world beyond this application; these are obtained from an
         *  Internet server through the medium of the
         *  dmbcs-market-data-api library. */
        Market_Data_Api::Market  world_data;

        /** The sequence ID of this market in our local database. */
        size_t  seqid;

        /** Whether we are keeping a database of prices for this
         *  market. */
        bool  tracked;

        /** The last time the component data for this market were
         *  synchronized with a market data server. */
        time_t  last_time;

     };  /* End of class Market_Meta_Data. */



  void  update_database
              (const Market_Data_Api::Delta&,  DB&,  const int  market_seqid);



  /** Fetch the data for this market from the Internet and update the
   *  database if necessary.  A progress indication may be provided in a
   *  dialog on top of the window. */
  bool  update_components  (Market_Meta_Data&,  DB&,  Gtk::Window *const);



  /** A self-building collection of all markets known to the on-line data
   *  server, indexed according to our local database sequence ID. */
  struct Markets  :  map <size_t, Market_Meta_Data>
  {
    /** Sole constructor, which self-builds our \c map from information
     *  contained in the local \a database. */
    explicit  Markets  (DB&  database);

    /** Constructing these objects is expensive; allow them to be moved
     *  around but not duplicated. */
    Markets ()                            = delete;
    Markets (Markets const &)             = delete;
    Markets (Markets &&)                  = default;
    Markets &operator= (Markets const &)  = delete;
    Markets &operator= (Markets &&)       = default;

  } ;  /* End of class Markets. */

    
  /** Market the market with \a symbol as being tracked, both in memory
   *  (here) and in the database.  Note that this does NOT cause any price
   *  data to be fetched from the server. */
  void  start_tracking  (Markets&,  DB&,  const string&  symbol);

  /** Get information on all known markets from the server, and update our
   *  own information and the database. */
  void  update_market_meta_data  (Markets&,  DB&);

  /** Update the meta-data with the above method, then update the
   *  components of each market we follow (note *not* the price
   *  information!)  These actions will not be allowed to take place more
   *  than once per 12 hours.  The return value indicates if anything has
   *  actually changed.  If a window is given, provide a report on changes
   *  in a dialog box on the window. */
  bool  update_components  (Markets *const,  Gtk::Window *const  =  nullptr);


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__MARKETS__H. */
