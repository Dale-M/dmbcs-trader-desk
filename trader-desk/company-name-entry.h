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


#ifndef DMBCS__TRADER_DESK__COMPANY_NAME_ENTRY__H
#define DMBCS__TRADER_DESK__COMPANY_NAME_ENTRY__H


#include <trader-desk/chart-grid.h>
#include <gtkmm.h>


/** \file
 *
 *  Declaration of the \c Company_Name_Entry class. */


namespace DMBCS::Trader_Desk {
  

  /** Drop-down box allowing display and selection of names of companies
   *  in a given market, plus a couple of little arrows to move backwards
   *  and forwards through the list. */

  struct Company_Name_Entry : Gtk::HBox
  {
    /** The object whose company data we are interested in. */
    Chart_Data &chart_data;

    /** The actual on-screen widget. */
    Gtk::Entry entry;

    /** A place to pack the forwards/backwards arrows together, for
     *  aesthetic reasons. */
    Gtk::HBox  sub_box;

    /** The image on the back button. */
    Gtk::Image previous_icon;

    /** The back button itself. */
    Gtk::Button previous_company_button;

    /** The image on the forward button. */
    Gtk::Image next_icon;

    /** The forward button itself. */
    Gtk::Button next_company_button;

    /** Static label in front of the selection box. */
    Gtk::Label label;

    /** Human-readable company names, part of \c tree_model. */
    Gtk::TreeModelColumn <Glib::ustring> name;

    /** The database sequence ID of the companies, in correspondence with
     *  the \c names above, part of \c tree_model. */
    Gtk::TreeModelColumn <unsigned> seqid;

    /** The tree model holds a list of all company names and seqids, and
     *  is used for two purposes: to provide completions to the entry
     *  input box, and to provide us with an STL-like container of names,
     *  e.g. for iterating through. */
    Glib::RefPtr<Gtk::ListStore> tree_model;

    /** Pointer to the currently selected (active) item in the \c
     *  tree_model. */
    Gtk::TreeModel::iterator cursor;

    /** We emit this signal when the name is changed (selected) by the
     *  user. */
    sigc::signal <void, unsigned> name_change;
    

    /** Sole constructor which composes the entire composite widget and
     *  has everything ready for operation. */
    explicit Company_Name_Entry (Chart_Data &);

    /* The various copy and move operations are deleted by default since
     * the base class doesn't support them. */

    /** Refresh the list of company names available to scroll through, and
     *  in the drop-down box. */
    void  read_names  (DB&,  size_t const &market_id, size_t const &company_id);

    /** The user has clicked the ‘next’ button. */
    void next_company_required ();

    /** The user has clicked the ‘previous’ button. */
    void previous_company_required ();

    /** The user has picked a new company name in the entry widget. */
    void do_name_select ();


  };  /* End of class Company_Name_Entry. */

 
}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /*  Undefined DMBCS__TRADER_DESK__COMPANY_NAME_ENTRY__H. */
