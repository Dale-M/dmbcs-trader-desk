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


#include <trader-desk/delta-analyzer.h>
#include <trader-desk/sd-envelope-analyzer.h>


/** \file
 *
 *  Implementation of the \c Analyzer_Stack constructor, a factory which
 *  knows about all the detailed analyzers.  If a new analyzer is to be
 *  added to the system, this file will need modifying to accomodate it;
 *  it is the *only* place that needs to know anything about specific
 *  analyzers. */


namespace DMBCS::Trader_Desk {


  Analyzer_Stack::Analyzer_Stack  (Chart_Data&  chart_data,  Preferences&)
  {
    analyzers.emplace_back  (new SD_Envelope_Analyzer  {chart_data});
    analyzers.emplace_back  (new Delta_Analyzer        {chart_data});

    for  (auto &a : analyzers)
         a  ->  signal_redraw_needed  ()
             .  connect  ([this] { redraw_needed_signal.emit (); });
  }


}  /* End of namespace DMBCS::Trader_Desk. */
