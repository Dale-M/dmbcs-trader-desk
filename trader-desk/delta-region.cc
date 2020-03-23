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


#include <trader-desk/delta-region.h>
#include <iomanip>


/** \file
 *
 *  Implementation of the \c Delta_Region class. */


namespace DMBCS::Trader_Desk {
    

  template<typename DURATION>
  static string readable_time_span (DURATION const &d)
  {
    namespace C = chrono;

    ostringstream hold;

    hold << fixed << setprecision (2);

    if (d < C::hours {48})
      hold << (number<C::minutes> (d) / 60.0)
           << " hours";
    else if (d < C::hours {10 * 24})
      hold << (number<C::hours> (d) / 24.0)
           << " days";
    else
      hold << (number<C::hours> (d) / (7.0 * 24.0))
           << " weeks";

    return hold.str ();
  }



  void Delta_Region::render (Chart_Context &canvas,
                             unsigned const &number_shares) const
  {
    canvas.set_source_rgb (start.price > end.price ? negative_colour 
                                                   : positive_colour);

    canvas.move_to (start);
    canvas.line_to ({end.time, start.price});
    canvas.line_to (end);
    canvas.line_to ({start.time, end.price});
    canvas.line_to (start);
    canvas.line_to (end);
    canvas.cairo->stroke ();

    canvas.set_source_rgb (start.price > end.price ? negative_colour 
                                                   : positive_colour,
                           0.2);

    canvas.move_to (start);
    canvas.line_to ({end.time, start.price});
    canvas.line_to (end);
    canvas.line_to ({start.time, end.price});
    canvas.cairo->fill ();

    ostringstream percentage;
    percentage << fixed << setprecision (2) 
               << (end.price - start.price) / start.price * 100.0 << '%';

    canvas.add (canvas.text, percentage.str (), Colour::DELTA_VALUE, 
                canvas.x ({end.time, (start.price + end.price) / 2.0}));

    ostringstream delta;
    delta << fixed << setprecision (2) << end.price - start.price;

    canvas.add (canvas.text, delta.str (), Colour::DELTA_VALUE, 
                canvas.x ({end.time, (start.price + end.price) / 2.0}));

    ostringstream cost;
    cost << fixed << setprecision (2) 
         << number_shares * (end.price - start.price) / 100.0;

    canvas.add (canvas.text, cost.str (), Colour::DELTA_VALUE, 
                {canvas.x (start.time) - 60,
                 canvas.y ((start.price + end.price) / 2.0)});

    canvas.add (canvas.text, readable_time_span (end.time - start.time),
                Colour::DELTA_VALUE,
                { (canvas.x (start.time) + canvas.x (end.time)) / 2,
                   canvas.y (min (start.price, end.price))});
  }


}  /* End of namespace DMBCS::Trader_Desk. */
