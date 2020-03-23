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


#include <trader-desk/moving-average-analyzer.h>


/** \file
 *
 *  Implementation of the \c Moving_Average_Analyzer class. */


namespace DMBCS::Trader_Desk {


  Moving_Average_Analyzer::Moving_Average_Analyzer (Chart_Data &cd)
    : chart_data (cd),
      mean_series {cd.prices.market_close_time}
  {
    chart_data . changed_signal . connect ([this] { compute (); });
  }



  vector <Gtk::Widget*> Moving_Average_Analyzer::make_control_widgets ()
  {
    auto *const scale
      = new Scale {chart_data, 
                   (double) (number<chrono::hours> (mean_window)  /  24)};
    
    scale -> value ()
          -> signal_value_changed ()
           . connect ([this, scale] { control_moved (scale); });
    
    return {scale};
  }



  void Moving_Average_Analyzer::stretch_outline (Time_Series::Range &outline)
  {
    /* !!!!  When we re-visit this, need to ensure that the mean
     *       time-series has been previously computed. */

    auto const range  =  mean_series.get_range ();

    auto const margin =  (std::max (outline.max_value, range.max_value)
                               - std::min (outline.min_value, range.min_value)) 
                           * 0.05;

    outline.max_value =  max (outline.max_value,  range.max_value + margin);

    outline.min_value =  min (outline.min_value,  range.min_value - margin);
  }



  void Moving_Average_Analyzer::graph_draw_hook
                         (Chart_Context &canvas,
                          Tide_Mark::List &marks,
                          unsigned,
                          vector <Tide_Mark::Price_Marker> const &markers)
  {
    canvas . draw_time_series  (mean_series,  Colour::MEAN_GRAPH,  0.5);


    /* The vertical bar which shows the mid-point of the latest window. */
    canvas . set_source_rgb  (Colour::MEAN_GRAPH);

    canvas . move_to  ({canvas.outline.end_time - mean_window / 2,
                        canvas.outline.min_value});

    canvas . line_to  ({canvas.outline.end_time - mean_window / 2,
                        canvas.outline.max_value});

    canvas . cairo -> stroke ();


    /* Put a tide-mark at the mean value at all points in time at which a
     * marker has been specified. */
    for (auto const &marker : markers)
      marks.emplace_back (marker (mean_series.interpolated_value 
                                         (marker (0.0, Colour::MEAN_TIDE).time),
                                  Colour::MEAN_TIDE));
  }



  void  Moving_Average_Analyzer::control_moved  (Scale const *const scale)
  {
    mean_window =  chrono::hours  ((int) scale->value ()->get_value () * 24);
    compute ();
  }



  void Moving_Average_Analyzer::compute ()
  {
    {
      lock_guard<mutex> l {chart_data.prices_mutex};

      mean_series = Time_Series::compute_moving_average 
                                             (chart_data.prices,
                                              mean_window,
                                              chart_data.extremes.start_time);
    }
    
    redraw_needed_.emit ();
  }


}  /* End of namespace DMBCS::Trader_Desk. */
