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


#ifndef DMBCS__TRADER_DESK__TRADE_INSTRUCTION__H
#define DMBCS__TRADER_DESK__TRADE_INSTRUCTION__H


#include <trader-desk/chart-data.h>
#include <gtkmm.h>


/** \file
 *
 *  Declaration of the \c Trade_Instruction class. */


namespace DMBCS::Trader_Desk {


  /** A component of a \c Hand_Analysis_Widget which entirely looks after
   *  itself (interacting with the rest of the program through signals),
   *  and allows the user to set the current price of a commodity, and to
   *  indicate that /buy/ and /sell/ actions have taken place on that
   *  current price.
   *
   *  It is expected that the owning \c Hand_Analysis_Widget will maintain
   *  the \c Chart_Data object given to the constructor for the lifetime
   *  of one of these objects. */

  class Trade_Instruction : public Gtk::HBox
  {
    /** The data we have on the commodity being watched.  We react to any
     *  changes in these via the \c chart_data_changed method. */
    Chart_Data &chart_data;
    
    /** String printed in front of information box. */
    Gtk::Label main_label {"Current price "};

    /** String printed after information box. */
    Gtk::Label units_label {"p "};

    /** The user edit box.  Updates from the user are sent directly to \c
     *  chart_data. */
    Gtk::Entry entry;


    /** The user has pressed the buy/sell button. */
    void trade_now_pressed ();

    /** A change in the chart (price history of commodity of interest) has
     *  taken place. */
    void chart_data_changed ();
  

    /** Get the contents of the edit box as a currency (pence) value. */
    Currency_Value value () const;


    /* Duplicating one of these objects is both pointless and dangerous. */
    Trade_Instruction (Trade_Instruction const &) = delete;
    Trade_Instruction &operator= (Trade_Instruction const &) = delete;
    Trade_Instruction (Trade_Instruction &&) = delete;
    Trade_Instruction &operator= (Trade_Instruction &&) = delete;

  
  public:

    /** Sole constructor which registers the \a chart_data object we
     *  follow, and sets up the GTK widget ready for use. */
    Trade_Instruction (Preferences&,  Chart_Data &);

    
  };  /* End of class Trade_Instruction. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /*  Undefined DMBCS__TRADER_DESK__TRADE_INSTRUCTION__H. */
