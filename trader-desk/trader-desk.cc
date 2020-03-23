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


#include  "auto-config.h"
#include  "alpha-vantage--monitor.h"
#include  "application.h"
#include  "markets.h"
#include  "update-latest-prices.h"
#include  "wizard.h"


/** \file
 *
 *  Implementation of the \c Window object, and the applicationʼs \c main
 *  entry point. */


namespace DMBCS::Trader_Desk {


static void  grid_injector  (Chart_Grid &grid,
                             const Update_Latest_Prices::Data&  data)
  {
    Chart *const  c  {grid.find_chart (data.company_seqid)};

    if (c)    c->data.new_event ({chrono::system_clock::to_time_t (data.time),
                                  data.price});
  }



struct  Window  :   Gtk::Window
  {
    /** All of the pieces that make up this GTK Window application,
     *  including widgets that go in the window, engine parts, and
     *  glue. */
    Application app;
    
    /** Top-level layout: menu stacked on top of a notebook. */
    Gtk::VBox v_box;
    Gtk::HBox menu_strip;
    Gtk::HBox alphavantage_clocks;


    explicit  Window  (Preferences&&);

    void update_latest_data ();

    void  change_company_name  (Chart_Data&  chart_data,  const int&  seqid);

    void  create_application  (Preferences&&  P);

    void  destroy_application  ();


  };  /* End of Window class. */


static  void  show_about  ()
  {
      Gtk::AboutDialog a;
      a.set_program_name (PACKAGE);
      a.set_version (VERSION);
      a.set_copyright
             (gettext ("Copyright (c) Dale Mellor 2017, 2020\nGPLv3+ applies"));
      a.set_documenters (vector<Glib::ustring> {"Dale Mellor"});
      a.set_license_type (Gtk::LICENSE_GPL_3_0);
      a.set_website ("https://rdmp.org/trader-desk");
      a.set_authors (vector<Glib::ustring> {"Dale Mellor"});
      a.set_logo (Gdk::Pixbuf::create_from_file (PKGDATADIR "/trader-desk.png")
                           ->scale_simple (100, 100, Gdk::INTERP_NEAREST));
      a.run ();
  }


static  void  run_wizard
                 (Preferences&  P,
                  const bool  force,
                  std::function<void (Preferences&)>  post_process  =  {})
  {
    Wizard *const  W   {new Wizard {P.file_path, force}};
    /* W->set_transient_for (*this); */
    /* W->set_attached_to  (*this); */
    W->set_modal (1);
    W->show ();
    W->signal_unmap ().connect ([W, &P, post_process]
                                        { P = W->database_prefs.prefs;
                                          dump (P);
                                          if (post_process)   post_process (P);
                                          delete W;  });
  }



static  void  run_preferences_dialog
                    (Window&  W,  const string&  alpha_vantage_message  =  {})
  {
    auto  D  {alpha_vantage_message.length ()
                 ?    new Preferences_Dialog  {W, W.app.user_prefs,
                                               alpha_vantage_message}
                 :    new Preferences_Dialog  {W, W.app.user_prefs}};
    D->signal_hide ()
      .connect ([&W, D, P=W.app.user_prefs]
                {  delete D;
                   run_wizard (W.app.user_prefs, 1,
                               [&W, P] (Preferences&  newP)
                               {
                                 if (! database_equal (P, newP))
                                   {
                                     W.destroy_application ();
                                     W.create_application (Preferences {newP});
                                   }
                               });  });
    D->show ();
  }



static  void  run_unforced_wizard  (Window&  W)
  {
    const auto  P  {W.app.user_prefs};
    
    run_wizard (W.app.user_prefs, 0,
                [&W, P] (Preferences&  newP)
                        {
                          if (! database_equal (P, newP))
                            {
                              W.destroy_application ();
                              W.create_application (Preferences {newP});
                            }
                        });
  }



void  Window::change_company_name  (Chart_Data&  chart_data,  const int&  seqid)
    {
        unique_ptr<DB>  db;

        for (auto &a : app.market_grids)
            {
                Chart *const  c  {a->find_chart (seqid)};

                if (c)   {   chart_data.subsume  (&c->data);
                             if (! db)  db  =  make_unique<DB> (app.user_prefs);
                             app . hand_analysis -> company_name
                                 . read_names  (*db,  a->market.seqid,  seqid);
                             return;     }
            }
    }
  


void  Window::create_application  (Preferences&&  P)
  {
      app.window = this;
      app.user_prefs  =  std::move (P);

      app.preferences_error
        =  [this] (const string&  alpha_vantage_message)
              {   run_preferences_dialog  (*this,  alpha_vantage_message);   };
      
      app.hand_analysis
          = make_unique<Hand_Analysis_Widget>
                ([this] (Chart_Data &cd, int const &seqid) 
                                {   change_company_name (cd, seqid);   },
                 app.user_prefs);

      app.actions = Gtk::ActionGroup::create ();

      app.actions->add (Gtk::Action::create ("quit", Gtk::Stock::QUIT),
                        Gtk::AccelKey {"<control>q"},
                        [this] { close (); });

      app.actions->add (Gtk::Action::create ("file-menu", 
                                             pgettext ("Menu", "_File")));
      app.actions->add
                   (Gtk::Action::create ("preferences", 
                                         pgettext ("Menu", "_Preferences")),
                    Gtk::AccelKey {"<control>p"},
                    [this] { run_preferences_dialog  (*this); });
      app.actions->add
                   (Gtk::Action::create ("wizard", 
                                         pgettext ("Menu", "_Wizard")),
                    Gtk::AccelKey {"<control>w"},
                    [this] { run_unforced_wizard  (*this); });
      app.actions->add (Gtk::Action::create ("display-menu", 
                                             pgettext ("Menu", "_Market")));
      app.actions->add (Gtk::Action::create ("market-menu",
                                             pgettext ("Menu", "_Update")));
      app.actions->add (Gtk::Action::create ("help-menu",
                                             pgettext ("Menu", "_Help")));
      app.actions->add (Gtk::Action::create ("about", 
                                             pgettext ("Menu", "_About")),
                        &show_about);


      DB  db  {app.user_prefs};
      for (auto &m  :  Markets {db})
        if (m.second.tracked)
          {
            const auto  i  {app.market_grids.size ()};

            if (i == 0)  set_title  (pgettext ("Label", "Trader Desk")
                                         + string {" : "}
                                         + m.second.world_data.name);

            app . market_grids
                . emplace_back  (make_unique<Chart_Grid>
                                     (app.user_prefs,  m.second));

            /* This needs to go into a sub-routine as it is required
             * elsewhere. */

            ostringstream a;   a << "display-grid-" << i;

            if (i < 9)
              {
                ostringstream b;   b << "<control>" << i+1;

                app . actions
                   -> add (Gtk::Action::create (a.str (),
                                                m.second.world_data.name),
                           Gtk::AccelKey (b.str ()),
                           [this, i, name = m.second.world_data.name]
                                     { app.display_grid (i);
                                       set_title (pgettext ("Label",
                                                            "Trader Desk")
                                                     + string {" : "}
                                                     + name);  });
              }

            else
              app.actions->add (Gtk::Action::create (a.str (),
                                                     m.second.world_data.name),
                                [this, i, name = m.second.world_data.name]
                                { app.display_grid (i);
                                  set_title (pgettext ("Label",
                                                       "Trader Desk")
                                                 + string {" : "}
                                                 + name); });
          }


      app.last_data_menu
        =  Gtk::Action::create ("update-recent",
                                pgettext  ("Menu",  "Update _latest data"));
      app.last_data_menu->set_sensitive (0);
      app.actions->add (app.last_data_menu,
                        [this] { update_latest_data (); });

      app.close_data_menu
        =  Gtk::Action::create ("update-closes",
                                pgettext  ("Menu",  "Update _close data"));
      app.actions->add (app.close_data_menu,
                        [this] { app.update_closing_prices (); });

      app.actions->add (Gtk::Action::create ("ingest-new-market",
                                             pgettext ("Menu",
                                                       "_Ingest new market")),
                        [this] { app.ingest_new_market (); });

      app.ui_manager = Gtk::UIManager::create ();
  
      app.ui_manager->insert_action_group (app.actions);

      add_accel_group (app.ui_manager->get_accel_group ());
  
      app.ui_manager->add_ui_from_string
                        ("<ui>"
                         "  <menubar name=\"menu\">"
                         "    <menu action=\"file-menu\">"
                         "      <menuitem action=\"ingest-new-market\"/>"
                         "      <menuitem action=\"preferences\"/>"
                         "      <menuitem action=\"wizard\"/>"
                         "      <menuitem action=\"quit\"/>"
                         "    </menu>"
                         "    <menu action=\"display-menu\">"

                         + [this] 
                           { ostringstream a;
                             for (size_t i=0; i < app.market_grids.size (); ++i)
                               a << "<menuitem action=\"display-grid-" 
                                                               << i << "\"/>\n";
                             return a.str (); }  ()
                         +
                         "    </menu>"
                         "    <menu action=\"market-menu\">"
                         "      <menuitem action=\"update-recent\"/>"
                         "      <menuitem action=\"update-closes\"/>"
                         "    </menu>"
                         "    <menu action=\"help-menu\">"
                         "      <menuitem action=\"about\"/>"
                         "    </menu>"
                         "  </menubar>"
                         "</ui>");

      v_box.pack_start (menu_strip,  Gtk::PACK_SHRINK);
      menu_strip.pack_start (*app.ui_manager->get_widget ("/menu"),
                             Gtk::PACK_SHRINK);
      menu_strip.pack_end  (alphavantage_clocks,  Gtk::PACK_SHRINK);
      Alpha_Vantage__Monitor::make_clock_widgets  (app.user_prefs,
                                                   alphavantage_clocks);
      Glib::signal_timeout()
           .connect ([]  {  Alpha_Vantage__Monitor::clocks->update ();
                            return  1;  },
                     200);
      
      

      /*   v_box.pack_start (*app.ui_manager->get_widget ("/ToolBar"), */
      /*                          Gtk::PACK_SHRINK); */

      app.notebook.set_show_tabs (0);
      app.notebook.set_show_border (0);
      app.notebook.append_page (*app.hand_analysis);

      if (app.market_grids.empty ())
        app.notebook.append_page (*Gtk::manage (new Gtk::Label {"No markets"}));
      else
        for (auto &g : app.market_grids)
          app.notebook.append_page (*g);
      
      v_box.pack_start (app.notebook);
      add (v_box);
      
      show_all ();

      app.display_grid (0);

      for (auto &a : app.market_grids)
        a->selection_signal
          .connect ([this, &a] { app.hand_analysis->subsume_selected (*a);
                                 app.close_data_menu->set_sensitive (0);
                                 app.last_data_menu->set_sensitive (1);
                                 app.notebook.set_current_page (0); });

      if (app.market_grids.empty ())
        app.ingest_new_market ();

      set_default_size (650, 420);
      set_icon_from_file (PKGDATADIR "/trader-desk.png");

  }  /* End of create_application. */



void  Window::destroy_application  ()
    {
      for (auto *const W  :  app.notebook.get_children ())
             app.notebook.remove (*W);
      app.market_grids.clear ();
      app.hand_analysis.reset ();
      for (auto *const W  :  menu_strip.get_children ())
             menu_strip.remove (*W);
      v_box.remove (menu_strip);
      Alpha_Vantage__Monitor::remove_clock_widgets  (alphavantage_clocks);
      v_box.remove (app.notebook);
      remove (/* v_box */);
    }



void Window::update_latest_data ()
  try
    {
      DB  db  {app.user_prefs};
      
      if (app.notebook.get_current_page ()  ==  0)
        {
          Update_Latest_Prices::do_update  (db,
                                            app.hand_analysis->chart.data,
                                            app.user_prefs);
          return;
        }
      
      Chart_Grid *const  market
               {app.market_grids [app.notebook.get_current_page ()-1].get ()};

      Update_Latest_Prices::Work  update  {market->market};
      
      do_update (&update,
                 db,
                 app.user_prefs,
                 [&db,  market]
                    (const Update_Latest_Prices::Data&  data)  ->  void
                     {   sql_injector   (db,      data);
                         grid_injector  (*market, data);  });
    }
    catch (const Alpha_Vantage::Error&  E)
      {
        Gtk::MessageDialog 
                 {*this,
                  gettext ("There seems to be a problem with the AlphaVantage "
                           "account, please check your settings on the "
                           "preferences panel.  The message from the "
                           "server is:") + string {"\n\n‘"} + E.what () + "’",
                  0,
                  Gtk::MESSAGE_ERROR}
           .run ();
        run_preferences_dialog  (*this);
      }
    catch (Update_Closing_Prices::No_Connection const &)
       {
             Gtk::MessageDialog  {*this,
                                  gettext ("No Internet Connection"),
                                  0,
                                  Gtk::MESSAGE_WARNING}
                .run ();
       }
    


Window::Window  (Preferences&&  P)  :  app {std::move (P)}
    {
      signal_map_event ()
        .connect ([this] (GdkEventAny*)   ->  bool
                  {  run_wizard (app.user_prefs, 1,
                                 [this] (Preferences&  P)
                                 { create_application (Preferences {P}); });
                    return  0;  } );
    }


}  /* End of namespace DMBCS::Trader_Desk. */



int main (int argc, char **argv)
try
  {
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);

    /*  Needed for Alpha_Vantage. */
    curlpp::Cleanup curl_lifetime;

    std::string  config_file;

    if (argc > 1)
      {
        using std::cout;

        if (argv [1] == std::string {"--config"}
                ||    argv [1] == std::string {"-c"})
          {
            if  (argc < 3)
              {
                std::cerr << PACKAGE_STRING
                          << "Error: -c option requires an argument.\n";
                exit (1);
              }
            config_file  =  argv [2];
          }

        else if (argv [1] == std::string ("--version"))
          {
            cout << PACKAGE_STRING << '\n';
            cout << gettext ("Copyright (C) 2017, 2020  Dale Mellor") << "\n\n"
                 << gettext ("License GPLv3+: GNU GPL version 3 or later "
                                        "<http://gnu.org/licenses/gpl.html>\n"
                             "This is free software: you are free to change "
                                                      "and redistribute it.\n"
                             "There is NO WARRANTY, to the extent permitted "
                                                                  "by law.\n");
            exit (0);
          }
        else if (argv [1] == std::string ("--help"))
          {
            cout << gettext ("usage") << ": trader-desk [options]\n";
            cout << gettext ("To report bugs or contact the authors please "
                                     "refer to http://rdmp.org/trader-desk\n");
            exit (0);
          }
      }
    
    Glib::thread_init ();

    Gtk::Main kit (argc, argv);

    namespace TD  =  DMBCS::Trader_Desk;
    TD::Window  window   {config_file.empty ()
                               ?  TD::Preferences::from_default_file ()
                               :  TD::Preferences::from_file  (config_file)};

    Gtk::Main::run (window);

    return 0;
  }

catch (std::runtime_error const &e)
  {
    std::cerr << e.what () << ".\n";
    std::exit (1);
  }
