/*
 * Copyright (c) 2017  Dale Mellor
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


#ifndef DMBCS__TRADER_DESK__APPLICATION__H
#define DMBCS__TRADER_DESK__APPLICATION__H


#include <trader-desk/hand-analysis-widget.h>
#include <trader-desk/update-latest-prices.h>


/** \file
 *
 *  Declaration of the \c Application class. */


namespace DMBCS::Trader_Desk {


  /** The glue that binds everything together, really half the
   *  implementation of the top-level \c Window defined in
   *  trader-desk.cc.
   *
   *  [This is still a work in progress.  Ultimately we want this \c
   *  Application class to be independent of the (GTK) GUI front-end, so
   *  need to change the split in functionality between this class and the
   *  \c Window class accordingly.] */

  struct Application
  {
    /*  The following are established outside of this class, and it is
     *  expected that this be done as soon as the class is constructed. */

    /** *The* set of user preferences.  May change at infrequent but
     *  random times.  */
    Preferences  user_prefs;

    /** The most top-level window of this application.  */
    Gtk::Window *window  {nullptr};

    /** A function we can push to Gtk::Idle when we need to recourse to
     *  the preferences dialog due to a problem in the preferences. */
    function<void (const string&)>  preferences_error;



    /** The part of the menu which is specific to the markets whose data
     *  we have loaded. */
    Glib::RefPtr <Gtk::Action>       last_data_menu;
    Glib::RefPtr <Gtk::Action>       close_data_menu;

    /** All of the actions available to the menu. */
    Glib::RefPtr <Gtk::ActionGroup>  actions;

    /** The menu manager. */
    Glib::RefPtr <Gtk::UIManager>    ui_manager;

    /** The notebook which appears, without tabs, as the main widget in
     *  the \c window. */
    Gtk::Notebook notebook;

    /** All of the data, organized into a list of grids of charts. */
    vector<unique_ptr<Chart_Grid>>   market_grids;

    /** The widget which shows detailed analysis of a particular companyʼs
     *  data, and allows for much interaction with the user. */
    unique_ptr<Hand_Analysis_Widget> hand_analysis;



    /**  The application is not in a good state until the constructor has
     *   run, user_prefs have been updated with fixed-up values (by virtue
     *   of the wizard), and create_database() is run.
     *
     *   !!!! This is far from ideal. */
    explicit  Application  (Preferences&&  P)  :  user_prefs  {move (P)}
    {}


    /** This method is used to *change* the grid being displayed,
     *  according to their \a position amongst the notebook pages,
     *  i.e. counting from one upwards. */
    void display_grid (size_t const &position);

    
    /** Mega-method which does all the work (including operating the
     *  display machinery) to get a new market working in the system. */
    void ingest_new_market ();


    /** Mega-method which does all the work to bring a marketʼs data up to
     *  date. */
    void update_closing_prices ();


  } ;  /* End of class Application. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__APPLICATION__H. */
