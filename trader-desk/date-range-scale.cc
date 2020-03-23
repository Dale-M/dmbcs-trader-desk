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


#include <trader-desk/date-range-scale.h>


namespace DMBCS::Trader_Desk {


Date_Range_Scale::Date_Range_Scale (Chart_Data &d,  Preferences&  P)
    : Exponential_Scale (d, pgettext ("Label", "Date range = %.0f days"),
                         Gdk::RGBA {"#0000ff"},
                         Gdk::RGBA {"#aaaaff"}, 
                         Gdk::RGBA {"#6666ff"}, 
                         1, 6 * 30, P.time_horizon * 365,
                         50 /* Initial setting. */),
      db  {P}
  {
      d . changed_signal . connect ([this] { on_data_changed (); });

      value_adjustment -> signal_value_changed ()
                        . connect ([this] { on_value_changed (); });
  }



  void Date_Range_Scale::on_value_changed ()
  {
    db.check_connection ();
    data.timeseries__change_span
                 (db,  chrono::hours {24} * int (value ()->get_value ()));
  }


}  /* End of namespace DMBCS::Trader_Desk. */
