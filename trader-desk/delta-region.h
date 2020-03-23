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


#ifndef DMBCS__TRADER_DESK__DELTA_REGION__H
#define DMBCS__TRADER_DESK__DELTA_REGION__H


#include <trader-desk/chart-context.h>
#include <trader-desk/text.h>


/** \file
 *
 *  Declaration of the \c Delta_Region class. */


namespace DMBCS::Trader_Desk {


    /** A \c Delta_Region is a rectangle marked out in (time, price) space
     *  with the intention of displaying the differences (deltas) in time
     *  and price along the edges and giving an indication of the rate of
     *  change of the price with time.  These are the quantities which
     *  represent a trader‘s gains or losses, actual or potential.
     *
     *  The class is only half-autonomous, it holding the extreme values
     *  in (time, price) space but relying on the application to actually
     *  provide and manipulate these values as required (in practice this
     *  requirement is met by the \c Delta_Analyzer class). */

    struct Delta_Region
    {
      /** The colour we use when \c start.price is _less_ than \c
       *  end.price.  The border will be this solid colour, and the fill
       *  will be the same but with a large fraction of transparency. */
      Colour positive_colour;

      /** The alternative to the above colour which we use when \c
       *  start.price is _more_ than \c end.price. */
      Colour negative_colour;

      /** The position of the defining point on the left (earliest time)
       *  edge of the rectangle. */
      Event start;

      /** The position of the defining point on the right (latest time)
       *  edge of the rectangle. */
      Event end;


      
      /** Partial class constructor which establishes the colours that
       *  will be used to render any delta regions.
       *
       *  It is left to the application to complete the specification of
       *  the object: viz the \c start and \c end points. */
      Delta_Region  (Colour const &p, Colour const &n)
        : positive_colour {p}, negative_colour {n}
      {}


      /** Partial class constructor which establishes a uni-colour
       *  realization of the object on a chart.
       *
       *  It is left to the application to complete the specification of
       *  the object: viz the \c start and \c end points. */
      Delta_Region  (Colour const &p)   :   Delta_Region {p, p}
      {}


      /** Show the region on the \a canvas along with summary data
       *  relating to the fact that the display refers to \a number_shares.
       *
       *  Note that the application MUST set the \c start and \c end
       *  corners of the delta region before calling this method. */
      void render (Chart_Context &canvas, unsigned const &number_shares) const;


    } ;  /* End of class Delta_Region. */


  }  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__DELTA_REGION__H. */
