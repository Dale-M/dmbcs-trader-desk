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


#include <trader-desk/delta-analyzer.h>


namespace DMBCS::Trader_Desk {


  Delta_Analyzer::Delta_Analyzer (Chart_Data &cd)
  {
    cd . new_company_signal 
       . connect ([this] { end_place = start_place; });
  }



  void Delta_Analyzer::graph_draw_hook
                              (Chart_Context &canvas,
                               Tide_Mark::List &,
                               unsigned number_shares,
                               vector <Tide_Mark::Price_Marker> const &)
  {
    if (end_place.x != start_place.x  ||  end_place.y != start_place.y)
      {
        if (mouse_active)
          {
            delta_region.start = {canvas.date (start_place.x), 
                                  canvas.value (start_place.y)};
            
            delta_region.end = {canvas.date (end_place.x),
                                canvas.value (end_place.y)};

            if (start_place.x > end_place.x)
              swap (delta_region.start, delta_region.end);
          }
        
        delta_region.render (canvas, number_shares);
      }
  }
    


  bool Delta_Analyzer::button_down (int const x, int const y)
  {
    end_place = start_place = {x, y};
    mouse_active = 1;
    return 1;
  }

    

  bool Delta_Analyzer::button_move (int const x, int const y)  
  { 
    if (! mouse_active)  return 0;
    end_place = {x, y};
    return 1;
  }

    

  bool Delta_Analyzer::button_up (int const, int const)
  {
    if (! mouse_active)  return 0;

    mouse_active = 0;

    /* Needed to remove the box from the screen if the mouse was simply
     * clicked and not moved. */
    redraw_needed . emit ();

    return 1;
  }
    

}  /* End of namespace DMBCS::Trader_Desk. */
