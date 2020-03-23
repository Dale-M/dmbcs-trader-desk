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


#include <trader-desk/chart-data.h>
#include <gtkmm.h>


namespace DMBCS::Trader_Desk {


  constexpr const bool  Chart_Data::NO_SIGNAL;
  constexpr const Currency_Value  Chart_Data::NO_POSITION;


void  Chart_Data::timeseries__change_span  (DB&  db,  const Duration&  window)
try  
  {
    const auto  start  {TODAY_MARK - window};

    bool test;
    {lock_guard  l  {prices_mutex};
        test  =  prices.empty ()  ?  1  :  start < prices.back ().time;
    }

    if (test)
      {
          reap_prefetch ();
          if  (last_fetch_time == Time_Point {}   ||   start < last_fetch_time)
              {
                   prices.extend_range  (db,  company_seqid,  window);
                   last_fetch_time  =  start;
              }
      }

    const Time_Series::Range  hold  {extremes};
    update_extremes (window);
    /* We are always in the GTK thread. */
    if (hold != extremes)    changed_signal.emit ();
  }
catch  (Mysql::DB_Connection::Exception&)  {}
  


static  void  new_timeseries (Chart_Data *const  CD,
                              DB&  db,
                              const Duration&  window,
                              const Duration&  market_close_time)
  {
    CD->kill_prefetch ();
    CD->reap_prefetch ();

    const auto  t  {chrono::system_clock::now ()};

    const auto  immediate_window 
                          {min (chrono::duration_cast<chrono::hours> (window),
                                chrono::hours {24*50})};

    CD->prices
      = Time_Series::from_database 
           (db, CD->company_seqid, t, immediate_window, market_close_time);

    CD->last_fetch_time   =   t  -  immediate_window;
    
    CD->update_extremes (window);
    CD->changed_signal.emit ();

    if (window != immediate_window)
            CD->prefetch_ (db.current_preferences,  {window});
  }

void Chart_Data::new_company (DB&  db,
                              const int        company_seqid_,
                              const string&    name,
                              const Duration&  window,
                              const Duration&  market_close_time)
     {
          reap_prefetch ();

          return_subsumed ();
          subsumed_object = nullptr;

          company_seqid = company_seqid_;
          company_name = name;

          prices = Time_Series {market_close_time};
          extremes = Time_Series::Range {};
          extremes.start_time  =  chrono::system_clock::now () - window;

          last_fetch_time = Time_Point {};
          new_timeseries  (this,  db,  window,  market_close_time);
          new_company_signal.emit ();
     }



extern "C"  int  emit_changed_signal  (gpointer  signal)
     {
          ((sigc::signal<void>*) signal)->emit ();
          return 1;
     }



static  void  do_prefetch
                     (DB&  db,  Chart_Data&  CD,  const vector<Duration>  span)
  {
    CD.prefetch_thread_stop = false;

    for (const Duration&  s  :  span)
      while (CD.last_fetch_time   >   TODAY_MARK  -  s)
        {
          const auto  this_time
                          {max (CD.last_fetch_time - chrono::hours {24}*500,
                                TODAY_MARK - s)};

          {lock_guard  l  {CD.prices_mutex};
                CD.prefetch_series  =  new Time_Series {CD.prices};
          }

          CD.prefetch_series->extend_range
                           (db,  CD.company_seqid,  TODAY_MARK - this_time);

          bool need_refresh {0};

          {lock_guard  l  {CD.prices_mutex};
                CD.prices           =  move (*CD.prefetch_series);
                CD.prefetch_series  =  nullptr;
                need_refresh   =  (CD.last_fetch_time > CD.extremes.start_time);
                CD.last_fetch_time  =  this_time;
          }

          if (CD.prefetch_thread_stop)   return;
          
          if (need_refresh)
          {
            CD.update_extreme_prices ();

            /* !!!! Unfortunate that this appears here--and the gtkmm
             *      header--(we have nothing to do with the graphics
             *      plane).  Funny that it doesn't work at the point where
             *      we actually enter the graphics plane, but I guess
             *      other parts of the system react to this signal and
             *      they in turn will trigger the graphics plane.  We are
             *      going to have to investigate all the points where
             *      signals are emitted! (Maybe it is just occurrences of
             *      this one particular signal?) */

            /* ALWAYS outside the GTK thread. */
            gdk_threads_add_idle  (emit_changed_signal, &CD.changed_signal);
          }
        }
  }

void  Chart_Data::prefetch_  (Preferences&  P,
                              const vector<Duration>&  span)
    {
      if (! prefetch_thread)
        prefetch_thread.reset (new thread ([this, span,  &P]
                                      {  DB  db  {P};
                                        do_prefetch  (db,  *this,  span); }));
    }



void Chart_Data::reap_prefetch ()
    {
        if (!  prefetch_thread)  return;

        prefetch_thread->join ();
        prefetch_thread.reset ();
    }



void Chart_Data::update_extreme_prices ()
  {
    lock_guard  l  {prices_mutex};

    const auto  extremes_
                   {prices.get_range (extremes.end_time - extremes.start_time)};

    extremes.min_value = extremes_.min_value;
    extremes.max_value = extremes_.max_value;
  }



void Chart_Data::note_current_price  (DB&  db,  Currency_Value const &value)
  {
      {lock_guard  l  {prices_mutex};

           latest_price  =  {chrono::system_clock::now (),  value};

           /* Expensive! */
           prices.insert (begin (prices), latest_price);

           extremes.end_time = latest_price.time;
      }

      update_extreme_prices ();

      db.quick ()
             << "update company set last_price=" << latest_price.price
             << ", last_price_date=from_unixtime(" 
             << number<chrono::seconds>  (latest_price.time.time_since_epoch ())
             << ") where seqid=" << company_seqid;

      /* Always in GTK thread. */
      changed_signal.emit ();
  }



void  Chart_Data::subsume  (Chart_Data *const  c)
      {
          return_subsumed ();

          subsumed_object = c;
          {
               lock_guard  l  {c->prices_mutex};
               lock_guard  m  {prices_mutex};

               extremes = c->extremes;
               prices = c->prices;
               last_fetch_time = c->last_fetch_time;
          }

          company_seqid   = c->company_seqid;
          company_name    = c->company_name;
          latest_price    = c->latest_price;

          changed_signal.emit ();
          new_company_signal.emit ();
      }



void  Chart_Data::return_subsumed  ()
    {
        if (! subsumed_object)   return;
      
        kill_prefetch ();
        reap_prefetch ();

        {
             lock_guard  l  {subsumed_object->prices_mutex};
             lock_guard  m  {prices_mutex};

             subsumed_object->extremes        = extremes;
             subsumed_object->latest_price    = latest_price;
             subsumed_object->prices          = prices;
             subsumed_object->last_fetch_time = last_fetch_time;
        }

        subsumed_object  =  nullptr;
    }



void  Chart_Data::new_event  (const Event&  e,  const bool&  no_signal)
      {
           {
               lock_guard  l  {prices_mutex};
               prices.insert_event  (latest_price = e);
           }

           if (e.time <= extremes.start_time)   return;
           
           extremes.end_time  = max (extremes.end_time, e.time);

           extremes.min_value = min (extremes.min_value, e.price);
           extremes.max_value = max (extremes.max_value, e.price);

           if (! no_signal)    {    unaccurate = 0;
                                    changed_signal.emit ();    }
      }

    
}  /* End of namespace DMBCS::Trader_Desk. */
