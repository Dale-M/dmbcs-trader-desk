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


#ifndef  DMBCS__TRADER_DESK__WIZARD__H
#define  DMBCS__TRADER_DESK__WIZARD__H


#include   <gtkmm.h>
#include   <trader-desk/auto-config.h>
#include   <trader-desk/preferences.h>
#include   <atomic>


namespace DMBCS::Trader_Desk {

  

#define  t_  pgettext



struct  Prefs  :  Preferences    {    bool    non_default_config_file  {0};
                                      bool    blank_password_forced    {0};
                                      string  root_password            {};    };

  

struct  Database_Prefs  :  Gtk::Grid
      {
           Prefs  prefs;

           Gtk::Entry  host;
           Gtk::Entry  socket;
           Gtk::Entry  port;
           Gtk::Entry  database;
           Gtk::Entry  user;
           Gtk::Entry  password;
           Gtk::Button submit {t_("Label", "Commit change")};

           Database_Prefs  ();
           Database_Prefs&  operator=  (const Prefs&  P);
           Prefs  update_prefs  (Prefs  P);
           Prefs  update_prefs  ()  {   return  prefs = update_prefs (prefs);  }
      };



struct  Wizard  :  Gtk::Assistant
{
  /*  The actual Preferences which we are checking/generating are held
   *  inside the database_prefs object below. */

  bool            force;
  Gtk::Widget*    last_page  {0};

  Gtk::VBox                  configuration_page;
    Gtk::Entry               config_file_entry;
    Gtk::Button     use_default_config_button {t_("Label", "Use _default"), 1};
      sigc::connection       udcb_i;
    Gtk::Button              find_file_button {t_("Label", "_Find file"), 1};
      sigc::connection       ffb_i;
    Gtk::Button              change_button  {t_("Label",  "Change")};
      sigc::connection       cb_i;
    unique_ptr<Gtk::Widget>  create_file_panel;
      Gtk::Button            create_file_button {t_("Label",  "Create file")};
              sigc::connection cfb_i;
    unique_ptr<Gtk::Widget>  non_default_panel;
      Gtk::CheckButton       acknowledge_check {t_("Label", "_Acknowledge"), 1};
        sigc::connection     ac_i;


  Gtk::VBox                    database_access_page;
    Database_Prefs             database_prefs;
    unique_ptr<Gtk::Widget>    no_password_panel;
      Gtk::Button              generate_password_button
                                        {t_("Label", "_Generate password"), 1};
        sigc::connection       gpb_i;
      Gtk::Button              use_no_password_button 
                                        {t_("Label", "_Use no password"), 1};
        sigc::connection       unpb_i;
    unique_ptr<Gtk::Widget>    blank_password_panel;
      Gtk::CheckButton         blank_password_ack_button
                                             {t_("Label", "_Acknowledged"), 1};
        sigc::connection       bpab_i;
    unique_ptr<Gtk::Widget>    cannot_access_panel_1;
    unique_ptr<Gtk::Widget>    cannot_access_panel_2;
    unique_ptr<Gtk::Widget>    privilege_panel;
      Gtk::Entry               database_root_entry;
    unique_ptr<Gtk::Widget>    create_database_panel;
      Gtk::Button              create_database_button
                                          {t_("Label", "Create database"), 1};
        sigc::connection       cdb_i;
    unique_ptr<Gtk::Widget>    create_password_panel;
      Gtk::Button              create_password_button
                                       {t_("Label", "Yes, set password")};
        sigc::connection       cpb_i;
      Gtk::Button              blank_password_button
                                       {t_("Label", "No, use blank password")};
        sigc::connection       bpb_i;
    unique_ptr<Gtk::Widget>    port_problem_panel;
    unique_ptr<Gtk::Widget>    socket_problem_panel;
    unique_ptr<Gtk::Widget>    host_problem_panel;

  Gtk::VBox       tables_page;

  Gtk::VBox                    data_page;
    Gtk::ProgressBar           progress_bar;
      atomic<int>              progress_total;
      atomic<int>              progress_count;
    unique_ptr<Gtk::Widget>    database_ready_panel;
    unique_ptr<Gtk::Widget>    no_data_panel;



        int  page_order  (Gtk::Widget *const,  Gtk::Widget *const);

        sigc::signal<void, Preferences>  signal_wizard_complete;

        Wizard  (const string&  config_file_name,  const bool  force);

        void   get_database_connection  (Prefs);
    };


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined  DMBCS__TRADER_DESK__WIZARD__H. */
