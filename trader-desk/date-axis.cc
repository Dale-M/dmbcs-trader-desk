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


#include <trader-desk/date-axis.h>


namespace DMBCS::Trader_Desk { namespace Date_Axis {

  
  inline constexpr time_t unix (Duration const &d)
  {
    return number<chrono::seconds> (d);
  }


  inline tm *squash (tm *const time, int count)
  {
    if (count-- > 0) time->tm_sec = 0;
    if (count-- > 0) time->tm_min = 0;
    if (count-- > 0) time->tm_hour = 0;
    if (count-- > 0) time->tm_mday = 1;
    if (count-- > 0) time->tm_mon = 0;

    return time;
  }

  inline Duration round_count (Duration const &t, int const &count)
  {
    time_t t_ = unix (t);
    
    return chrono::seconds (mktime (squash (localtime (&t_), count)));
  }
  

  Duration round_year   (Duration t) { return round_count (t, 5); }
  Duration round_month  (Duration t) { return round_count (t, 4); }


  Duration round_week   (Duration t)
  {
    time_t t_ = unix (t);
    tm date = *localtime (&t_);

    squash (&date, 3);

    mktime (&date);

    /* Go back to the previous Monday (tm_mday = 1). */
    if (date.tm_wday == 0)
      date.tm_mday -= 6;
    else
      date.tm_mday -= date.tm_wday - 1;

    return chrono::seconds (mktime (&date));
  }


  Duration round_day    (Duration t) { return round_count (t, 3); }
  Duration round_hour   (Duration t) { return round_count (t, 2); }
  Duration round_minute (Duration t) { return round_count (t, 1); }
  Duration round_second (Duration t) { return t; }
    
 
} }  /* End of namespace DMBCS::Trader_Desk::Date_Axis. */
