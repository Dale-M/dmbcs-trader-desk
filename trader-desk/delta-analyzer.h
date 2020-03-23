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


#ifndef DMBCS__TRADER_DESK__DELTA_ANALYZER__H
#define DMBCS__TRADER_DESK__DELTA_ANALYZER__H


#include <trader-desk/analyzer.h>
#include <trader-desk/colour.h>
#include <trader-desk/delta-region.h>


namespace DMBCS::Trader_Desk {


  struct Delta_Analyzer  :  Analyzer
  {
    /** Transparent structure to record mouse positions in the GTK
     *  window. */
    struct Place  { int x, y; };

    /** The mouse coordinates of the starting corner of the delta region
     *  (where the mouse button was first pressed). */
    Place start_place {-1, -1};

    /** The mouse coordinates of the end corner of the delta region.  If
     *  this equals \c start_place then the delta region is deemed not
     *  active. */
    Place end_place {-1, -1};

    /** Indicates if the mouse is currently being used to define the
     *  bounds of the delta rectangle. */
    bool mouse_active {0};

    /** Triggered internally if we need the system to re-draw the chart
     *  area. */
    sigc::signal <void>   redraw_needed;

    /** The object which we are controlling, which is responsible for
     *  actually displaying the delta region and its computed parameters
     *  on a chart canvas. */
    Delta_Region delta_region { Colour::POSITIVE_DELTA,
                                Colour::NEGATIVE_DELTA };
    

    /** Set up for operations, and watch the \a chart_data for company
     *  changes when we must ‘blank’ our on-screen presence. */
    explicit Delta_Analyzer (Chart_Data &chart_data);


    /** If there is anything to draw then set up \c delta_region if
     *  necessary and call through that object to get the actual work of
     *  drawing the region done. */
    void graph_draw_hook (Chart_Context &context,
                          Tide_Mark::List &,
                          unsigned number_shares,
                          vector <Tide_Mark::Price_Marker> const &)  override;
    

    /** Provide our signal handler. */
    sigc::signal<void> &signal_redraw_needed ()  override
    {  return redraw_needed;  }


    /** Note that the mouse is now active and set (both) the corners of
     *  the delta region to the position of the mouse. */
    bool button_down (int const x, int const y)  override;
    

    /** If the mouse is active, set the end point of the delta region
     *  definition to the mouse position. */
    bool button_move (int const x, int const y)  override;
    

    /** If the mouse is active, set it to inactive and force a re-draw in
     *  case the region no longer should be displayed, e.g. if the mouse
     *  was just clicked. */
    bool button_up (int const x, int const y)  override;
    

  } ;  /* End of class Delta_Analyzer. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__DELTA_ANALYZER__H. */
