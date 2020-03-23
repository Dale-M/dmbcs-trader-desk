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


#include  "alpha-vantage--monitor.h"
#include  "application.h"
#include  "update-closing-prices.h"
#include  <dmbcs-market-data-api.h>


/** \file
 *
 *  Implementation of the \c Application::update_closing_prices
 *  mega-method. */


namespace DMBCS::Trader_Desk {


struct Progress_Dialog  :  Gtk::Dialog
    {
         Gtk::ProgressBar progress_bar;
         Gtk::Label       company_name;

         explicit  Progress_Dialog  (Gtk::Window &parent);
    };



Progress_Dialog::Progress_Dialog  (Gtk::Window &parent)
      : Gtk::Dialog (pgettext ("Window title", "Updating database"), parent, 1)
    {
        company_name.set_ellipsize (Pango::ELLIPSIZE_END);
        company_name.set_alignment (Gtk::ALIGN_START);
        get_vbox ()->pack_start
          (*Gtk::make_managed<Gtk::Label>
                    (pgettext ("Information", 
                               "trader-desk: Fetching data from Internet")));
        get_vbox ()->pack_start (company_name, Gtk::PACK_EXPAND_WIDGET);
        get_vbox ()->pack_start (progress_bar, Gtk::PACK_SHRINK);
        add_button (Gtk::Stock::STOP, 0);
        get_vbox ()->set_spacing (10);
        set_default_size (400, 10);
        show_all ();
    }

    

extern "C"  int  queue_draw  (Gtk::Widget *const  w)
            {    w->queue_draw ();   return 0;   }



          struct  no_connection_args
                 {   Gtk::Window*  window;   string  message;   };

extern "C"  int  run_no_internet_connection_message
                                         (const no_connection_args *const  A)
    {
        Gtk::MessageDialog  {*A->window,  A->message,  0,  Gtk::MESSAGE_WARNING}
             .run ();
        delete  A;
        return  0;
    }



extern "C"  int  delete_widget  (Gtk::Widget *const  w)
            {    delete w;   return 0;   }



  struct pulse_progress_bar_args
    {  Progress_Dialog*  dialog;   double  fraction;   string  company_name;  };

extern "C"  int  pulse_progress_bar  (const pulse_progress_bar_args *const  A)
   {
      A -> dialog -> progress_bar . set_fraction  (A->fraction);
      A -> dialog -> company_name . set_text      (A->company_name);
      delete  A;
      return 0;
   }

  

  struct preferences_error_args  {  Application*  app;  string  message;  };

extern "C"  int  preferences_error_  (const preferences_error_args *const  A)
   {
       A->app->preferences_error (A->message);
       delete  A;
       return  0;
   }



static  void  grid_injector  (Chart_Grid&  grid,
                              Chart**  current_chart,
                              const Update_Closing_Prices::Data&  data)
  {
    if (! *current_chart)
         *current_chart = grid.find_chart (data.company_seqid);

    if  (*current_chart)
      (*current_chart) -> data
                        . new_event 
                          ({chrono::system_clock::from_time_t 
                                           (t (data.year, data.month, data.day))
                              + (*current_chart)->data.prices.market_close_time,
                            data.close},
                           Chart_Data::NO_SIGNAL);
  }

void Application::update_closing_prices ()
  {
    const size_t  a  {(size_t) notebook.get_current_page ()};

    if (a < 1  ||  a > market_grids.size ())   return;

    Chart_Grid&  grid  {*market_grids [a - 1]};

    try   {
              DB  db  {user_prefs};
              grid.regenerate (db,  user_prefs);
          }
    catch (const Market_Data_Api::Bad_Communication&  e)
          {
              Gtk::MessageDialog {*window, e.what (), 0, Gtk::MESSAGE_WARNING}
                       .run ();
              return;
          }

    for (auto&  c  :  grid.chart)    c->data.unaccurate = 1;

    grid.queue_draw ();

    std::thread
      {[this, progress_dialog = new Progress_Dialog (*window), &grid]
       {
         sigc::connection  call_id;

         try
           {
             DB  db  {user_prefs};
             Update_Closing_Prices::Work  update  {.market  =  grid.market};

             call_id  =  progress_dialog->signal_response ()
                                         .connect ([&update] (int)
                                                   {  update.stop = true;  });

             /** We donʼt want to hunt for the correct chart every time a
              *  new datum is reported to us, so only do the search when
              *  this is \c nullptr and re-use the last search result
              *  (stored here) otherwise. */
             Chart*  current_chart  {nullptr};

             do_update
                (update,
                 db,
                 user_prefs,

                 /* Progress callback. */
                 [progress_dialog, &grid]
                          (const double&  x,
                           const Update_Closing_Prices::Company&  company)
                    {   gdk_threads_add_idle
                               ((int(*)(void*))  pulse_progress_bar,
                                new  pulse_progress_bar_args
                                    {progress_dialog,  x,  company.name});   },

                 /* Datum injector. */
                 [&db, &grid, &current_chart] 
                            (Update_Closing_Prices::Data const &data)
                    {    if (data.close == 0)   return;
                         sql_injector  (db,  data);
                         grid_injector (grid, &current_chart, data);   },

                 /* Company_done. */
                 [&grid, &current_chart] (int const &company_seqid)
                    {    if (! current_chart)
                              current_chart = grid.find_chart (company_seqid);

                         current_chart->data.extremes
                            = current_chart->data
                                            .prices
                                            .get_range (chrono::hours (50*24));

                         current_chart->data.unaccurate = 0;
                         gdk_threads_add_idle  ((int(*)(void*))queue_draw,
                                                current_chart);
                         current_chart = nullptr;       });
           }

         catch (const Price_Server::Bad_API_Key&  E)
           {
             gdk_threads_add_idle
                       ((int(*)(void*)) preferences_error_,
                        new  preferences_error_args  {this,  E.what ()});
           }

         catch (const Update_Closing_Prices::No_Connection&  E)
           {
             gdk_threads_add_idle
                       ((int(*)(void*))  run_no_internet_connection_message,
                        new  no_connection_args  {window, E.what ()});
           }

         call_id.disconnect ();
         gdk_threads_add_idle  ((int(*)(void*))  delete_widget,
                                progress_dialog);
       }}
    .detach ();
  }


}  /* End of namespace DMBCS::Trader_Desk. */
