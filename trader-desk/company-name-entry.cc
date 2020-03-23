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


#include <trader-desk/company-name-entry.h>


namespace DMBCS::Trader_Desk {


  Company_Name_Entry::Company_Name_Entry  (Chart_Data &cd)
    : chart_data     (cd),
      previous_icon  {Gtk::Stock::GO_BACK,    Gtk::ICON_SIZE_SMALL_TOOLBAR},
      next_icon      {Gtk::Stock::GO_FORWARD, Gtk::ICON_SIZE_SMALL_TOOLBAR},
      label          {pgettext ("Label", "Company name: ")}
  {
    Gtk::TreeModel::ColumnRecord column_record;
    column_record.add (name);
    column_record.add (seqid);

    tree_model = Gtk::ListStore::create (column_record);

    auto completion = Gtk::EntryCompletion::create ();
    completion->set_model (tree_model);
    completion->set_text_column (name);
    entry.set_completion (completion);

    entry.set_width_chars (30);

    previous_company_button . add (previous_icon);
    next_company_button     . add (next_icon);

    sub_box.pack_start (previous_company_button, Gtk::PACK_SHRINK);
    sub_box.pack_start (next_company_button,     Gtk::PACK_SHRINK);

    pack_start  (sub_box, Gtk::PACK_SHRINK, 20);
    pack_start  (label,   Gtk::PACK_SHRINK,  0);
    pack_start  (entry,   Gtk::PACK_SHRINK,  0);

    previous_company_button.signal_clicked ()
                           .connect ([this] { previous_company_required (); });

    next_company_button.signal_clicked ()
                       .connect ([this] { next_company_required (); });

    chart_data . changed_signal
               . connect ([this]
                          { if (chart_data.company_name != entry.get_text ())
                              entry.set_text (chart_data.company_name); });

    /* When the user types/selects a company name, then presses enter... */
    entry . signal_activate () . connect ([this] { do_name_select (); } );
  }



  void Company_Name_Entry::read_names (DB&  db,
                                       size_t const &market_id,
                                       size_t const &company_id)
  {
    tree_model->clear ();

    auto  sql  {db.row_query ()};

    sql << "   select rtrim(name), seqid "
        << "     from company "
        << "    where market=" << market_id
        << " order by name asc";

    cursor  =  begin (tree_model->children ());

    for (sql.execute (); sql; ++sql)
      {
        auto row = tree_model->append ();
        (*row) [name]  = sql.next_entry<string> ();
        (*row) [seqid] = sql.next_entry<int> ();

        if ((*row) [seqid]  ==  company_id)
          cursor = row;
      }
  }



  void Company_Name_Entry::do_name_select ()
  { 
    cursor  =  find_if (begin (tree_model->children ()),
                        end (tree_model->children ()),
                        [this, text=entry.get_text ()]
                          (Gtk::TreeRow const &x)
                          { return x [name]  ==  text; });

    name_change . emit ((*cursor) [seqid]);
  }



  void Company_Name_Entry::next_company_required ()
  {
    if (cursor != tree_model->children ().end ())
      ++cursor;

    if (cursor == tree_model->children ().end ())
      cursor = begin (tree_model->children ());

    entry.set_text ((*cursor) [name]);

    name_change.emit ((*cursor) [seqid]);
  }



  void Company_Name_Entry::previous_company_required ()
  {
    if (cursor == begin (tree_model->children ()))
      cursor = end (tree_model->children ());

    entry.set_text ((*--cursor) [name]);

    name_change.emit ((*cursor) [seqid]);
  }


}  /* End of namespace DMBCS::Trader_Desk. */
