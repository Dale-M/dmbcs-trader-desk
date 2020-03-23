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


#include  "preferences.h"
#include  "alpha-vantage.h"
#include  <fstream>
#include  <regex>


namespace DMBCS::Trader_Desk {



Preferences  Preferences::defaults  ()
     {
       return  
         {
             .trade_cost_offset         =  0.0,
             .trade_cost_factor         =  0.0,
             .database_host             =  "localhost",
             .database_user             =  "trader_desk",
             .database_password         =  "123456",
             .database_instance         =  "trader_desk",
             .database_socket           =  "/run/mysqld/mysqld.sock",
             .database_port             =  3306,
             .market_meta_data_service  =  "https://rdmp.org:9443/trader-desk/",
             .market_data_service       =  "https://www.alphavantage.co/query",
             .market_data_service_key   =  ""
         };
     }



void  dump  (const Preferences&  P)
  {
    dump  (P,  P.file_path.empty ()
                     ?  getenv ("HOME") + string {"/.config/trader-desk.conf"}
                     :  P.file_path);
  }

void  dump  (const Preferences&  P,  const string&  file)
                     {    dump  (P,  ofstream {file});    }

static  Preferences  dump  (Preferences&&  P,  const string&  file)
                     {    dump  (P, file);    return P;    }

void  dump  (const Preferences&  P,  ostream&&  O)
   {
       O << "trade_cost_offset: " << P.trade_cost_offset << "\n"
         << "trade_cost_factor: " << P.trade_cost_factor << "\n"
         << "database_host: "     << P.database_host     << "\n"
         << "database_user: "     << P.database_user     << "\n"
         << "database_password: " << P.database_password << "\n"
         << "database_instance: " << P.database_instance << "\n"
         << "database_socket: "   << P.database_socket   << "\n"
         << "database_port: "     << P.database_port     << "\n"
         << "market_meta_data_service: " << P.market_meta_data_service << "\n"
         << "market_data_service: " << P.market_data_service << "\n"
         << "market_data_service_key: " << P.market_data_service_key << "\n";
   }



string  Preferences::default_file_name  ()
     {   return  getenv ("HOME") + string {"/.config/trader-desk.conf"};   }

Preferences  Preferences::from_default_file  ()
     {   return  from_file  (default_file_name ());   }

Preferences  Preferences::from_file  (const string&  file)
     {
           Preferences  ret
             {[&file]   {  if  (ifstream  F  {file};  F.good ())
                                return from_file (move (F));
                           return  dump  (defaults (),  file);  } ()  };
           ret.file_path  =  file;
           return  ret;
      }

        static  string  read_line  (istream&  I)
        {
               static  regex  R  {" *[^:\n]*: *(.*)$"};
               smatch  M;
               string  line;
               while  (getline (I, line),  I.good ())
                      if  (regex_match  (line,  M,  R))    return  M [1];
               return  {};
        }

Preferences  Preferences::from_file  (istream&&  I)
   {
       Preferences  ret;
       ret.trade_cost_offset =  strtod  (read_line (I).data (),  nullptr);
       ret.trade_cost_factor =  strtod  (read_line (I).data (),  nullptr);
       ret.database_host     =  read_line (I);
       ret.database_user     =  read_line (I);
       ret.database_password =  read_line (I);
       ret.database_instance =  read_line (I);
       ret.database_socket   =  read_line (I);
       ret.database_port     =  atoi  (read_line (I).data ());
       ret.market_meta_data_service  =  read_line (I);
       ret.market_data_service  =  read_line (I);
       ret.market_data_service_key  =  read_line (I);
       return  ret;
   }



bool  database_equal  (const Preferences&  A,  const Preferences&  B)
   {
       return  A.database_host  ==  B.database_host
                   &&  A.database_user  ==  B.database_user
                   &&  A.database_password  ==  B.database_password
                   &&  A.database_instance  ==  B.database_instance
                   &&  A.database_socket  ==  B.database_socket
                   &&  A.database_port  ==  B.database_port;
   }
  


static  void  commit_prefs  (Preferences_Dialog&  D,  Preferences&  P)
{
                                                   /* Pounds to pence. */
  P.trade_cost_offset  =  D.trade_cost_offset.get_value () * 100;
                                                  /* Percent to fraction. */
  P.trade_cost_factor  = D.trade_cost_factor.get_value () / 100.0;
  
  P.database_host  =  D.database_host.get_text ();
  P.database_user  =  D.database_user.get_text ();
  P.database_password  =  D.database_password.get_text ();
  P.database_instance  =  D.database_instance.get_text ();
  P.database_port  =  atoi (D.database_port.get_text ().data ());
  P.database_socket  =  D.database_socket.get_text ();

  P.market_meta_data_service  =  D.market_meta_data_service.get_text ();
  P.market_data_service  =  D.market_data_service.get_text ();
  P.market_data_service_key  =  D.market_data_service_key.get_text ();

  dump (P);
}


        static  void  set_margins  (Gtk::Widget *const  W,  const int  size)
           {
                 W->set_margin_start (size);
                 W->set_margin_end (size);
                 W->set_margin_top (size);
                 W->set_margin_bottom (size);
           }

Preferences_Dialog::Preferences_Dialog
                         (Gtk::Window &parent,
                          Preferences&  P,
                          const string&  alpha_vantage__message)
        : Gtk::Dialog (pgettext ("Window title", "Trader-Desk: Preferences"),
                       parent),
          preferences {P}
  {
    get_vbox ()->set_spacing (20);

    
    /*************  Trade costs   **************************/

    trade_cost_offset.set_increments (1.0, 10.0);
    trade_cost_offset.set_range (0.0, 1000.0);
                                              /* Pence to pounds. */
    trade_cost_offset.set_value (preferences.trade_cost_offset / 100);

    trade_cost_factor.set_increments (1.0, 10.0);
    trade_cost_factor.set_range (0.0, 100.0);
                                           /* Fraction to percent. */
    trade_cost_factor.set_value (preferences.trade_cost_factor * 100.0);

    {
      auto *const  frame  { Gtk::make_managed<Gtk::Frame>
                                      (pgettext ("Label", "Trading costs"))};
      get_vbox ()->pack_start  (*frame,  Gtk::PACK_SHRINK);

      auto *const  grid  {Gtk::make_managed<Gtk::Grid> ()};
      frame->add  (*grid);

      grid->set_column_spacing (20);
      set_margins  (grid,  10);
      grid->set_margin_left (100);

  auto  grid_line  
    {  [grid] (const int  row,         const string&  label,
               Gtk::Widget&  control,  const string&  units)
       {
         grid->attach  (*Gtk::make_managed<Gtk::Label> (label, Gtk::ALIGN_END),
                        0, row);
         grid->attach  (control, 1, row);
         grid->attach  (*Gtk::make_managed<Gtk::Label>
                                                (units,  Gtk::ALIGN_START),
                        2, row);
       }
    };

      grid_line  (0,  pgettext ("Label", "Fixed cost of trading"),
                      trade_cost_offset,
                      pgettext ("Units:Worded:Monetary", "pounds"));
      
      grid_line  (1,  pgettext ("Label", "Proportional cost of trading"),
                      trade_cost_factor,
                      pgettext ("Units:Worded", "percent"));
    }
    

    Gtk::Grid *const  database_  {Gtk::make_managed<Gtk::Grid> ()};
    {
      Gtk::Frame *const df  {Gtk::make_managed<Gtk::Frame>
                                      (pgettext ("Label", "Local database"))};
      get_vbox ()->pack_start (*df, Gtk::PACK_SHRINK);
      df->add  (*database_);
      database_->set_column_spacing  (20);
      set_margins (database_,  10);
      database_->set_margin_left (100);
    }


    auto  create_text_input
      {
       [this, &D = *database_]
         (const int  row,  const string&  label,  Gtk::Entry&  E,
          const string&  initial_value)
         {
               D.attach (*Gtk::make_managed<Gtk::Label> (label, Gtk::ALIGN_END),
                         0, row);
               D.attach (E, 1, row);
               E.set_text  (initial_value);
               E.set_input_purpose  (Gtk::INPUT_PURPOSE_FREE_FORM);
               E.set_hexpand (1);
         }
      };


    create_text_input  (0,  pgettext ("Label", "Database host"),
                        database_host,  preferences.database_host);
    
    create_text_input  (1,  pgettext ("Label", "Database user"),
                        database_user,  preferences.database_user);

    create_text_input  (2,  pgettext ("Label", "Database password"),
                        database_password,  preferences.database_password);
    database_password.set_input_purpose (Gtk::INPUT_PURPOSE_PASSWORD);
    database_password.set_visibility (0);
    database_password.set_invisible_char ('*');

    create_text_input  (3,  pgettext ("Label", "Database instance"),
                        database_instance,  preferences.database_instance);
    
    database_->attach (*Gtk::make_managed<Gtk::Label>
                        (pgettext ("Label", "Database port"),  Gtk::ALIGN_END),
                      0, 4);
    database_port.set_text (to_string (preferences.database_port));
    database_port.set_input_purpose (Gtk::INPUT_PURPOSE_DIGITS);
    database_->attach (database_port, 1, 4);

    create_text_input  (5, pgettext ("Label", "Database socket"),
                        database_socket,  preferences.database_socket);




    Gtk::Grid *const  servers_  {Gtk::make_managed<Gtk::Grid> ()};
    {
      Gtk::Frame *const  F  {Gtk::make_managed<Gtk::Frame>
                                  (pgettext ("Label", "Internet services"))};
      get_vbox ()->pack_start  (*F, Gtk::PACK_SHRINK);
      F->add (*servers_);
      servers_->set_column_spacing (20);
      set_margins (servers_,  10);
      servers_->set_margin_left (100);
    }




    servers_->attach (*Gtk::make_managed<Gtk::Label>
                             (pgettext ("Label", "Market meta-data service"),
                              Gtk::ALIGN_END),
                     0, 0);
    market_meta_data_service.set_hexpand (1);
    market_meta_data_service.set_text (preferences.market_meta_data_service);
    market_meta_data_service.set_width_chars
              (max<int>  (preferences.market_meta_data_service.length (),  40));
    servers_->attach (market_meta_data_service, 1, 0);

    servers_->attach (*Gtk::make_managed<Gtk::Label>
                                 (pgettext ("Label", "Market data service"),
                                  Gtk::ALIGN_END),
                     0, 1);
    market_data_service.set_hexpand (1);
    market_data_service.set_text (preferences.market_data_service);
    servers_->attach (market_data_service, 1, 1);

    servers_->attach (*Gtk::make_managed<Gtk::Label>
                            (pgettext ("Label", "Market data service API key"),
                             Gtk::ALIGN_END),
                     0, 2);
    market_data_service_key.set_hexpand (1);
    market_data_service_key.set_input_purpose (Gtk::INPUT_PURPOSE_PASSWORD);
    market_data_service_key.set_visibility (0);
    market_data_service_key.set_invisible_char ('*');
    market_data_service_key.set_text (preferences.market_data_service_key);
    servers_->attach (market_data_service_key, 1, 2);

    if  (alpha_vantage__message.length ())
      {
        Gtk::Frame *const  F   {Gtk::make_managed<Gtk::Frame>
                                      ("Message from AlphaVantage service:")};
        servers_->attach (*F, 0, 3, 2, 1);
        F->set_margin_top (18);
        Gtk::TextView *const  T  {Gtk::make_managed<Gtk::TextView> ()};
        F->add  (*T);
        T->set_wrap_mode  (Gtk::WRAP_WORD);
        T->get_buffer ()
         ->insert_markup  (T->get_buffer ()->end (),
                           "<span foreground=\"#aa0000\">"
                                       + alpha_vantage__message + "</span>");
      }

    /*  The action buttons.  */
    {
      Gtk::HBox *const  H  {Gtk::make_managed<Gtk::HBox> ()};
      get_vbox ()->pack_start  (*H,  Gtk::PACK_SHRINK);
      Gtk::Button *const  B1  {Gtk::make_managed<Gtk::Button>
                                      (pgettext ("Label", "Commit changes"))};
      H->pack_start (*B1, Gtk::PACK_EXPAND_PADDING);
      B1->signal_clicked ()
         .connect  ([this]  { commit_prefs (*this,  this->preferences);
                              close (); });
      Gtk::Button *const  B2  {Gtk::make_managed<Gtk::Button>
                                             (pgettext ("Label", "Cancel"))};
      H->pack_start (*B2, Gtk::PACK_EXPAND_PADDING);
      B2 -> signal_clicked () . connect  ([this]  { close (); });
    }

    show_all ();
  }


}  /* End of namespace DMBCS::Trader_Desk. */
