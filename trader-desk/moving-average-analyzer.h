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


#ifndef DMBCS__TRADER_DESK__MOVING_AVERAGE_ANALYZER__H
#define DMBCS__TRADER_DESK__MOVING_AVERAGE_ANALYZER__H


#include <trader-desk/analyzer.h>
#include <trader-desk/scale.h>


/** \file
 *
 *  Declaration of the \c Moving_Average_Analyzer class. */


namespace DMBCS::Trader_Desk {


  /** An \c Analyzer which computes and displays a smoothed version of a
   *  prices time-series. */

  struct Moving_Average_Analyzer : Analyzer
  {

    /** The widget which we proffer to control ourselves is nothing more
     *  than an exponential scale, calibrated to allow specification of
     *  averaging windows between one day and a year, with fine control
     *  out to 14 days. */

    struct Scale : Exponential_Scale
    {
      Scale (Chart_Data &chart_data, double const &initial_value)
                    : Exponential_Scale (chart_data,
                                         pgettext ("Label",
                                                   "Mean window = %.0f days"),
                                         Gdk::RGBA {"#ff0000"},
                                         Gdk::RGBA {"#ffaaaa"},
                                         Gdk::RGBA {"#ff6666"},
                                         1, 14, 365,
                                         initial_value)
      {}
    };


    /** The data that we are to analyze. */
    Chart_Data  &chart_data;

    /** The size of the window over which we compute means. */
    Duration     mean_window {chrono::hours {14*24}};

    /** The resulting time-series of local mean values. */
    Time_Series  mean_series;

    /** Fired whenever the analysis of data produces new results, which will
     *  need rendering in the GUI. */
    sigc::signal <void>   redraw_needed_;


    /** Called when the user slides the control on the \a
     *  exponential_scale, meaning to change the moving-average window. */
    void control_moved (Scale const *const);

    /** Called whenever we must re-compute the moving-average
     *  time-series, including when the \c chart_data change. */
    void compute ();


    /** Sole constructor which registers the \a chart_data we are to
     *  analyze. */
    explicit Moving_Average_Analyzer (Chart_Data &);


    /********************** Analyzer interface. ****************************/


    /** Make a single widget which controls the size of the moving average
     *  window. */
    vector <Gtk::Widget*> make_control_widgets ()  override;


    /** Draw the \c mean_series and add a \a tide label at all the \a
     *  marked points in time. */
    void graph_draw_hook (Chart_Context &,
                          Tide_Mark::List &tide,
                          unsigned number_shares,
                          vector <Tide_Mark::Price_Marker> const &marked) 
      override;


    /** Make sure the \a range includes the entire mean series, with room
     *  to breathe. */
    void stretch_outline (Time_Series::Range &range)  override;


    /** Return our signal so that the application can connect and act when
     *  we need a re-draw to take place. */
    sigc::signal <void> &signal_redraw_needed ()  override  
    {  return redraw_needed_;  }
    

  };  /* End of class Moving_Average_Analyzer. */
    

}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__MOVING_AVERAGE_ANALYZER__H. */
