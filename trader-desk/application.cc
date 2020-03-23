/*
 * Copyright (c) 2017  Dale Mellor
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


#include <trader-desk/application.h>


namespace DMBCS::Trader_Desk {


  void Application::display_grid (size_t const &g) 
  { 
    if (notebook.get_current_page ()  ==  0)
      {
        hand_analysis->chart.data.update_extremes (Chart_Grid::DEFAULT_SPAN);
        hand_analysis->chart.data.return_subsumed ();
      }
    
    notebook.set_current_page (g + 1);

    close_data_menu->set_sensitive (1);
    last_data_menu->set_sensitive (0);
  }


}  /* End of namespace DMBCS::Trader_Desk. */
