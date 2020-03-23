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


#ifndef DMBCS__TRADER_DESK__CHART_GRID__H
#define DMBCS__TRADER_DESK__CHART_GRID__H


#include <trader-desk/chart.h>
#include <trader-desk/markets.h>


/** \file
 *
 *  Declaration of the \c Chart_Grid class. */


namespace DMBCS::Trader_Desk {


  /** Widget which manages a whole bunch of charts, for all companies in a
   *  market, and displays them all at once in a grid on the screen. */

  struct Chart_Grid : Gtk::EventBox
  {
    Preferences&  user_prefs;

    /** The duration displayed in each thumbnail. */
    static constexpr chrono::hours const DEFAULT_SPAN {50 * 24};

    /** Object packed with nothing but \c Chart widgets. */
    Gtk::Table table;

    /** All the \c Chart's we are managing. */
    vector <unique_ptr <Chart>> chart;

    /** Pointer into the \c chart list: the one we last clicked on. */
    Chart *selection {nullptr};

    /** We emit this whenever the user clicks on a chart in the grid. */
    sigc::signal <void> selection_signal;

    /** The market we hold and display charts for. */
    Market_Meta_Data market;


    /** Sole constructor, gives us a fully operational object. */
    Chart_Grid (Preferences&, const Market_Meta_Data&);

    /** Find the chart corresponding to the company with the database \a
     *  seqid, or return \c nullptr. */
    Chart *find_chart (int const &seqid);

    /** Completely re-construct this object based on the data currently in
     *  the database.  If \a force is TRUE, then this object will be
     *  constructed according to the information in the database; if \a
     *  force is FALSE then the database may be updated with new
     *  information about the market components, and if a change is made
     *  in these then this object will be refreshed. */
    void regenerate (DB &,  Preferences&,  bool const force = 0);

    /** Make sure all the individual charts have their data loaded before
     *  we attempt to render them on-screen. */
    bool on_draw (const Cairo::RefPtr<Cairo::Context>&) override;

    /** Called when the user selects an individual chart.  We update our
     *  state and emit the \c selection_signal. */
    bool on_button_release_event (GdkEventButton *const) override;
    

  };  /* End of class Chart_Grid. */

    
}  /* End of namespace DMBCS::Trader_Desk. */
    

#endif  /* Undefined DMBCS__TRADER_DESK__CHART_GRID__H. */
