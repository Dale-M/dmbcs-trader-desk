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


#ifndef DMBCS__TRADER_DESK__DATE_AXIS__H
#define DMBCS__TRADER_DESK__DATE_AXIS__H


#include <gtkmm.h>
#include <trader-desk/time-series.h>


namespace DMBCS::Trader_Desk {


  /** Really part of the implementation of \c Chart to help with drawing
   *  the date axis along the bottom edge.  The class provides information
   *  about the possible spacings between tick marks, and methods which
   *  round any date down to the nearest tick. */

  namespace Date_Axis
  {

    /** A \c Discretization represents a level of detail to display on a
     *  time axis.  The core concept is the real_interval between tick
     *  marks, but this is not always consistent (lengths of months, leap
     *  years), so we consider an increased \c interval which is bigger
     *  than any interval which might occur and smaller than any double
     *  interval, and then we use the system's calendar functions to \c
     *  round_down to the last appropriate real date. */

    struct Discretization 
    {
      /** Slightly bigger than the largest real interval between ticks
       *  (remember that months and years vary), but definitely smaller
       *  than two ticks.  */
      Duration interval;

      /** The real interval between ticks. */
      Duration real_interval;

      /** A \c strftime format which provides appropriate labels on the
       *  ticks.  A \c nullptr here represents a sentinel place-holder for
       *  the end of a list of discretizations. */
      char const *const format;

      /** Function which will round the \a Duration down to the nearest
       *  tick. */
      Duration (*round_down_) (Duration);

      /** Convenience wrapper around above function. */
      Duration round_down (Duration const &d)  { return (*round_down_) (d); }

      /** As above, but round the \c Time_Point \a t down to the nearest
       *  tick. */
      Time_Point round_down (Time_Point t) const
      {
        return chrono::system_clock::from_time_t (0)
                     + (*round_down_) (t.time_since_epoch ());
      }
    };

    
    /* The specialized rounding functions.  Since we use the C library
     * calendar functions, we always work in terms of duration (as
     * seconds) since the Unix epoch. */

    Duration round_year   (Duration t);
    Duration round_month  (Duration t);
    Duration round_week   (Duration t);
    Duration round_day    (Duration t);
    Duration round_hour   (Duration t);
    Duration round_minute (Duration t);
    Duration round_second (Duration t);

    
    namespace C = chrono;


    /* Null-terminated array of discretization levels which we use, in
     * increasing granularity. */

    static constexpr const Discretization discretization []
             = { { C::seconds (1), 
                   C::seconds (1),
                   "%H:%M:%S", &round_second },
                 { C::seconds (70), 
                   C::seconds (60),
                   "%H:%M", &round_minute },
                 { C::seconds (6 * 60),
                   C::seconds (5 * 60),
                   "%H:%M", &round_minute },
                 { C::seconds (70 * 60),
                   C::seconds (60 * 60),
                   "%H:00", &round_hour },
                 { C::seconds (25 * 60 * 60),
                   C::seconds (24 * 60 * 60),
                   "%d", &round_day },
                 { C::seconds (8 * 24 * 60 * 60),
                   C::seconds (7 * 24 * 60 * 60),
                   "%d", &round_week },
                 { C::seconds (35 * 24 * 60 * 60),
                   C::seconds (31 * 24 * 60 * 60),
                   "%b", &round_month },
                 { C::seconds (380 * 24 * 60 * 60),
                   C::seconds (365 * 24 * 60 * 60),
                   "%Y", &round_year },
                 { C::seconds (0), C::seconds (0), nullptr, nullptr } };


  }  /* End of namespace Date_Axis. */
  
  
}  /* End of namespace DMBCS::Trader_Desk. */


#endif /* Undefined DMBCS__TRADER_DESK__DATE_AXIS__H. */
