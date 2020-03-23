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


#ifndef DMBCS__TRADER_DESK__DATE_RANGE_SCALE__H
#define DMBCS__TRADER_DESK__DATE_RANGE_SCALE__H


#include <trader-desk/scale.h>


namespace DMBCS::Trader_Desk {


  /** A \c Scale which controls the visual span of a \c Time_Series \c
   *  Chart. */

  struct Date_Range_Scale  :  Exponential_Scale
     {
         DB  db;

         /** Sole constructor which provides a fully functioning
          *  object. */
         Date_Range_Scale  (Chart_Data &,  Preferences&);

         /** Called when the user slides the graphical slider. */
         void  on_value_changed  ();

     };  /* End of class Date_Range_Scale. */
  
  
}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__DATE_RANGE_SCALE__H. */
