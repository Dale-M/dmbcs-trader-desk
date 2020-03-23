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


#ifndef DMBCS__TRADER_DESK__SHARES_SCALE__H
#define DMBCS__TRADER_DESK__SHARES_SCALE__H


#include <trader-desk/scale.h>


/** \file
 *
 *  Declaration of the \c Shares_Scale class. */


namespace DMBCS::Trader_Desk {


  /** Basic example of an \c Exponential_Scale which sets itself according
   *  to the \c Chart_Data, and conversely applies user input from the
   *  graphical slider to the \c Chart_Data. */

  class Shares_Scale : public Exponential_Scale
  {
    /** Called when the user pushes the slider about. */
    void on_value_changed ();

    /** Called when the \c Chart_Data change. */
    void on_data_changed ()  override;
    
  public:

    /** Sole constructor which registers the \a Chart_Data which we watch
     *  over. */
    Shares_Scale (Chart_Data &d);


  };  /* End of class Shares_Scale. */
  
  
}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__SHARES_SCALE__H. */
