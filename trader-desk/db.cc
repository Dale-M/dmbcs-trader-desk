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


#include <trader-desk/db.h>
#include <iostream>


namespace DMBCS::Trader_Desk {


  DB::DB  (Preferences&  P)   :  DB_Connection {P},
                                 current_preferences {P}
  {
    
    /*  Not sure if there is some proper way to do this, but things
     *  currently donʼt work unless we drop the current connection to the
     *  database and make a new one. */
    /* reconnect (P); */
  }



  DB&  DB::check_connection  ()
  {
    if (! database_equal (current_preferences,  last_preferences))
      {
        last_preferences  =  current_preferences;
        DB_Connection::reconnect (current_preferences);
      }
    return  *this;
  }


}  /* End of namespace DMBCS::Trader_Desk. */
