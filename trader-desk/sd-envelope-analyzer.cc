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


#include <numeric>
#include <trader-desk/sd-envelope-analyzer.h>


/** \file
 *
 *  Implementation of the \c SD_Envelope_Analyzer class. */


namespace DMBCS::Trader_Desk {
    

  SD_Envelope_Analyzer::SD_Envelope_Analyzer (Chart_Data &cd)
    : moving_average {cd}
  {
    moving_average  .  signal_redraw_needed ()
                    .  connect ([this] { data_changed (); });
  }



  vector <Gtk::Widget*> SD_Envelope_Analyzer::make_control_widgets ()
  {
    vector <Gtk::Widget*> ret = moving_average.make_control_widgets ();

    Scale *const s = new Scale {moving_average.chart_data,
                                envelope_width};

    s -> value () -> signal_value_changed ()
                   . connect ([this, s] { control_moved (s); } );

    ret.emplace_back (s);

    return ret;
  }



  template <typename X>  inline X sq (X const &x)  { return x*x; }


  static Currency_Value standard_deviation_ (Time_Series const &t,
                                             Time_Series const &mean,
                                             Time_Point  const &earliest_time)
  {
    if (mean.size () < 2)
      return 0.0;

    auto const end_  =  std::upper_bound 
                              (begin (mean),  end (mean),
                               earliest_time,
                               [] (Time_Point const &val,
                                   Time_Series::value_type const &a)
                                           { return a.time < val; });

    return std::sqrt (std::inner_product (begin (mean),  end_,
                                          begin (t),
                                          Currency_Value {0.0},
                                          plus<Currency_Value> {},
                                          [] (Event const &a, Event const &b)
                                             { return sq (a.price - b.price); })
                         / (mean.size () - 1));
  }



  void SD_Envelope_Analyzer::data_changed ()
  {
    {
      lock_guard<mutex> l {moving_average.chart_data.prices_mutex};

      standard_deviation 
        = standard_deviation_ (moving_average.chart_data.prices,
                               moving_average.mean_series,
                               moving_average.chart_data.extremes.start_time);
    }
    
    redraw_needed_.emit ();
  }



  void SD_Envelope_Analyzer::control_moved (Scale const *const scale)
  {
    if (abs (envelope_width - scale->value ()->get_value ())  >  1.0e-3)
      {
        envelope_width = scale->value ()->get_value ();
        redraw_needed_.emit ();
      }
  }



  void SD_Envelope_Analyzer::stretch_outline (Time_Series::Range &outline)
  {
    auto const range  = moving_average.mean_series.get_range ();
    auto const margin = (outline.max_value - outline.min_value) * 0.05;

    outline.max_value = max (outline.max_value,
                             range.max_value
                                     + envelope_width * standard_deviation
                                     + margin);

    outline.min_value = min (outline.min_value,
                             range.min_value
                                     - envelope_width * standard_deviation
                                     - margin);
  }



  void SD_Envelope_Analyzer::graph_draw_hook
                          (Chart_Context &context,
                           Tide_Mark::List &marks,
                           unsigned number_shares,
                           vector <Tide_Mark::Price_Marker> const &markers)
  {
    if (moving_average.mean_series.empty ())
      return;

    context.set_source_rgb (Colour::SD_ENVELOPE);

    context.move_to (moving_average.mean_series.front ());

    auto const envelope = envelope_width * standard_deviation;

    auto i  =  begin (moving_average.mean_series);

    for (;
         i != end (moving_average.mean_series)
                 &&  i->time >= context.outline.start_time;
         ++i)
      context.line_to ({i->time, i->price + envelope});

    for (--i; i >= begin (moving_average.mean_series); --i)
      context.line_to ({i->time, i->price - envelope});

    context.cairo->fill ();

    for (auto const &t : markers)
      {
        auto const mean 
          = moving_average.mean_series
                          .interpolated_value (t (0.0, Colour::MEAN_TIDE).time);

        marks.emplace_back (t (mean - envelope, Colour::ENVELOPE_TIDES));
        marks.emplace_back (t (mean + envelope, Colour::ENVELOPE_TIDES));
      }

    moving_average . graph_draw_hook (context, marks, number_shares, markers);
  }


}  /* End of namespace DMBCS::Trader_Desk. */
