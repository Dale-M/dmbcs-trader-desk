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


#ifndef DMBCS__TRADER_DESK__PREFERENCES__H
#define DMBCS__TRADER_DESK__PREFERENCES__H


#include <gtkmm.h>
#include <build-aux/gettext.h>


/** \file
 *
 *  Declaration of the \c Preferences and \c Preferences_Dialog
 *  classes. */


namespace DMBCS::Trader_Desk {


  using namespace  std;


  /** There are a number of parameters which affect the whole of the
   *  trader-desk application, and this flat structure simply contains
   *  those parameters. */

  struct  Preferences
  {
    double  trade_cost_offset;
    double  trade_cost_factor;

    /* MariaDB connection parameters.  */
    string    database_host;
    string    database_user;
    string    database_password;
    string    database_instance;
    string    database_socket;
    uint16_t  database_port;

    /* The RDMP HTTP end-point. */
    string    market_meta_data_service;
    /* The AlphaVantage HTTP end-point... */
    string    market_data_service;
    /* ... and private key. */
    string    market_data_service_key;

    /* Not really a user preference at this time. */
    static constexpr const int  time_horizon  {10};

    /* If set, the file from whence these parameters came. */
    string  file_path  {};

    static  string  default_file_name  ();

    static  Preferences  from_file  (istream&&);
    static  Preferences  from_file  (const string&  file_path);
    static  Preferences  from_default_file  ();

    static  Preferences  defaults  ();
  };


  void  dump  (const Preferences&,  ostream&&);
  void  dump  (const Preferences&,  const string&  file_name);
  void  dump  (const Preferences&);

  bool  database_equal  (const Preferences&,  const Preferences&);



  /** There are a number of parameters which affect the whole of the
   *  trader-desk application, and this dialog box is designed to let the
   *  user edit those global parameters from the ‘Preferences’ item on the
   *  menu.
   *
   *  From the applicationʼs point of view use of this object is very
   *  simple: use the sole constructor to instantiate and then call the \c
   *  run method.  The dialog will take over all GUI operations until it
   *  is closed, and in the meantime will look after itself.
   *
   *  In operation the dialog will directly modify the static data in \c
   *  Position_Analyzer and update the database as soon as the user makes
   *  a change to any entry. */

  struct Preferences_Dialog  :  Gtk::Dialog
  {
    Preferences&  preferences;

    Gtk::SpinButton trade_cost_offset {1.0, 2};
    Gtk::SpinButton trade_cost_factor {1.0, 2};

    /* MariaDB connection parameters.  */
    Gtk::Entry      database_host;
    Gtk::Entry      database_user;
    Gtk::Entry      database_password;
    Gtk::Entry      database_instance;
    Gtk::Entry      database_port;
    Gtk::Entry      database_socket;

    /* The RDMP HTTP end-point. */
    Gtk::Entry      market_meta_data_service;
    /* The AlphaVantage HTTP end-point... */
    Gtk::Entry      market_data_service;
    /* ... and private key. */
    Gtk::Entry      market_data_service_key;
    

    Preferences_Dialog (Gtk::Window &,  Preferences&,
                        const string&  alpha_vantage_message  =  {});


  } ;  /* End of class Preferences. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__PREFERENCES__H. */
