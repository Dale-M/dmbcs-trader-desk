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


#include <trader-desk/shares-scale.h>
#include <iomanip>


namespace DMBCS::Trader_Desk {


  Shares_Scale::Shares_Scale (Chart_Data &d)
                : Exponential_Scale (d, 
                                     pgettext ("Label", 
                                               "Number of shares = %.0f"),
                                     Gdk::RGBA ("#00dd00"),
                                     Gdk::RGBA ("#aaffaa"),
                                     Gdk::RGBA ("#66ff66"),
                                     1, 1000, 10000,
                                     1)
  {
    d.changed_signal . connect  ([this] { on_data_changed (); });

    value_adjustment -> signal_value_changed ()
                     . connect  ([this] { on_value_changed (); });
  }



  void Shares_Scale::on_data_changed ()
  {
    if (data.open_position.price < 0)
      scale.show ();
    else
      scale.hide ();

    if (data.number_shares  !=  unsigned (value ()->get_value ()))
      {
        set_value (data.number_shares);
        scale.get_adjustment ()  ->  value_changed ();
      }
  }



  void Shares_Scale::on_value_changed ()
  {
    if (data.number_shares !=  (unsigned) (value ()->get_value () + 0.5))
      {
        data.number_shares =  (unsigned) (value ()->get_value () + 0.5);
        data.update_extreme_prices ();
        data.changed_signal.emit ();
      }
  }


}  /* End of namespace DMBCS::Trader_Desk. */
