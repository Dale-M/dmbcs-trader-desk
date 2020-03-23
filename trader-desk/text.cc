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


#include <trader-desk/text.h>
#include <limits>


namespace DMBCS::Trader_Desk {
    

  inline double clamp  (double const &x, double const &l, double const &h)
  {
    return max (min (x, h), l);
  }



  static double boundary_stress (Text::Item const &i,
                                 Text::Boundary const &b,
                                 double const &critical)
  {
    double const ret
            = i.size.y * (clamp (i.placement.x + i.size.x - b.right,
                                 0.0, i.size.x)
                           + clamp (b.left - i.placement.x, 0.0, i.size.x))
            + i.size.x * (clamp (b.top - i.placement.y, 0.0, i.size.y)
                        + clamp (i.placement.y + i.size.y - b.bottom,
                                 0.0, i.size.y));

    return  ret > -numeric_limits<double>::min () ? 2 * ret + critical
                                                  : 0.0;
  }
        


  static float overlap (Text::Item const &a, Text::Item const &b,
                        float const &critical_value)
  {
    static double const pad = 6.0;

    if (a.placement.x >= b.placement.x + b.size.x + pad
        ||  a.placement.x + a.size.x  <=  b.placement.x - pad
        ||  a.placement.y >= b.placement.y + b.size.y
        ||  a.placement.y + a.size.y  <=  b.placement.y)
      return 0;

    auto const right  = min (a.placement.x + a.size.x,
                             b.placement.x + b.size.x) + pad;
    auto const left   = max (a.placement.x, b.placement.x) - pad;
    auto const top    = max (a.placement.y, b.placement.y);
    auto const bottom = min (a.placement.y + a.size.y,
                             b.placement.y + b.size.y);

    return critical_value + sqrt ((right - left) * (bottom - top));
  }



  inline double squash (double const &x)
  {
    return 1.0 - exp (-x / 100.0);
  }


  inline double tension (Text::Item const &i)
  {
    return squash (hypot (i.native_position.x - i.placement.x,
                          i.native_position.y - i.placement.y));
  }


  inline void bring_inside (Text::Item &i, Text::Boundary const &b)
  {
    i.placement = {clamp (i.placement.x, b.left, b.right - i.size.x),
                   clamp (i.placement.y, b.top, b.bottom - i.size.y)};
  }



  double Text::compute_tension (double const &critical_value,
                                Boundary const &boundary) const
  {
    double acc {0.0};

    for (auto a = begin (items); a != end (items); ++a)
      {
        for (auto b = a + 1; b != end (items); ++b)
          acc +=  overlap (*a, *b, critical_value);

        acc +=  tension (*a)  +  boundary_stress (*a, boundary, critical_value);
      }

    return acc;
  }



  void Text::shuffle (double shift_size,
                      double const &shift_limit,
                      Boundary const &boundary)
  {
    if (items.empty ())   return;

    for (; shift_size > shift_limit; shift_size /= 2.0)
      {
        for (auto &i : items)
          bring_inside (i, boundary);

        auto const acceptable_limit = items.size ();
        auto initial_tension = compute_tension (acceptable_limit, boundary);

        for (;;)
          {
            Item *hot_item {nullptr};
            enum { up, down, left, right} best_direction {up};
            double best_tension = numeric_limits<double>::max ();

            {
              auto test = [this, acceptable_limit, &best_tension,
                           &hot_item, &best_direction, boundary]
                                    (Item &i,
                                     Place const &shift,
                                     decltype (best_direction) const &direction)
                    {
                      i.placement += shift;
                      auto const tension = compute_tension (acceptable_limit,
                                                            boundary);
                      if (tension < best_tension)
                        {
                          hot_item = &i;
                          best_direction = direction;
                          best_tension = tension;
                        }
                    };

              for (auto &i : items)
                {
                  test  (i,  Place {shift_size, 0},           right);
                  test  (i,  Place {- 2 * shift_size, 0},     left );
                  test  (i,  Place {shift_size, shift_size},  up   );
                  test  (i,  Place {0, - 2 * shift_size},     down );

                  i.placement  +=  Place {0, shift_size};
                }
            }
            

            if (! hot_item)  return;


            switch (best_direction)
              {
              case up:    hot_item->placement += {0, shift_size};   break;
              case down:  hot_item->placement += {0, -shift_size};  break;
              case left:  hot_item->placement += {-shift_size, 0};  break;
              case right: hot_item->placement += {shift_size, 0};   break;
              }

            /* Has the tension decreased?  If not we should give up,
             * regardless of the acceptable limit. */

            if (initial_tension - best_tension  
                     <  numeric_limits<decltype(initial_tension)>::epsilon ())
              break;

            if ((initial_tension = best_tension)  <  acceptable_limit)
              break;

          }  /* Loop back for further improvements. */

      } /* End of loop over shift_size. */

  }  /* End of ‘shuffle’ function. */


}  /* End of namespace DMBCS::Trader_Desk. */
