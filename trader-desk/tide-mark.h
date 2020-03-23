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


#ifndef DMBCS__TRADER_DESK__TIDE_MARK__H
#define DMBCS__TRADER_DESK__TIDE_MARK__H


#include <trader-desk/colour.h>
#include <trader-desk/time-series.h>


/** \file
 *
 *  Definition and complete inline implementation of the \c Tide_Mark
 *  class. */


namespace DMBCS::Trader_Desk {


  /** A \c Tide_Mark is simply a point in (time, price)-space (i.e. an \c
   *  Event) through which we draw horizontal and vertical cross-hairs.
   *  The main requirement is to be able to pass a list of interesting
   *  points in time around the program--the current mouse position,
   *  current time, position-entry time--and then have each sub-system
   *  furnish a set of price marks; this is the purpose of the \c
   *  Price_Marker function type which is an object with time bound and
   *  which takes a price as an argument, with the end product being an
   *  actual \c Tide_Mark at that (time, price) point.
   *
   *  Note further how the horizontal and vertical lines have distinctive
   *  colours. */

  struct Tide_Mark : Event
  {
    /** The container type we (the application) use for containers of \c
     *  Tide_Mark's. */
    typedef vector<Tide_Mark> List;

    /** The type of a function which manufactures a \c Tide_Mark using
     *  intrinsically predefined values for the time and time-tide
     *  colour. */
    typedef function <Tide_Mark (Currency_Value const &, Colour const &)>
              Price_Marker;

    /** Colour of price-wise tide-line. */
    Colour value_colour;

    /** Colour of time-wise tide-line. */
    Colour temporal_colour;


  private:

    /*  Construction must be done via the \c tide_mark function below.
     *
     *  Rather than constructing these objects in one go, applications
     *  must first use \c price_marker() to manufacture us a function,
     *  which in turn manufactures a \c Tide_Mark. */
    constexpr Tide_Mark (Event const &e, Colour const &v, Colour const &t)
      : Event {e}, value_colour {v}, temporal_colour {t}
    {}
    

    /*  No reason to make copies, so guard against accidentally doing
     *  so. */
    Tide_Mark (Tide_Mark const &) = delete;
    Tide_Mark &operator= (Tide_Mark const &) = delete;


  public:

    /*  Moves are necessary as these items are stored in \c vector's. */
    Tide_Mark (Tide_Mark &&) = default;
    Tide_Mark &operator= (Tide_Mark &&) = default;
    

    /** A factory method which produces a \c Price_Marker, which in turn
     *  produces \c Tide_Marks with event time and time-tide colours fixed
     *  at \a time and \a time_colour, respectively. */
    static Price_Marker price_marker (Time_Point  const &time,
                                      Colour      const &time_colour)
    {
      return [time, time_colour]  (Currency_Value const &v, Colour const &c)
             {  return Tide_Mark {{time, v}, c, time_colour};  };
    }
    

  } ;  /* End of class Tide_Mark. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__TIDE_MARK__H. */
