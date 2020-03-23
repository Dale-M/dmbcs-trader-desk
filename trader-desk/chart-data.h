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


#ifndef DMBCS__TRADER_DESK__CHART_DATA__H
#define DMBCS__TRADER_DESK__CHART_DATA__H


#include <trader-desk/time-series.h>
#include <sigc++/sigc++.h>
#include <mutex>
#include <thread>


namespace DMBCS::Trader_Desk {


  /** This class looks after the data which form the graph across a
   *  chart.  It has several features.
   *
   *  1) It holds more data than we need, on the off-chance it might be
   *     wanted later.
   *
   *  2) It actually can get as much data as are available in the database
   *     in a background thread, so that they are available without delay.
   *
   *  3) One \c Chart_Data object is able to take control (subsume) the
   *     data of another \c Chart_Data object.  This is required as the
   *     usual object creation/passing paradigms do not work when objects
   *     are also widgets visible on-screen. */

  struct Chart_Data
  {
    /** A placebo value to indicate that no actual position on the chart
     *  is used. */
    static constexpr Currency_Value  const NO_POSITION  {-1.0};

    
    /** The database's unique identifier for this company. */
    int company_seqid;

    /** A human-readable name string. */
    string company_name;

    /** Access to the prices needs this mutex. */
    mutex prices_mutex;

    /** The actual data we hold.  There may be more here than we actually
     *  need at the present time, i.e. they may go further back in time
     *  than current analyses demand. */
    Time_Series  prices  {chrono::seconds {0}};

    /** The range of data we are interested in.  Will be a subset of the
     *  range of \c prices. */
    Time_Series::Range extremes;

    /** A thread to background-fetch more data. */
    unique_ptr<thread> prefetch_thread;

    /** A holding place for the background thread to do its work. */
    Time_Series *prefetch_series {nullptr};

    /** When this goes \c TRUE, that is taken as a signal to a background
     *  thread to abandon its operations. */
    bool prefetch_thread_stop {false};

    /** If true we consider the entire time-series to be inaccurate, and
     *  show it pink.  Usually used when we are in the process of updating
     *  the latest known prices. */
    bool  unaccurate {0};

    /** We can't rely on the prices time-series to tell us the earliest
     *  datum requested from the database, because the database might not
     *  have gone back that far, and we don't want to keep requesting data
     *  that don't exist.  Hence we keep this record here. */
    Time_Point  last_fetch_time;

    /* !! managed entirely outside the class. */
    /** The number of shares for which analytical data are computed and
     *  shown. */
    unsigned  number_shares {0};

    /** The point in the data space at which a current position was
     *  opened. */
    Event  open_position {0, NO_POSITION};
    
    /** If the user hand-enters a price point, we store that here. */
    Event  latest_price  {0, NO_POSITION};

    /** If we took the data from another object, we store the source here
     *  so that it can subsequently be returned. */
    Chart_Data *subsumed_object {nullptr};

    /** We emit this signal when any aspect of the data are changed. */
    sigc::signal<void>  changed_signal;

    /** This signal is emitted after the data have been completely
     *  subsumed by those for another company. */
    sigc::signal<void>  new_company_signal;

    

    /** If the null constructor is used, then a call of new_company is the
     *  only action that will make any sense; or we could subsume the data
     *  of another \c Chart_Data object. */
    Chart_Data ()  =  default;


    /** The sole working constructor.  Sets the object up ready for
     *  action, but does not actually load in any data at this point. */
    Chart_Data (int const s, string const &n,
                Duration const &market_close_time)
      :  company_seqid {s},
         company_name {n},
         prices {market_close_time}
    {}


    /** The destructor simply cleans up all of its resources. */
    ~Chart_Data ()
    {
      reap_prefetch ();
    }

    
    /** Put a flag up to instruct a running background data pre-fetch
     *  thread to abandon its work and stop. */
    void kill_prefetch () 
    {
      prefetch_thread_stop = true;
    }



    /** Kick off a background thread to get data as far back in time as \a
     *  span. */
    void prefetch_ (Preferences&,  const vector<Duration>&  span);
    

    /** Wait if necessary for the background thread to finish, and then
     *  reap the new data into the existing \c prices time-series. */
    void reap_prefetch ();


    /** Take over the data contained in \a c.  This is specifically for
     *  the case when \a c is a widget on the market thumbnail page, and
     *  we want to analyze the data in detail in the hand-analysis
     *  page. */
    void subsume (Chart_Data *const c);


    /** Give the subsumed data back to the original source, leaving us
     *  bereft until we subsume someone else's data. */
    void return_subsumed ();
    

    /** Assume that the range of prices inside the current range of dates
     *  has changed, and update the recorded upper and lower bounds. */
    void update_extreme_prices ();
    

    /** Re-compute the \c range for the given \a window back from the
     *  current time. */
    void update_extremes (Duration const &window)
    {
      lock_guard<mutex> l {prices_mutex};
      extremes = prices.get_range (window);
    }


    /** Add an event to the time-series corresponding to the \a value at
     *  the current time.  This information is also stored on the company
     *  record in the database. */
    void note_current_price (DB&,  Currency_Value const &value);
    

    /** Causes the span of our data to be changed in real time, i.e. not
     *  in the background thread.  Usually, because of the work of the
     *  background thread, this function will not take very long to
     *  complete the operation. */
    void timeseries__change_span (DB&,  Duration const &window);


    /** Re-initialize the class to hold the data of company with database
     *  identifier \a company_seqid.  The data filling a period \a window
     *  to the start of the current day will be obtained as quickly as
     *  possible, but note that this might not happen as soon as the
     *  function returns; a large window will be broken up and the data
     *  progressively retrieved in a background thread. */
    void new_company (DB&,
                      const int        company_seqid,
                      const string&    name,
                      const Duration&  window,
                      const Duration&  market_close_time);
    

    /** Flag for the following method. */
    static constexpr bool const NO_SIGNAL {1};
    
    /** Insert the new datum \a e into the \c prices time-series, and
     *  update the extremes if the new point is more recent than the start
     *  of the current extremes (the usual case).  A \c changed_signal
     *  will be emitted unless \c NO_SIGNAL is passed as the second
     *  argument. */
    void new_event (Event const &e, bool const &no_signal = 0);

    
  };  /* End of class Chart_Data. */
  
  
}  /* End of namespace DMBCS::Trader_Desk. */


#endif /* Undefined DMBCS__TRADER_DESK__CHART_DATA__H. */
