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


#include  "../trader-desk/trade-instruction.h"
#include  <sstream>


/** \file
 *
 *  Implementation of \c Trade_Instruction class. */


namespace DMBCS::Trader_Desk {


  Currency_Value Trade_Instruction::value () const
  {
    Currency_Value ret;
    istringstream (entry.get_text ()) >> ret;
    return ret;
  }
    


  void Trade_Instruction::chart_data_changed ()
  {
    if (! chart_data.prices.empty ())
      {
        ostringstream hold;

        hold << (chart_data.latest_price.price > 0.0 
                    ? chart_data.latest_price.price
                    : chart_data.prices.front ().price);

        entry.set_text (hold.str ().c_str ());
      }
  }



  Trade_Instruction::Trade_Instruction (Preferences&  P,
                                        Chart_Data &cd)   :  chart_data (cd)
  {
    pack_start (main_label,       Gtk::PACK_SHRINK, 0);
    pack_start (entry,            Gtk::PACK_SHRINK, 0);
    pack_start (units_label,      Gtk::PACK_SHRINK, 0);

    entry.set_width_chars (4);

    /* Set up the trade_now_button. */
    chart_data_changed ();

    entry.signal_activate ()
      .connect ([this,  &P]
                       { DB  db  {P};
                         chart_data.note_current_price (db,  value ()); });

    chart_data.changed_signal
              .connect ([this] { chart_data_changed (); });
  }


}  /* End of namespace DMBCS::Trader_Desk. */
