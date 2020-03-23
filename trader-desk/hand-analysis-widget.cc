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


#include <trader-desk/hand-analysis-widget.h>


namespace DMBCS::Trader_Desk {


Hand_Analysis_Widget::Hand_Analysis_Widget 
                  (function<void(Chart_Data&,int const &)> gcfg,
                   Preferences&  P)
    : chart        {Chart::Style::HAND_ANALYSIS,  P},
      company_name {chart.data},
      trade        {P,  chart.data},
      date_range   {chart.data,  P},
      shares       {chart.data},
      get_chart_from_grid {gcfg}
  {
       pack_start (address_bar,    Gtk::PACK_SHRINK,        5);
       pack_start (display_h_box,  Gtk::PACK_EXPAND_WIDGET, 5);

       address_bar.pack_start (company_name, Gtk::PACK_EXPAND_PADDING, 0);
       address_bar.pack_start (trade,        Gtk::PACK_EXPAND_PADDING, 0);

       display_h_box.pack_start (chart,          Gtk::PACK_EXPAND_WIDGET, 5);
       display_h_box.pack_start (controls_h_box, Gtk::PACK_SHRINK, 0);

       controls_h_box.set_spacing (SCALE_SEPARATION);

       controls_h_box.pack_start (shares,     Gtk::PACK_SHRINK, 0);
       controls_h_box.pack_start (date_range, Gtk::PACK_SHRINK, 0);

       for (auto const &w : chart.analyzer->make_control_widgets ())
         controls_h_box.pack_start  (*Gtk::manage (w),  Gtk::PACK_SHRINK);

       chart . analyzer
            -> signal_redraw_needed ()
             . connect ([this] { queue_draw (); });

       company_name . name_change
                    . connect ([this, &P] (int const &seqid)
                               { get_chart_from_grid (chart.data, seqid);
                                 new_chart (P);  });

  }  /* End of method Hand_Analysis_Widget::Hand_Analysis_Widget. */



void  Hand_Analysis_Widget::new_chart  (Preferences&  P)
  {
    chart.data.extremes.start_time
      = chart.data.extremes.end_time
          - chrono::hours (24 * (int) date_range.value ()->get_value ());

    chart.data . prefetch_
                  (P,
                   {chrono::hours {24 * (int)date_range.value ()->get_value ()},
                    chrono::hours {10 * 365 * 24}});

    chart.data.changed_signal.emit ();
  }
    


void  Hand_Analysis_Widget::subsume_selected  (Chart_Grid &grid)
     {
         DB  db  {grid.user_prefs};
          company_name.read_names (db,
                                   grid.market.seqid,
                                   grid.selection->data.company_seqid);
          chart.data.subsume (&grid.selection->data);
          new_chart (grid.user_prefs);
     }


}  /* End of namespace DMBCS::Trader_Desk. */
