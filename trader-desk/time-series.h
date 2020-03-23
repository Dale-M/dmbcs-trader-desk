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


#ifndef DMBCS__TRADER_DESK__TIME_SERIES__H
#define DMBCS__TRADER_DESK__TIME_SERIES__H


#include <chrono>
#include <vector>
#include <trader-desk/db.h>


/** \file
 *
 *  Definition of \c Time_Point, Currency_Value, Duration types and the
 *  composed \c Event type, declaration of \c Time_Series class, and
 *  definition of convenience functions \c number<tick> (count ticks in
 *  duration) and \c t (construct a \c time_t from real-world calendar
 *  data). */


namespace DMBCS::Trader_Desk {

  
  /** All trades and closing prices are fixed to a \c Time_Point. */
  typedef  chrono::system_clock::time_point  Time_Point;

  /** All trades and closing prices are expressed as \c
   *  Currency_Value's. */
  typedef  double  Currency_Value;

  /** All time spans are expressed as \c Duration's. */
  typedef  Time_Point::duration  Duration;


  /* The time at which the day started when this application started
   * running.  Whenever we consider closing prices, we regard this as the
   * high-tide mark of the window of closing events. */
  /* !!! We really need to re-evaluate the need for this. */
  extern const Time_Point TODAY_MARK;


  /** Whatever flavour of \c DURATION \a t we have, determine the number
   *  of complete \c TICK's that the duration spans. */
  template <typename TICK, typename DURATION>
  inline constexpr Duration::rep number (DURATION const &t)
  {
    return chrono::duration_cast<TICK> (t) . count ();
  }


  /** Convert from the ‘normal’ notion of calendar date to a \c time_t.
   *  THE ARGUMENTS ARE STREET-WISE VALUES. */
  inline time_t t (int const year, int const month, int const day,
                   int const hour, int const minute, int const second)
  {
    struct tm t {second, minute, hour, day, month - 1, year - 1900, 
                 0, 0, 0, 0, 0};
    return mktime (&t);
  }
    
  /** Convert from the ‘normal’ notion of calendar date to a \c time_t.
   *  THE ARGUMENTS ARE STREET-WISE VALUES. */
  inline time_t t (int const year, int const month, int const day)
  {
    return t (year, month, day, 0, 0, 0);
  }
    


  /** Every trade position open and close, and every market-closing price,
   *  is expressed as an \c Event: simply a time/value pair. */
  struct Event
  {
    Time_Point time;
    Currency_Value price {0.0};

    constexpr Event (const Time_Point&  t,  const Currency_Value&  c)
        :  time {t},  price {c}
    {}

    Event (const time_t  t,  const Currency_Value&  c)
        : Event {chrono::system_clock::from_time_t (t), c}
    {}
      
    constexpr Event () = default;
  };



  /** A vector of (time, price) pairs representing the value history of a
   *  commodity.  It is a class invariant that the vector will ALWAYS be
   *  sorted with later dates at the front, earlier ones at the back (the
   *  motivation being that when the history needs to be extended, it will
   *  invariably be extended backwards in time and then the new data will
   *  simply be appended to the existing data vector). */

  struct Time_Series : vector <Event>
  {
    /** A general-purpose transparent data object used to demarcate a box
     *  in (time x price) space, usually used to indicate the achieved
     *  bounds of a \c Time_Series.  Note that \c start_time is
     *  numerically less than \c end_time. */

    struct Range
    {
      Time_Point     start_time;
      Time_Point     end_time;
      Currency_Value min_value {0};
      Currency_Value max_value {0};

      /** Is the event \a e inside or on the border of this range? */
      constexpr bool  contains  (const Event&  e)  const
      {  return start_time <= e.time  &&  end_time >= e.time  
                   &&  min_value <= e.price  &&  max_value >= e.price;  }

      constexpr bool  operator!=  (const Time_Series::Range&  a)  const
      { return  a.start_time != start_time  ||  a.end_time != end_time
                   ||  a.min_value != min_value  ||  a.max_value != max_value; }
    };


    /** The number of seconds after midnight that the market from which
     *  this time-series derives closes. */
    Duration market_close_time;


    /** Effectively our null constructor, creating an empty time
     *  series. */
    explicit Time_Series (const Duration&  m) : market_close_time {m}
    {}


    /** The one useful (named) class constructor.  Manufacture a new
     *  time-series extracted from the database.  The time-span will be
     *  from \a latest_time and spanning \a window_size'd time interval to
     *  the past.  The \a market_close_time is added to the date of all
     *  data read from the closing prices table. */
    static Time_Series from_database (DB &db,
                                      const int          seqid,
                                      const Time_Point&  latest_time,
                                      const Duration&    window_size,
                                      const Duration&    market_close_time);


    /** Get more data from the database, extending the length of the time
     *  series we are holding further back in time, from our latest datum
     *  to a distance \a window_size back in time. */
    void extend_range (DB &db,
                       const int  seqid,
                       const Duration&  window_size);


    /** Get the value of the commodity at a point in time, linearly
     *  interpolated between the two closest spanning points recorded in
     *  the time-series. */
    Currency_Value interpolated_value (const Time_Point&  date) const;


    /** Get a \c Range object which boxes the time-series data up to an
     *  interval of \a date_range into the past. */
    Range get_range (const Duration&  date_range) const;


    /** Get a \c Range object which boxes the entire time-series data
     *  (this is actually dealt with as a special case inside the above
     *  method). */
    Range get_range () const
                  {  return get_range (chrono::seconds::max ());  }


    /** Insert \a e into its correct place in the current time-series.
     *  This will likely be an expensive operation. */
    void insert_event (const Event&  e);


    /** Produce a new time series by applying a moving average of size \a
     *  window to the \a incoming one, which is left unchanged.  The
     *  returned series only goes back as far as \a earliest_time. */
    static Time_Series compute_moving_average (const Time_Series&  incoming,
                                               const Duration&  window,
                                               const Time_Point& earliest_time);
    

  };  /* End of class Time_Series. */

  
}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__TIME_SERIES__H. */
