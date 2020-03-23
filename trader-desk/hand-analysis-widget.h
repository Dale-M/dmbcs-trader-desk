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


#ifndef DMBCS__TRADER_DESK__HAND_ANALYSIS_WIDGET__H
#define DMBCS__TRADER_DESK__HAND_ANALYSIS_WIDGET__H


#include <trader-desk/company-name-entry.h>
#include <trader-desk/trade-instruction.h>
#include <trader-desk/date-range-scale.h>
#include <trader-desk/shares-scale.h>


namespace DMBCS::Trader_Desk {


    /*
      +-----VBox-------------------------------------------------------------------------------------------+
      |  +----address_bar--------------------------------------------------------------------------------+ |
      |  |  +--company_name-------------+ +--positions---------------------+ +--trade_instruction----+   | |
      |  |  |                           | |                                | |                       |   | |
      |  |  +---------------------------+ +--------------------------------+ +-----------------------+   | |
      |  +-----------------------------------------------------------------------------------------------+ |
      |  +----display_h_box------------------------------------------------------------------------------+ |
      |  |  +----chart------------------------+  +----controls_h_box----------------------------------+  | |
      |  |  |                                 |  | +-shares-+ +-date_range--+ +--<Analyzer scales>--+ |  | |
      |  |  |                                 |  | |        | |             | |                     | |  | |
       ...                                  ...                                                        ...
      |  |  |                                 |  | |        | |             | |                     | |  | |
      |  |  |                                 |  | +--------+ +-------------+ +---------------------+ |  | |
      |  |  +---------------------------------+  +----------------------------------------------------+  | |
      |  +-----------------------------------------------------------------------------------------------+ |
      +----------------------------------------------------------------------------------------------------+
    */

  /** This is a compound widget which entirely looks after itself under
   *  the GTK machinery and the cooperation between various components
   *  (almost all hang off the back of the chart object which throws a
   *  signal whenever anything changes).  The composition--layout--of the
   *  widget is shown above. */

  struct Hand_Analysis_Widget : Gtk::VBox
  {
    /** The gap between sliders in the \c controls_h_box, needed so that
     *  complicated controls can emulate the appearance of this top-level
     *  widget. */
    static constexpr int const SCALE_SEPARATION {10};

    /** Layout widget as per diagram above. */
    Gtk::HBox display_h_box;

    /** Layout widget as per diagram above. */
    Gtk::HBox controls_h_box;

    /** Control widget as per diagram above. */
    Gtk::HBox address_bar;

    /** The chart which we are hand-analyzing. */
    Chart  chart;

    /** Display and select the company whose data are in the \c chart. */
    Company_Name_Entry  company_name;

    /** Buy/sell button and current price input. */
    Trade_Instruction  trade;

    /** Select the range of dates over which data are shown. */
    Date_Range_Scale  date_range;

    /** Select the number of shares in a hypothetical position (if we are
     *  in a position, this will be a static object which simply shows the
     *  number of shares). */
    Shares_Scale  shares;

    /** Us calling this method, provided by the wider application, will
     *  get the \a Chart_Data populated with the data from a chart which
     *  corresponds to the company with \a seqid. */
    function<void(Chart_Data &data, int const &seqid)> get_chart_from_grid;


    /** Sole constructor which gives us a fully operational object.  The
     *  incoming function \a gcfg must provide the machinery we need in \c
     *  get_chart_from_grid. */
    Hand_Analysis_Widget (function<void(Chart_Data&, int const &)> gcfg,
                          Preferences&);


    /** Find the selected chart in the \a grid, and then subsume that
     *  chart into our display: take a shadow copy of the data and show
     *  these in the graph. */
    void subsume_selected (Chart_Grid &grid);


  private:

    /*  Implementation detail: whenever we change to charting some new
     *  data we must arrange for the associated pre-fetch thread to run so
     *  that the amount of data available for display matches at least the
     *  time-period which the current chart spans. */
    void  new_chart  (Preferences&);


  };  /* End of class Hand_Analysis_Widget. */

    
}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__HAND_ANALYSIS_WIDGET__H. */
