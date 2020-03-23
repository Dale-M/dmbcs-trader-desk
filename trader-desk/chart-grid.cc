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


#include <trader-desk/chart-grid.h>

    
/** \file
 *
 *  Implementation of the \c Chart_Grid class. */


namespace DMBCS::Trader_Desk {


  constexpr chrono::hours const Chart_Grid::DEFAULT_SPAN;


Chart_Grid::Chart_Grid   (Preferences&  P,
			  const Market_Meta_Data&  m)
           :  user_prefs {P},  market {m}
   {
       add_events (Gdk::BUTTON_RELEASE_MASK);
       add (table);
       DB  db  {user_prefs};
       regenerate (db,  P,  1 /* force */);
   }



Chart*  Chart_Grid::find_chart  (const int&  company_seqid)
  {
    auto const c = find_if (begin (chart), end (chart),
                            [company_seqid] (unique_ptr<Chart> const &x)
                                       { return x->data.company_seqid 
                                                          == company_seqid; });

    return  c != end (chart)  ?  c->get ()  :  nullptr;
  }



void   Chart_Grid::regenerate   (DB&  db,  Preferences&  P,  const bool  force)
  {
    if   (force   ||   update_components (market,
                                          db,
                                          (Gtk::Window*) get_toplevel ()))
      {
        table.resize (1, 1);
        chart.clear ();

        auto sql = db.row_query ();

        sql << "   select seqid, rtrim(name) "
            << "     from company "
            << "    where market=" << market.seqid
            << " order by name asc";

        sql.execute ();

        int number_columns = int (ceil (sqrt (sql.number_rows ())));

        chart.reserve (sql.number_rows ());

        int  row  {0};
        int  column  {0};

        for (; sql; ++sql)
          {
            const auto  seqid  {sql.next_entry<int> ()};

            chart.emplace_back (new Chart {Chart::Style::THUMB,  P});

            /* We fix up the zero duration when the widget is exposed, so
             * that we donʼt delay getting the application started by
             * pre-loading tons of data. */
            chart.back ()->data.new_company (db,
                                             seqid,
                                             sql.next_entry<string> (),
                                             chrono::hours {50*24},
                                             market.world_data.close_time);

            table.attach (*chart.back (), column, column + 1, row, row + 1);

            if (0  ==  (column = (column+1) % number_columns))    ++row;
          }

        show_all ();
      }
  }



bool Chart_Grid::on_draw (const Cairo::RefPtr<Cairo::Context>&  C)
  {
    unique_ptr<DB>  db;

    for (auto &c : chart)
      {
        if  (c->data.extremes.start_time  ==  c->data.extremes.end_time)
	  {
	    if (! db)  db  =  make_unique<DB>  (user_prefs);
	    c->data.timeseries__change_span (*db,  DEFAULT_SPAN);
	  }
        else
          table.propagate_draw (*c, C);
      }

    return 1;
  }



bool  Chart_Grid::on_button_release_event  (GdkEventButton *const  event)
  {
     if (chart.empty ())   return 0;

     const Gtk::Allocation  alloc  {chart.front ()->get_allocation ()};

     guint rows, columns;
     table.get_size (rows, columns);

     const int  index = int (event->x) / alloc.get_width ()
                          +  columns * (int (event->y) / alloc.get_height ());

     if (index >= 0  &&  index < int (chart.size ()))
         {
              selection = chart [index].get ();
              selection_signal.emit ();
         }

     return 1;
  }


}  /* End of namespace DMBCS::Trader_Desk. */
