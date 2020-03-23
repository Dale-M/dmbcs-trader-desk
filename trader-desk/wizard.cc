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


#include   <trader-desk/wizard.h>
#include   <trader-desk/db.h>
#include   <fstream>
#include   <iostream>
#include   <random>
#include   <regex>
#include   <thread>


namespace DMBCS::Trader_Desk {
  

Database_Prefs&  Database_Prefs::operator=  (const Prefs&  P)
    {
        prefs = P;
        host     .set_text (P.database_host);
        socket   .set_text (P.database_socket);
        port     .set_text (to_string (P.database_port));
        database .set_text (P.database_instance);
        user     .set_text (P.database_user);
        password .set_text (P.database_password);
        submit.set_sensitive (0);
        return  *this;
    }



Prefs  Database_Prefs::update_prefs  (Prefs  P)
    {
        P.database_host      =  host.get_text ();
        P.database_socket    =  socket.get_text ();
        P.database_port      =  atoi (port.get_text ().c_str ());
        P.database_instance  =  database.get_text ();
        P.database_user      =  user.get_text ();
        P.database_password  =  password.get_text ();
        return  P;
    }

    

        static  void  entry_  (Database_Prefs&  P,    Gtk::Entry&  E,
                               const string&  label,  const int  row)
           {
              P.attach (*Gtk::manage (new Gtk::Label {label + ": "}), 0, row);
              E.set_width_chars (30);
              E.signal_changed ().connect ([&P] {P.submit.set_sensitive ();});
              E.signal_activate ().connect ([&P] {P.submit.clicked ();});
              P.attach  (E,  1,  row);
           }

Database_Prefs::Database_Prefs  ()
  {
    const auto entry
          {  [this] (Gtk::Entry&  E,  const string&  L,  const int  R)
                      {   return  entry_ (*this, E, L, R);  }   };

      entry  (host,   t_("Label", "Host"),   0);
      entry  (socket, t_("Label", "Socket"), 1);

      attach (*Gtk::make_managed<Gtk::Label>
                                         (t_("Label", "Port") + string {": "}),
              0, 2);
      port.signal_changed ().connect ([this] {submit.set_sensitive ();});
      port.signal_activate ().connect ([this] {submit.clicked ();});
      attach (port, 1, 2);

      entry  (database, t_("Label", "Database"), 3);

      entry  (user,     t_("Label", "User"),     4);
      user.signal_changed ().connect ([this] { prefs.blank_password_forced = 0;
                                               password.set_sensitive (); });

      entry  (password, t_("Label", "Password"), 5);
      
      auto *const  B  {Gtk::make_managed<Gtk::HBox> ()};
      attach  (*B, 0, 6, 2, 1);
      B->pack_start  (submit,  Gtk::PACK_EXPAND_PADDING);
      submit.set_sensitive (0);
      show_all ();
  }



static  tuple<unique_ptr<Gtk::Widget>, Gtk::TextBuffer*, Gtk::HBox*>
        make_panel  (Gtk::VBox&  V,  const string&  nature)
  {
      auto  H  {make_unique<Gtk::HBox>  (0, 15)};
      V.pack_start (*H, Gtk::PACK_SHRINK);
      H->pack_start  (*Gtk::make_managed<Gtk::Image> ("dialog-" + nature,
                                                      Gtk::ICON_SIZE_DIALOG),
                      Gtk::PACK_SHRINK);
      auto *const  V2  {Gtk::make_managed<Gtk::VBox> (0, 15)};
      H->pack_start  (*V2);
      auto *const  T  {Gtk::make_managed<Gtk::TextView> ()};
      V2->pack_start  (*T, Gtk::PACK_SHRINK);
      T->set_wrap_mode (Gtk::WRAP_WORD);
      T->override_background_color  (Gdk::RGBA {"rgba(0,0,0,0)"});
      auto *const  H2  {Gtk::make_managed<Gtk::HBox> ()};
      V2->pack_start  (*H2, Gtk::PACK_SHRINK);
      return  {move (H),  T->get_buffer ().get (),  H2};
  }



static  void  make_configuration_page  (Wizard&  W)
   {
      W.configuration_page.set_spacing  (65);
      auto  V  {Gtk::make_managed<Gtk::VBox>  (0,  15)};
      W.configuration_page.pack_start  (*V,  Gtk::PACK_SHRINK);
      V->pack_start  (*Gtk::make_managed<Gtk::Label>
                              (t_("Label", "Using config file") + string {":"}),
                      Gtk::PACK_SHRINK);
      V->pack_start  (W.config_file_entry,  Gtk::PACK_SHRINK);
      W.config_file_entry.set_width_chars  (30);
      auto *const  H  {Gtk::make_managed<Gtk::HBox>  (0,  15)};
      V->pack_start  (*H,  Gtk::PACK_SHRINK);
      H->pack_start  (W.use_default_config_button,  Gtk::PACK_EXPAND_PADDING);
      H->pack_start  (W.find_file_button,  Gtk::PACK_EXPAND_PADDING);
      H->pack_start  (W.change_button,  Gtk::PACK_EXPAND_PADDING);

      W.configuration_page.show_all ();

      {auto  [P, T, H]  {make_panel  (W.configuration_page,  "warning")};
         W.create_file_panel  =  move (P);
         T->set_text (gettext ("This file does not exist, shall I create it?"));
         H->pack_start (W.create_file_button,  Gtk::PACK_EXPAND_PADDING);
      }

      {auto  [P, T, H]  {make_panel  (W.configuration_page,  "warning")};
         W.non_default_panel  =  move (P);
         T->set_text (gettext ("This is not the default config file path.  "
                               "This means that you will either have to copy "
                               "the file to the default location, or use the "
                               "-c option to specify the file whenever you "
                               "run the trader-desk application."));
         H->pack_start  (W.acknowledge_check,  Gtk::PACK_SHRINK);
      }

      W.config_file_entry.signal_changed ()
                         .connect ([&W] { W.change_button.set_sensitive (); });
      W.config_file_entry.signal_activate ()
                         .connect ([&W] { W.change_button.clicked (); });
   }



template<typename  Callback>
static  void  connect_clicked
                        (sigc::connection&  C,  Gtk::Button&  B,  Callback  F)
    {
        C.disconnect ();
        C = sigc::connection {B.signal_clicked ().connect  (F)};
    }
  


        static  void  check_config_file
              (Wizard&  W,  const string&  file_name,  bool  not_default);

static  void  config_file_chooser  (Wizard&  W)
  {
      Gtk::FileChooserDialog  D  {W,
                                  t_("Label", "trader-desk config file"),
                                  Gtk::FILE_CHOOSER_ACTION_SAVE};
      D.add_button  (t_("Label", "Open"),  Gtk::RESPONSE_ACCEPT);
      D.set_default_response  (Gtk::RESPONSE_ACCEPT);
      D.signal_response ()
       .connect ([&W, &D] (const int  response)
                 {  D.hide ();
                    if (response == Gtk::RESPONSE_ACCEPT)
                           check_config_file  (W,  D.get_filename (), 0);   });
      D.run ();
  }



static  void  check_config_file
               (Wizard&  W,  const string&  file_name,  bool  not_default = 0)
  {
    W.config_file_entry.set_text  (file_name);
    W.config_file_entry
     .set_width_chars  (max<int> (W.config_file_entry.get_width_chars (),
                                  file_name.length ()));
    W.use_default_config_button
     .set_sensitive (file_name != Preferences::default_file_name ());
    connect_clicked
             (W.udcb_i,
              W.use_default_config_button,
              [&W, not_default]
              { check_config_file
                   (W,  Preferences::default_file_name (),  not_default);  });
    connect_clicked   (W.ffb_i,
                       W.find_file_button,
                       [&W] { config_file_chooser (W); });
    W.change_button.set_sensitive (0);
    connect_clicked
           (W.cb_i,
            W.change_button,
            [&W, not_default]
            {  check_config_file
                   (W,  W.config_file_entry.get_text (),  not_default);  });
    ifstream  i  {file_name};
    const bool  file_exists  {i.good ()};
    if  (file_exists)
      {
        i.close ();
        W.database_prefs.prefs  =  Prefs {Preferences::from_file (file_name)};
        W.create_file_panel->hide ();
      }
    else
      {
        W.create_file_panel->show_all ();
        connect_clicked
             (W.cfb_i,
              W.create_file_button,
              [&W, &E = W.config_file_entry, not_default] 
                   {  Preferences::from_file (E.get_text ());
                      check_config_file  (W,  E.get_text (),  not_default);});
      }
    
    if  (file_name  !=  Preferences::default_file_name ())
      {
        ((Gtk::Image*) ((Gtk::Container*) W.non_default_panel.get ())
         ->get_children ().front ())
          ->set_from_icon_name (string {"dialog-"}
                                    + (not_default ? "information" : "warning"),
                                Gtk::ICON_SIZE_DIALOG);
        W.non_default_panel->show_all ();
        W.acknowledge_check.set_active  (not_default);
        connect_clicked
          (W.ac_i,  W.acknowledge_check,
           [&W]  {  check_config_file  (W,
                                        W.config_file_entry.get_text (),
                                        W.acknowledge_check.get_active ());  });
      }
    else
      W.non_default_panel->hide ();

    const bool  done
      {file_exists  &&  (not_default
                           || file_name == Preferences::default_file_name ())};
    W.set_page_complete  (W.configuration_page,  done);
    if (W.force  &&  done)   W.next_page ();
  }



static  unique_ptr<Gtk::Widget>  absolute_message  (Gtk::VBox&  page,
                                                    const string&  message)
   {
     auto  [P,  T,  H]  {make_panel  (page,  "warning")};
     T->set_text  (message);
     return  move (P);
   }
  


static  void  make_database_page  (Wizard&  W,  Gtk::VBox&  page)
{
  page.pack_start  (W.database_prefs,  Gtk::PACK_SHRINK);
  page.show_all ();

  {auto  [P,  T,  H]  {  make_panel  (page,  "warning")  };
    W.no_password_panel  =  move (P);
    T->set_text  (gettext ("No password has been set for this user.  You are "
                           "strongly recommended to set one.  Enter and commit "
                           "a new password above, let me generate a random "
                           "password for you, or press skip if you are sure "
                           "you donʼt need one."));
    H->pack_start (W.generate_password_button,  Gtk::PACK_EXPAND_PADDING);
    H->pack_start (W.use_no_password_button,  Gtk::PACK_EXPAND_PADDING);
  }
  
  {auto  [P,  T,  H]  {  make_panel  (page,  "information")  };
    W.blank_password_panel  =  move (P);
    T->set_text  (gettext ("No database password is being used for this "
                           "user."));
    H->pack_start (W.blank_password_ack_button,  Gtk::PACK_SHRINK);
  }

  W.cannot_access_panel_1
    = absolute_message
         (page,
          gettext ("Cannot access the database with this account.  Please "
                   "correct the user and password fields above.  You may also "
                   "insert the root password, if you know it, and I will try "
                   "to help sort the problem out; you will need to do this if "
                   "you are trying to create a new user."));

  W.cannot_access_panel_2
    = absolute_message
         (page,
          gettext ("Cannot access the database with this account.  Please "
                   "correct the user and password fields above."));


  {auto  [P,  text,  buttons] {make_panel (W.database_access_page, "warning")};
    W.privilege_panel  =  move (P);
    text->set_text (gettext ("This user does not have access to this database "
                             "instance.  Either modify the settings above or "
                             "provide the root password in the box below"));
    buttons->pack_start (*Gtk::make_managed<Gtk::Label>
                                    (t_("Label", "Database root password: ")));
    W.database_root_entry.set_width_chars (30);
    buttons->pack_start  (W.database_root_entry,  Gtk::PACK_SHRINK);
  }

  W.database_root_entry
      .signal_activate ()
      .connect ([&W]
                {  W.database_prefs.prefs.root_password
                                           =  W.database_root_entry.get_text ();
                   W.get_database_connection (W.database_prefs.prefs);});


  {auto  [P, T, H]  {make_panel  (W.database_access_page,  "warning")};
    W.create_database_panel  =  move (P);
    T->set_text  (gettext  ("This database instance does not exist.  Either "
                            "edit the entry above or select the create "
                            "button."));
    H->pack_start (W.create_database_button,  Gtk::PACK_EXPAND_PADDING);
  }


  {auto  [P,  T,  H]  { make_panel  (W.database_access_page,  "warning") };
    W.create_password_panel  =  move (P);
    T->set_text  (gettext ("The user password is not set in the database.  "
                           "Do you want me to do that for you now?"));
    H->pack_start (W.blank_password_button,  Gtk::PACK_EXPAND_PADDING);
    H->pack_start (W.create_password_button,  Gtk::PACK_EXPAND_PADDING);
  }
  

  W.port_problem_panel
    =  absolute_message  (page,
                          gettext ("Cannot contact a database on that PORT."));

  W.socket_problem_panel
    =  absolute_message
          (page,  gettext ("Cannot contact a local database on that SOCKET."));

  W.host_problem_panel
    =  absolute_message
         (page, gettext ("The database host you specified cannot be reached."));
}



static  string  slurp_file  (ifstream&&  input)
  {
    input.seekg (0,  ios::end);
    string  line  (input.tellg (),  char {});
    input.seekg (0,  ios::beg);
    input.read  (line.data (),  line.length ());
    return line;
  }



static  void  prepare_data_page  (Wizard&);

static  gboolean  progress_ticker  (gpointer  W_)
          {
              Wizard&  W  {*(Wizard*) W_};
              /* !!  Thread local. */
              static int old_progress  =  0;
              if  (W.progress_total  <  0)
                  {
                      prepare_data_page  (W);
                      return  0;
                  }
              if  (W.progress_count != old_progress)
                  {
                      old_progress  =  W.progress_count;
                      if (W.progress_total == 0)
                          W.progress_bar.pulse ();
                      else
                          W.progress_bar.set_fraction
                              (W.progress_count / (double) W.progress_total);
                  }
              return  1;
          }



static  void  prepare_data_page  (Wizard&  W)
    {
      for (auto *const i  :  W.data_page.get_children ())
             W.data_page.remove (*i);

      DB  db  {W.database_prefs.prefs};
      if  (db.scalar_result<int> (0, "select count(*) from company")  !=  0)
        {
          if  (W.force)   {    W.close ();  /* next_page (); */    return;    }
          W.set_page_complete  (W.data_page);
          W.data_page.pack_start  (*W.database_ready_panel,  Gtk::PACK_SHRINK);
          W.data_page.show_all ();
          return;
        }

      ifstream  I  {PKGDATADIR "/data.sql.xz"};
      if  (! I.good ())
        {
          if  (W.force)   {    W.close ();  /* next_page (); */    return;    }
          W.set_page_complete  (W.data_page);
          W.data_page.pack_start  (*W.no_data_panel,  Gtk::PACK_SHRINK);
          W.data_page.show_all ();
          return;
        }


      W.data_page.pack_start
                      (*Gtk::make_managed<Gtk::Label>
                              (pgettext ("Label", "Pre-populating database")),
                       Gtk::PACK_SHRINK);
      W.data_page.pack_start  (W.progress_bar,  Gtk::PACK_SHRINK);
      W.show_all ();

      gdk_threads_add_timeout  (250,  progress_ticker,  &W);

      std::thread 
        {
         [&W]
         {
           DB  db  {W.database_prefs.prefs};
           /*  !!  Would prefer to call out to xz library. */
           system  ("cat " PKGDATADIR "/data.sql.xz "
                       "|  xz -d "
                       ">  /tmp/trader-desk--data.sql");
           const string  sql
                  {slurp_file (ifstream {"/tmp/trader-desk--data.sql"})};
           unlink  ("/tmp/trader-desk--data.sql");
           W.progress_total  =  0;
           W.progress_count  =  0;
           for (size_t  a  {0}; ; )
               {
                    const size_t  b  {sql.find ("INSERT", a)};
                    if  (b == sql.npos)   break;
                    ++W.progress_count;
                    a  =  b + 1;
               }
           W.progress_total  =  W.progress_count.load ();
           W.progress_count  =  0;
           for (size_t  a  {0};  ;  )
               {
                    const size_t  b  {sql.find ("INSERT", a)};
                    if  (b == sql.npos)   break;
                    const size_t  c  {sql.find (";", b)};
                    db.instruction (sql.substr (b, c-b));
                    ++W.progress_count;
                    a  =  b + 1;
               }
           W.progress_total  =  -1;
         }
       } . detach ();
    }



static  bool  do_tables  (Wizard&  W,  const bool  build)
try
  {
    Prefs&  P  {W.database_prefs.prefs};
    DB  db  {P};

    if (1  ==  db.scalar_result (0, "select version from global"))
      return  1;

    if (! build)  return 0;

    db.instruction ("create table global "
                             "(version int, "
                              "last_markets_update datetime default 0)");

    db.instruction ("insert into global "
                         "set version=1");
    
    db.instruction ("create table company "
                             "(seqid int(6) primary key auto_increment, "
                              "name varchar(50) not null, "
                              "symbol varchar(6) not null, "
                              "market int(6) not null default 0, "
                              "last_price float, "
                              "last_price_date datetime, "
                              "last_close_date date not null "
                                                    "default '0000-00-00')");

    db.instruction ("create table prices "
                             "(date date, "
                              "company int(6), "
                              "open float, high float, low float, "
                              "close float, "
                              "volume int(11), adjusted_close float)");

    db.instruction ("alter table prices "
                         "add primary key (date, company)");
    
    db.instruction ("create table market "
                             "(seqid int(6) primary key auto_increment, "
                              "symbol varchar(6), "
                              "name varchar(36), "
                              "component_extension varchar(6), "
                              "last_update datetime default 0, "
                              "tracked bool default 0, "
                              "close_time time)");

    db.instruction ("create table alphavantage_ticks "
                             "(time int(11) primary key)");
    
    return  1;
}
catch  (exception&)     {  return  0;   }



static  void  prepare_table_page  (Wizard&  W)
    {
        do_tables (W, 1);
        W.set_page_complete  (W.tables_page);
        if  (W.force)   {    W.next_page ();    return;    }
        Gtk::TextView *const  T  {Gtk::make_managed<Gtk::TextView> ()};
        W.tables_page.add  (*T);
        T->get_buffer ()->set_text ("The database tables are in place.");
        W.tables_page.show_all ();
    }



int  Wizard::page_order  (Gtk::Widget *const  a,  Gtk::Widget *const  b)
     {
         if (a == 0   ||   b == 0   ||   a == b)   return 0;
         if (a == (Gtk::Widget*)&configuration_page)
                      return  -1;
         if (a == (Gtk::Widget*)&database_access_page)
                      return  b == (Gtk::Widget*)&configuration_page ? 1 : -1;
         if (a == (Gtk::Widget*)&tables_page)
                      return  b == (Gtk::Widget*)&data_page ? -1 : 1;
         return  1;
     }



static  void  make_data_page  (Wizard&  W,  Gtk::VBox&  page)
{
  {auto  [P,  T,  H]  {  make_panel  (page,  "information")  };
    W.no_data_panel  =  move (P);
    T->set_text  (gettext ("There are no data available to pre-populate "
                           "the database.  You will be invited to import "
                           "data from the Internet."));
  }

  {auto  [P,  T,  H]  {  make_panel  (page,  "information")  };
    W.database_ready_panel  =  move (P);
    T->set_text  (gettext ("The database is populated with some data."));
  }
}
  


Wizard::Wizard  (const string&  config_file_name,  const bool  f)   :  force {f}
    {
       const auto  add_page  
         {   [&W=*this] (Gtk::VBox&  V,  const string&  title)
                    {    W.append_page (V);
                         W.set_page_type (V, Gtk::ASSISTANT_PAGE_CONTENT);
                         W.set_page_title (V, title);
                         V.set_spacing (40);  } };

       add_page  (configuration_page,  t_("Label", "Config file"));
       make_configuration_page  (*this);

       add_page  (database_access_page,  t_("Label", "Database access"));
       make_database_page  (*this,  database_access_page);
       
       add_page  (tables_page, t_("Label", "Tables"));

       add_page  (data_page,  t_("Label", "Historical data"));
       make_data_page  (*this,  data_page);
       set_page_type  (data_page,  Gtk::ASSISTANT_PAGE_CONFIRM);

       database_prefs.submit
                     .signal_clicked ()
                     .connect ([this]
                               {  database_prefs.update_prefs ();
                                  get_database_connection
                                            (database_prefs.prefs);  });

       signal_close ()
         .connect ([this] { signal_wizard_complete.emit (database_prefs.prefs);
                            hide ();  });

       signal_cancel ().connect ([this] {hide ();});
       show_all ();
       set_icon_from_file (PKGDATADIR "/trader-desk.png");
       signal_prepare ()
         .connect ([this]  (Gtk::Widget *const  page)
                   {   if (page_order (last_page, page) > 0)  force = 0;
                       last_page  =  page;
                       if (page == (Gtk::Widget*) &configuration_page)
                           check_config_file  (*this,
                                               database_prefs.prefs.file_path);
                       else if (page == (Gtk::Widget*) &database_access_page)
                           get_database_connection (database_prefs.prefs);
                       else if (page == (Gtk::Widget*) &tables_page)
                           prepare_table_page  (*this);
                       else if (page == (Gtk::Widget*) &data_page)
                           prepare_data_page (*this);});
       check_config_file  (*this,  config_file_name);
    }



static  bool  host_problem  (Wizard&  W,  const string&  error)
      {
            /* !!!!  This probably wouldn't work if the machine is using a
             *       language other than English?  */
            static const regex  unknown_host_RE  {".*unknown.*server host.*",
                                                  regex::icase};
            if  (! regex_match  (error,  unknown_host_RE))    return  0;
            W.host_problem_panel->show_all ();
            return  1;
      }



static  bool  socket_problem  (Wizard&  W,  const string&  error)
     {
          static const regex  bad_socket_RE
                                {".*can.t.*connect.*local.*server.*socket.*",
                                 regex::icase};
          if  (!  regex_match  (error,  bad_socket_RE))     return 0;
          W.socket_problem_panel->show_all ();
          return  1;
     }


static  bool  port_problem  (Wizard&  W,  const string&  error)
     {
          static const regex  bad_port_RE   {".*can.t.*connect.*server.*",
                                             regex::icase};
          if  (!  regex_match  (error,  bad_port_RE))   return  0;
          W.port_problem_panel->show_all ();
          return  1;
     }



         static  void  do_set_password_on_database  (Database_Prefs&  P)
         {
             string  old_password  {};
             swap  (old_password,  P.prefs.database_password);
             DB {P.prefs}
                .instruction ("set password=password('" + old_password + "')");
             swap  (old_password,  P.prefs.database_password);
         }

static  bool  set_password_on_database  (Wizard&  W,  Database_Prefs&  P)
   {
       if (W.force)   {   do_set_password_on_database (P);
                          return 1;   }
     
       W.create_password_panel->show_all ();
       connect_clicked  (W.bpb_i,  W.blank_password_button,
                         [&W,  &P]  { P.prefs.database_password  =  "";
                                      W.get_database_connection  (P.prefs);  });
       connect_clicked  (W.cpb_i,  W.create_password_button,
                         [&W,  &P]
                    {   do_set_password_on_database  (P);
                        W.get_database_connection  (P.prefs);    });
       P.password.set_sensitive (1);
       return  0;
   }



static  tuple <Prefs,  unique_ptr<DB>>  get_root_access  (Prefs  P)
    {
        Prefs  root_P  {P};
        root_P.database_user  =  "root";
        root_P.database_instance  =  "";

        try   {   if (P.root_password.length ())
                       {
                          root_P.database_password  =  P.root_password;
                          return  {move (P),  make_unique<DB> (root_P)};
                       }   }
        catch  (exception&)   {}

        root_P.database_password  =  P.database_password;

        try   {    P.root_password  =  P.database_password;
                   P.database_password  =  {};
                   return   {move (P),  make_unique<DB>  (root_P)};  }
        catch  (exception&)   {}

        root_P.database_password  =  {};

        try  {   P.root_password  =  {};
                 return   {move (P),  make_unique<DB>  (root_P)};  }
        catch  (exception&  E)   { cerr << "XXX: " << E.what () << "\n"; }

        return  {move (P),  unique_ptr<DB> {}};
    }
  


static  void  create_database_as_user  (Database_Prefs&  P)
    {
         Preferences  prefs  {P.prefs};
         prefs.database_instance  =  "";
         DB  db  {prefs};
         auto  I  {db.instruction ()};
         I  <<  "create database "  <<  P.prefs.database_instance
            << " character set 'utf8'";
         I.execute ();
    }


  
static  void  create_database_as_root  (Database_Prefs&  P)
  {
     Prefs  prefs  {P.prefs};
     prefs.database_instance  =  "";
     auto  [p,  root_db]   {get_root_access  (prefs)};
     P.prefs.root_password  =  p.root_password;
     if  (! root_db)  create_database_as_user  (P);
     root_db->instruction ("create database " + P.prefs.database_instance
                               + " character set 'utf8'");
  }



static  bool  database_instance_problem  (Wizard&  W,  Database_Prefs&  P)
{
  Prefs  prefs  {P.prefs};
  prefs.database_instance  =  "";
  auto  [p, root_db]  {get_root_access (prefs)};
  prefs.root_password  =  p.root_password;
  
  if  (root_db)
    {
      P.prefs.root_password  =  prefs.root_password;
      if  (root_db->scalar_result<int> (0,
                                        "select count(*) "
                                          "from information_schema.schemata "
                                         "where schema_name='%s'",
                                        P.prefs.database_instance.data ()))
          return  1;
      if (W.force)    {   create_database_as_root (P);    return 1;    }
      W.create_database_panel->show_all ();
      connect_clicked  (W.cdb_i,  W.create_database_button,
                        [&W,  &P]  {   create_database_as_root (P);  });
      return  0;
    }
      
  try
    {
      DB  user_db  {prefs};
      if  (user_db.scalar_result<int> (0,
                                       "select count(*) "
                                         "from information_schema.schemata "
                                        "where schema_name='%s'",
                                       P.prefs.database_instance.data ()))
          return  1;
      if  (W.force)
        {
          try   {   create_database_as_user  (P);    return 1;   }
          catch  (exception&)
            {
              W.privilege_panel->show_all ();
              return 0;
            }
        }
      W.create_database_panel->show_all ();
      connect_clicked  (W.cdb_i,  W.create_database_button,
                        [&W,  &P]  {   create_database_as_user (P);  });
    }
  catch (exception&  E)  {}

  return  0;
}

  

inline  void  grant_privileges
             (DB&  root_db,  const string&  database,  const string&  user)
     {   root_db.instruction ("grant all on " + database + ".* to " + user);   }

static  void  grant_privileges  (DB&  root_db,  Wizard&  W)
     {
         const auto&  P  {W.database_prefs.prefs};
         grant_privileges  (root_db,  P.database_instance,  P.database_user);
     }

static  void  grant_privileges  (Wizard&  W)
     {
         auto  [prefs,  root_db]  {get_root_access (W.database_prefs.prefs)};
         if  (root_db)    { W.privilege_panel->hide ();
                            return  grant_privileges  (*root_db,  W); }
         W.privilege_panel->show_all ();
     }



static  bool  no_user_access  (Wizard&  W,  const string& error)
{
  static  const regex  error_re  {".*access denied for user.*", regex::icase};

  if  (! regex_match  (error,  error_re))   return  1;

  {Prefs  p  {W.database_prefs.prefs};
        p.database_instance = "";
        try    {   DB {p};   grant_privileges  (W);   return  1;   }
        catch (exception&)   {}
  }
  
  auto  [p, root_db]  {get_root_access  (W.database_prefs.prefs)};
  W.database_prefs  =  p;

  if  (! root_db)
    {
      W.cannot_access_panel_1->show_all ();
      W.database_prefs.password.set_sensitive (1);
      return  0;
    }

  if  (root_db->scalar_result<int>
                     (0,
                      "select count(*) from mysql.user where User='%s'",
                      W.database_prefs.prefs.database_user))
    {
      W.cannot_access_panel_2->show_all ();
      W.database_prefs.password.set_sensitive (1);
      return  0;
    }

  try    {auto  I  {root_db->instruction ()};
              I  <<  "create user '"
                 << W.database_prefs.prefs.database_user
                 << "'";
              if  (W.database_prefs.prefs.database_password.length ())
                   I  << " identified by '"
                      << W.database_prefs.prefs.database_password << "'";
              I.execute  ();
         }
           /*  Putting in user names containing a hypen produces errors,
            *  but the user is correctly constructed nonetheless. */
         catch (exception&)  {}
  
  grant_privileges  (*root_db,  W);

  return  1;
}



     static  void  generate_random_password  (Database_Prefs&  P)
        {
            static  mt19937  E;
            static  uniform_int_distribution  I  {'a',  'z'};
            string  new_password;
            for (int i = 0; i < 20; ++i)    new_password += I (E);
            Preferences  temp_P  {P.prefs};
            temp_P.database_instance  =  "";
            DB {temp_P}
                .instruction ("set password=password('" + new_password + "')");
            P.prefs.database_password  =  new_password;
        }

static  bool  blank_password_problem  (Wizard&  W,  Database_Prefs&  P)
{
   bool  complete  {0};
   if  (! P.prefs.blank_password_forced)
     {
       if (W.force)    {   generate_random_password (P);
                           return 1;     }
       W.no_password_panel->show_all ();
       connect_clicked  (W.gpb_i,  W.generate_password_button,
                         [&W,  &P] 
                         {    generate_random_password  (P);
                              W.get_database_connection  (P.prefs);    });
       connect_clicked  (W.unpb_i,  W.use_no_password_button,
                         [&W, &P] 
                         {    P.prefs.blank_password_forced = 1;
                              W.get_database_connection (P.prefs);  });
     }

   else  /*  Blank password forced. */
     {
       W.blank_password_panel->show_all ();
       W.blank_password_ack_button.set_active ();
       W.bpab_i.disconnect ();
       W.bpab_i = W.blank_password_ack_button
         .signal_toggled ().connect ([&W,  &P] 
                                     {  P.prefs.blank_password_forced = 0;
                                        W.get_database_connection (P.prefs); });
       complete = 1;
     }

   P.password.set_sensitive (1);
   return  complete;
}



void  Wizard::get_database_connection  (Prefs  P)
  {
    no_password_panel->hide ();
    blank_password_panel->hide ();
    cannot_access_panel_1->hide ();
    cannot_access_panel_2->hide ();
    privilege_panel->hide ();
    create_database_panel->hide ();
    create_password_panel->hide ();
    port_problem_panel->hide ();
    socket_problem_panel->hide ();
    host_problem_panel->hide ();

    database_prefs  =  P;
    database_prefs.password.set_sensitive (0);
    database_prefs.submit.set_sensitive (0);
    set_page_complete  (database_access_page,  0);
    bool  complete  {1};

    try   {   DB  db  {P};   }
    catch  (exception&  E)
      {
        cerr <<  "Database response: " << E.what () << "\n";

        if  (host_problem (*this,  E.what ())
                     ||  socket_problem  (*this,  E.what ())
                     ||  port_problem  (*this,  E.what ()))
            return;

        if  (P.database_password.empty ())
          complete  =  no_user_access (*this,  E.what ())   &&   complete;

        else
          {
             string  original_password  {""};
             swap  (P.database_password,  original_password);

             try   {   {  DB  db  {P};  }
                       swap  (P.database_password,  original_password);
                       complete
                          =  set_password_on_database (*this,  database_prefs)
                                        &&   complete;      }
             catch (exception&  E)
               {
                 swap  (P.database_password,  original_password);
                 complete  =  no_user_access (*this, E.what ())   &&  complete;
               }
         }
      }


    if  (database_prefs.prefs.database_password.empty ()  &&  complete)
         complete  =  blank_password_problem  (*this,  database_prefs);
   
    complete  =  database_instance_problem  (*this,  database_prefs)
                      &&  complete;

    set_page_complete  (database_access_page,  complete);
    if  (force  &&  complete)   next_page ();
  }



}  /* End of namespace DMBCS::Trader_Desk. */
