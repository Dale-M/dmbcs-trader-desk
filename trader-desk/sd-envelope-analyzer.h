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


#ifndef DMBCS__TRADER_DESK__SD_ENVELOPE_ANALYZER__H
#define DMBCS__TRADER_DESK__SD_ENVELOPE_ANALYZER__H


#include <trader-desk/moving-average-analyzer.h>


/** \file
 *
 *  Declaration of the \c SD_Envelope_Analyzer class. */


namespace DMBCS::Trader_Desk {


  /** A chart analyzer which puts an envelope around the prices chart
   *  related to the standard deviation (SD) of the prices data around
   *  their mean: any proportion of this SD can be chosen by the user
   *  acting on a slider (\c Scale).
   *
   *  This analyzer incorporates a \c Moving_Average_Analyzer within it,
   *  and provides a composite control to the application which allows for
   *  user adjustment of both the averaging window and relative width (as
   *  proportion of SD) of the envelope. */

  class SD_Envelope_Analyzer : public Analyzer
  {

    /** Simple linear scale which allows the user to choose an envelope
     *  width relative to SD between 0.1 and 10.0. */

    struct Scale : Linear_Scale
    {
      Scale (Chart_Data &chart_data, double const &initial_value)
                 : Linear_Scale (chart_data,
                                 pgettext ("Label Abbrev:Standard deviation",
                                           "Envelope width = %.2f x std. dev."),
                                 Gdk::RGBA {"#dddd00"},
                                 Gdk::RGBA {"#ffffaa"},
                                 Gdk::RGBA {"#ffff66"},
                                 0.1, 10.0, initial_value)
      {}
    };


    /** The moving average object which we encapsulate (and leverage for
     *  our own functioning). */
    Moving_Average_Analyzer moving_average;

    /** The width of the envelope as a fraction of the \c
     *  standard_deviation. */
    double envelope_width {2};

    /** The current standard deviation of the data in the prices
     *  time-series about the current mean time-series. */
    double standard_deviation {0};

    /** We emit this signal whenever we re-compute the \c
     *  standard_deviation. */
    sigc::signal<void> redraw_needed_;


    /** Called when the \a scale slider indicates that the user has
     *  changed the position. */
    void control_moved (Scale const *const);

    /** Called when the \c moving_average object signals that the mean
     *  time-series has just been re-computed. */
    void data_changed ();


  public:

    /** Sole constructor which sets up a fully populated and operational
     *  object. */
    explicit SD_Envelope_Analyzer (Chart_Data &cd);


    /* Analyzer interface. */

    /** Return a (newly allocated) composite object which includes sliders
     *  for ourself and the \c moving_average. */
    vector <Gtk::Widget*> make_control_widgets ()  override;

    /** Make sure the vertical (price) limits of the \a Range are wide
     *  enough to display the full envelope. */
    void stretch_outline (Time_Series::Range &)  override;

    /** Display the envelope and \c moving_average chart. */
    void graph_draw_hook (Chart_Context &context,
                          Tide_Mark::List &,
                          unsigned number_shares,
                          vector <Tide_Mark::Price_Marker> const &)  override;

    /** Return the signal object on which we emit when anything
     *  changes. */
    sigc::signal<void> &signal_redraw_needed ()  override
    { return redraw_needed_; }


  };  /* End of class SD_Envelope_Analyzer. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__SD_ENVELOPE_ANALYZER__H. */
