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


#include <trader-desk/time-series.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>


/** \file
 *
 *  Implementation of the \c Time_Series class. */


namespace DMBCS::Trader_Desk {


  Time_Point const TODAY_MARK
              { [] {
                     auto round_days = [] (time_t const &t) 
                       {
                         struct tm tm;  localtime_r (&t, &tm);

                         tm.tm_sec = 0;
                         tm.tm_min = 0;
                         tm.tm_hour = 0;

                         return mktime (&tm);
                       };

                     return chrono::system_clock::from_time_t 
                                 (round_days 
                                     (chrono::system_clock::to_time_t
                                         (chrono::system_clock::now ()))); 
                   } ()
              };
    


  inline   Duration::rep  T  (Time_Point const &t)
  { 
    return number<chrono::seconds> (t.time_since_epoch ()); 
  }



  void Time_Series::insert_event (Event const &e)
  {
    const auto  t  {find_if (begin (),  end (),  [t = e.time] (const Event&  a)
                                                    { return a.time < t; })};

    if (t == end ()) emplace_back (e);
    else             insert (t, e);
  }
    


  void Time_Series::extend_range (DB &db,
                                  int const seqid,
                                  Duration const &window_size)
  {
    auto const earliest_date  =  (empty () ? TODAY_MARK : front ().time)
                                    -  window_size;

    auto const last_time  =  empty ()  ?  TODAY_MARK  
                                       :  back ().time - chrono::seconds (1);

    if (earliest_date  <=  last_time)
      {
        auto const x  =  from_database  (db,
                                         seqid,
                                         last_time,
                                         last_time - earliest_date,
                                         market_close_time);

        insert (end (), std::begin (x), std::end (x));
      }
  }



  Time_Series Time_Series::from_database (DB &db,
                                          int const seqid,
                                          Time_Point const &latest_date,
                                          Duration const &window_size,
                                          Duration const &market_close_time)
  {
    Time_Series  ret  {market_close_time};


    /* If the user has entered a recent price for this stock, we need to
     * splice the value into the time-series we produce (presumably we are
     * only interested in doing this if the date is later than any data we
     * already have). */

    time_t  user_date  {0};
    Currency_Value  user_price  {0.0};

    {
      auto sql = db.row_query ();

      sql << "select UNIX_TIMESTAMP(last_price_date), last_price "
          << "  from company "
          << " where company.seqid=" << seqid;

      sql.execute ();

      if (sql)   {
                    user_date = sql.next_entry (user_date);
                    user_price = sql.next_entry (user_price);
                    ++sql;
                 }
    }

    if (user_date > T (latest_date))   user_date = 0;

    {
      auto sql = db.row_query ();

      sql << "  select unix_timestamp(date), close "
          << "    from prices "
          << "   where company=" << seqid
          << "         and date >= from_unixtime(" 
          <<                               T (latest_date - window_size) << ") "
          << "         and date <= from_unixtime(" << T (latest_date) << ") "
          << "order by date desc";

      sql.execute ();

      if (! sql)   return ret;
      
      auto user_time = chrono::system_clock::from_time_t (user_date);

      for (; sql; ++sql)
        {
          auto const date  =  sql.next_entry<Time_Point> () 
                                                     + ret.market_close_time;

          if (user_time  >  date)
            {
              ret.emplace_back (user_date, user_price);
              user_time = chrono::system_clock::from_time_t (0);
            }
          
          ret.emplace_back  (date,  sql.next_entry (Currency_Value {0.0}));
        }
    }

    return ret;

  }  /* End of from_database method. */



  auto Time_Series::interpolated_value (Time_Point const &date) const 
    -> Currency_Value
  {
    if (empty ())
      return 0;

    auto const t = lower_bound (rbegin (), rend (),
                                Event {date, 0.0},
                                [] (value_type const &a,
                                    value_type const &b)
                                   { return a.time < b.time; });

    if (t == rbegin ())
      return t->price;

    return (t-1)->price + (t->price - (t-1)->price)
                             * ((T (date) - T ((t-1)->time))
                                  / (double) (T (t->time) - T ((t-1)->time)));
  }



  auto  Time_Series::get_range (Duration const &date_range) const  ->  Range
  {
    Range ret;

    if (empty ())
      return ret;
    
    ret.end_time = front ().time;

    const_iterator const end_
      =  date_range == chrono::seconds::max ()
          ?  end ()
          :  std::upper_bound (begin (),
                               end (),
                               ret.end_time - date_range,
                               [] (Time_Point const &val, value_type const &a)
                                               { return a.time < val; });

    auto const e
      = minmax_element (begin (),
                        end_,
                        [] (value_type const &a, value_type const &b)
                                   { return a.price < b.price; });

    ret.min_value = e.first->price;
    ret.max_value = e.second->price;

    if (end_ != begin ())   ret.start_time = (end_ - 1)->time;
    else                    ret.start_time = end_->time;

    return ret;
  }



  Time_Series Time_Series::compute_moving_average (Time_Series const &in,
                                                   Duration const &window,
                                                   Time_Point const &earliest)
  {
    if (in.size () < 2)
      return in;

    Time_Series ret {in.market_close_time};

    auto const forward_window_size = window / 2;
    auto const backward_window_size = window - forward_window_size;

    auto const start_time = earliest - window;

    /* These iterators run through _increasing_ dates. */
    auto window_front = in.rbegin ();
    while  (window_front->time  <  start_time   &&   window_front != in.rend ())
      ++window_front;
    auto window_back = window_front;

    int count = 0;
    double sum = 0.0;

    for (;
         window_front != in.rend ()
           &&  window_front->time < window_back->time + forward_window_size;
         ++ window_front)
      {
        sum += window_front->price;
        ++count;
      }

    ret.emplace_back (window_back->time,
                      count > 0 ? sum / count : 0.0);

    for (auto i = window_back + 1;  i != in.rend ();  ++i)
      {
        for (;
             window_front != in.rend ()
               &&  window_front->time < i->time + forward_window_size;
             ++window_front)
          {
            sum += window_front->price;
            ++ count;
          }

        for (;
             window_back != window_front
                     &&  window_back->time < i->time - backward_window_size;
             ++window_back)
          {
            sum -= window_back->price;
            -- count;
          }

        ret.emplace_back (i->time,  count > 0 ? sum / count : 0.0);
      }

    reverse (std::begin (ret), std::end (ret));

    return ret;
  }


}  /* End of namespace DMBCS::Trader_Desk. */
