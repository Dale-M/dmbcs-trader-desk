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


#include <trader-desk/application.h>
#include <dmbcs-market-data-api.h>


/** \file
 *
 *  Implementation of the \c Application::ingest_new_market
 *  mega-method. */


namespace DMBCS::Trader_Desk {


  void Application::ingest_new_market ()
  {
    try
      {
        DB  db  {user_prefs};
        Markets markets {db};

        try   {
                  update_market_meta_data (markets,  db);
              }
        catch (Market_Data_Api::Bad_Communication const &e)
          {
            Gtk::MessageDialog {*window, e.what (), 0, Gtk::MESSAGE_WARNING}
                      .run ();
            return;
          }

        Gtk::Dialog dialog (pgettext ("Instruction", "Select market"),
                            *window,
                            Gtk::DIALOG_MODAL);

        auto add_label = [&dialog] (string const &m)
          {
            Gtk::Label *const t = new Gtk::Label;
            t->set_markup (m);
            dialog . get_vbox ()  ->  pack_start (*Gtk::manage (t),
                                                  Gtk::PACK_SHRINK);
          };

        add_label (pgettext ("Instruction", "Select market"));

        if (find_if (begin (markets), end (markets),
                     [] (Markets::value_type const &m)
                     { return ! m.second.tracked; })
               == markets.end ())
          {
            add_label (string {"<span color=\"red\">"}
                              + pgettext ("Join-A", "Sorry, there are no")
                              + "</span>");

            add_label (string {"<span color=\"red\">"}
                              + pgettext ("Join-A", "new markets.") 
                              + "</span>");

            dialog  . add_button (pgettext ("Instruction", "Cancel"),
                                  Gtk::RESPONSE_CANCEL);
            dialog  . set_size_request (300, 250);
            dialog  . show_all ();

            dialog  . run ();
          }

        else
          {
            Gtk::TreeModelColumn<string> name;
            Gtk::TreeModelColumn<Market_Meta_Data const *> data;
            Gtk::TreeModel::ColumnRecord columns;
            columns.add (name);
            columns.add (data);

            auto list = Gtk::ListStore::create (columns);
            for (auto const &u : markets)
              if (! u.second.tracked)
                {
                  auto b = list->append ();
                  (*b) [name] = u.second.world_data.name;
                  (*b) [data] = &u.second;
                }

            Gtk::TreeView view {list};
            view  . append_column (pgettext ("Label", "Market"), name);

            Gtk::TreeModel::Path selected_row;


            view . signal_row_activated ()
                 . connect ([&selected_row, &dialog] 
                                 (Gtk::TreeModel::Path const &path,
                                  Gtk::TreeViewColumn *const)
                                 { selected_row = path;
                                   dialog.response (Gtk::RESPONSE_OK); });

            dialog  . get_vbox ()  -> pack_start (view);

            add_label (string {"<span color=\"red\" size=\"small\">"}
                                + pgettext ("Instruction, Join-B", 
                                            "Double-click a market name") 
                                + "</span>");

            add_label (string {"<span color=\"red\" size=\"small\">"}
                               + pgettext ("Instruction, Join-B",
                                           "to ingest its data") 
                               + "</span>");

            dialog  . add_button (pgettext ("Instruction", "Cancel"), 
                                  Gtk::RESPONSE_CANCEL);
            dialog  . set_size_request (300, 250);
            dialog  . show_all ();

            if (Gtk::RESPONSE_OK  ==  dialog . run ())
              {
                dialog.hide ();

                auto market_data  =  *(*list->get_iter (selected_row)) [data];

                start_tracking  (markets,  db,  market_data.world_data.symbol);

                /* Check that the seqid is not in market_grids. */

                auto a  =  find_if  (begin (market_grids), 
                                     end (market_grids),
                                     [seqid = market_data.seqid]
                                       (unique_ptr<Chart_Grid> const &g)
                                       { return g->market.seqid == seqid; });

                if (a == end (market_grids))
                  {
		    /* !!!!  How is this getting destroyed?  */
                    auto *const  g  {new Chart_Grid  {user_prefs, market_data}};

                    if (market_grids.empty ())  notebook.remove_page (1);
                    
                    market_grids.emplace_back (g);

                    g->selection_signal
                      .connect ([this, g]
                                   { hand_analysis->subsume_selected (*g);
                                     close_data_menu->set_sensitive (0);
                                     last_data_menu->set_sensitive (1);
                                     notebook.set_current_page (0); });

                    notebook.append_page (*g);
                    display_grid (market_grids.size () - 1);
                    notebook.show_all ();

                    /* Now fix up the menus. */
                    {
                      auto const n = market_grids.size () - 1;
                      ostringstream a;  a << "display-grid-" << n;

                      actions->add (Gtk::Action::create 
                                       (a.str (),  market_data.world_data.name),
                                    [this, n] { display_grid (n); });

                      ui_manager->add_ui_from_string
                        ("<ui>"
                         "   <menubar name=\"menu\">"
                         "      <menu action=\"display-menu\">"
                         "         <menuitem action=\"" + a.str () + "\"/>"
                         "      </menu>"
                         "   </menubar>"
                         "</ui>");
                    }

                    /* !!!!  Maybe think of re-setting the connection to
                     *       the database here. */

                    update_closing_prices ();
                  }
              }
          }
      }

    catch (Market_Data_Api::No_Network &e)
      {
        Gtk::MessageDialog  {*window,  e.what (),  0,  Gtk::MESSAGE_WARNING}
                  .run ();
      }
  }


}  /* End of namespace DMBCS::Trader_Desk. */
