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


#ifndef DMBCS__TRADER_DESK__COLOUR__H
#define DMBCS__TRADER_DESK__COLOUR__H


#include <cmath>


/** \file
 *
 *  Declaration of the \c Colour class. */


namespace DMBCS::Trader_Desk {


  using namespace std;


  /** A characteristic given to tide lines and text labels. */
  struct Colour
  {
    /** Component of colour, each in the range [0.0, 1.0]. */
    float red, green, blue;

  private:
    /** Sole constructor, provide fully specified object. */
    constexpr Colour (float const &r, float const &g, float const &b)
        : red {r}, green {g}, blue {b}
    {}

  public:
    static const Colour  CURSOR_TIDES;
    static const Colour  COMPANY_NAME_TITLE;
    static const Colour  PRICE_TIDES;
    static const Colour  PROFIT_LINE;
    static const Colour  POSITION_TIDES;
    static const Colour  CHART_BACKGROUND;
    static const Colour  MEAN_TIDE;
    static const Colour  MEAN_GRAPH;
    static const Colour  SD_ENVELOPE;
    static const Colour  ENVELOPE_TIDES;
    static const Colour  PRICE_GRAPH;
    static const Colour  TIME_AXIS;
    static const Colour  PRICE_AXIS;
    static const Colour  MARK_LABEL_BACK;
    static const Colour  NO_DATA_REGION;
    static const Colour  POSITIVE_DELTA;
    static const Colour  NEGATIVE_DELTA;
    static const Colour  DELTA_VALUE;

    /** Extra-special value which indicates that this feature should not
     *  be drawn at all. */
    static const Colour  NO_DISPLAY;


  } ;  /* End of class Colour. */


  /** Two colours compare equal if they look the same to a human observer.
   *  Note that comparing anything with \c NO_DISPLAY (apart from \c
   *  NO_DISPLAY itself) will fail. */
  inline constexpr bool operator== (const Colour  &a, const Colour  &b)
  {
     return abs (a.red - b.red) + abs (a.green - b.green) 
                                + abs (a.blue - b.blue)
              < 0.01;
  }


  /** The exact inverse of the above function. */
  inline constexpr bool operator!= (const Colour  &a, const Colour  &b)
  {  return ! (a == b);  }

    
}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__COLOUR__H. */
