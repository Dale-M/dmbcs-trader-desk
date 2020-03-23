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


#include  <trader-desk/markets.h>
#include  <trader-desk/time-series.h>   /* Only for number<> (). */


namespace DMBCS::Trader_Desk {


Markets::Markets  (DB&  db)
  {
    auto  sql  {db.row_query ()};

    sql << "select seqid, symbol, name, component_extension, tracked, "
        << "       unix_timestamp(last_update), "
        << "       60*hour(close_time)+minute(close_time) "
        << "  from market";

    Market_Meta_Data  m;

    try
      {
        for  (sql.execute ();  sql;  ++sql)
          {
            sql  >>  m.seqid            >>  m.world_data.symbol
                 >>  m.world_data.name  >>  m.world_data.component_extension
                 >>  m.tracked;

            m.last_time  =  sql.next_entry ((time_t) 0);

            m.world_data.close_time  =  chrono::minutes {sql.next_entry (0)};

            this->insert  ({m.seqid,  m});
          }
      }
    catch  (Mysql::DB_Connection::Exception&)  {}
  }



    static  Markets::iterator  find_symbol  (Markets&  markets,
                                             const string&  symbol)
    {
      return  find_if  (markets.begin (),  markets.end (),
                        [&symbol]  (const Markets::value_type&  d)
                            { return  d.second.world_data.symbol == symbol; });
    }

void  update_market_meta_data  (Markets&  markets,  DB&  db)
try
  {
    if (Market_Data_Api::short_time 
             (db.scalar_result 
                  ((time_t) 0,
                   "select unix_timestamp(last_markets_update) from global")))
      return;

    /* !!! Need to be prepared for this not to work (offline?) */
    for  (const Market_Data_Api::Market&  i  :  Market_Data_Api::get_markets ())
      if   (find_symbol  (markets,  i.symbol)   ==   markets.end ())
        {
          auto  s  {db.instruction ()};

          s << "insert into market "
            << "   set symbol='" << i.symbol << "', "
            << "       name=\"" << i.name << "\", "
            << "       component_extension='" 
                                          << i.component_extension << "', "
            << "       close_time=sec_to_time(" 
                             << number<chrono::seconds> (i.close_time)
                             << ") ";

          s.execute ();

          markets.insert ({(size_t) s.insert_id (),
                           {.world_data  =  i,
                            .seqid       =  (size_t) s.insert_id (), 
                            .tracked     =  0,
                            .last_time   =  0}});
        }

    db.instruction ("update global "
                                "set last_markets_update=from_unixtime(%d)",
                    time (nullptr));
  }
catch  (Mysql::DB_Connection::Exception&)  {}



bool update_components
                (Markets&  markets,  DB&  db,  Gtk::Window *const  window)
  {
    /* !!!  This wants to be the time on the individual market, *not* the
     *      global time-stamp. */
    if (Market_Data_Api::short_time 
            (db.scalar_result 
                ((time_t) 0,
                 "select unix_timestamp(last_markets_update) from global")))
        return  false;
    
    update_market_meta_data (markets,  db);

    bool  ret  {false};

    for  (auto&  m  :  markets)
        if (m.second.tracked)
            ret  =  ret  ||  Trader_Desk::update_components
                                                 (m.second,  db,  window);

    db.instruction ("update global "
                       "set last_markets_update=from_unixtime(%d)",
                    time (nullptr));

    return ret;
  }



void  update_database
        (const Market_Data_Api::Delta&  d,  DB&  db,  const int  market_seqid)
  {
      const int  company_seqid 
              {db.scalar_result  (0,
                                  "select seqid "
                                    "from company "
                                   "where symbol='%s' and market=%d",
                                  d.symbol.c_str (),
                                  market_seqid)};

      if (company_seqid)
              db.quick ()  <<  "update company "
                           <<     "set market="  <<  market_seqid
                           <<  " where seqid="   <<  company_seqid;
      else
              db.quick ()  <<  "insert into company "
                           <<          "set name=\""  <<  d.name   << "\", "
                           <<              "symbol='" <<  d.symbol << "', "
                           <<              "market="  <<  market_seqid;
  }



     static  void  replace  (string&  in,  const char  a,  const string&  text)
           {
                for  (size_t cursor {0};
                      (cursor = in.find (a, cursor))  !=  in.npos;
                      ++cursor)
                   in.replace  (cursor,  1,  text);
           }

    static  string  htmlize  (string in)
           {
                 replace  (in,  '&',  "&amp;");
                 replace  (in,  '<',  "&lt;" );
                 replace  (in,  '>',  "&gt;" );

                 return in;
           }

bool  update_components  (Market_Meta_Data&  market_data,
                          DB&  db,
                          Gtk::Window *const  window)
  {
    if (Market_Data_Api::short_time (market_data.last_time))
        return false;

    auto  instructions  {Market_Data_Api::get_component_delta
                                              (market_data.world_data.symbol,
                                               market_data.last_time)};

    market_data.last_time = time (0);

    db.instruction ("update market "
                    "   set last_update=from_unixtime(%d) "
                    " where seqid=%d",
                    market_data.last_time,  market_data.seqid);

    if (instructions.empty ())   return false;


    /* List added companies for the benefit of the user. */
    string  additions;

    /* List removed companies for the benefit of the user. */
    string  removals;

    /* The total number of additions and removals. */
    size_t  line_count  {0};

    /* Whether or not a terminating ‘...’ has been added to the additions. */
    bool  additions_terminated  {false};

    /* Whether or not a terminating ‘...’ has been added to the removals. */
    bool  removals_terminated  {false};

    auto make_updater
      =  [&line_count] 
         (string&  line,  bool&  terminated)
          {
            return [&line, &terminated, &line_count]
                   (string const &name, string const &symbol)
                         {
                           if (++line_count < 10)
                             line   +=   "    " + htmlize (name) 
                                           + " (" + symbol + ")\n";

                           else if (! terminated)
                             {
                               terminated = 1;
                               line   +=   "... (more)...\n";
                             }
                         };
          };

    auto update_additions  =  make_updater (additions, additions_terminated);
    auto update_removals   =  make_updater (removals,  removals_terminated);


    const Market_Data_Api::Delta*  move_pending  {nullptr};


    for  (auto const &i  :  instructions)
      {
        if (move_pending)
          {
            if (i.action != i.SIDEWAYS)   break;

            /* !!! This is a potentially dangerous operation if the same
             *     symbol is used in different markets.  We will have to
             *     be careful how we run the server in this regard. */
            auto const company_seqid 
                          = db.scalar_result  (0,
                                               "select seqid "
                                                 "from company "
                                                "where symbol='%s'",
                                               move_pending->symbol.c_str ());

            if (! company_seqid)
              db.instruction ("insert into company "
                                 "set name=\"%s\", "
                                     "symbol='%s', "
                                     "market=%d",
                              i.name.c_str (), i.symbol.c_str (),
                              market_data.seqid);

            else
              db.instruction ("update company "
                                 "set name=\"%s\", "
                                     "symbol='%s', "
                                     "market=%d "
                               "where seqid=%d",
                              i.name.c_str (), i.symbol.c_str (),
                              market_data.seqid, company_seqid);
            
            update_additions (i.name, i.symbol);

            move_pending  =  nullptr;
          }
        

        else if  (i.action  ==  i.SIDEWAYS)
          {
            move_pending  =  &i;
          }


        else if  (i.action  ==  i.ADD)
          {
            update_database  (i,  db,  market_data.seqid);
            update_additions  (i.name,  i.symbol);
          }

        else  /* i.action == REMOVE */
          {
            db.quick ()  <<  "update company "
                         <<     "set market=0 "
                         <<   "where symbol='" << i.symbol << "' "
                         <<     "and market="  <<  market_data.seqid;

            update_removals (i.name, i.symbol);
          }
      }

    if  (! window)   return  true;
    
    string a = "<b>MARKET MOVEMENTS</b>\n\n<u>"
                           +  market_data.world_data.name  +  "</u>\n";

    if (additions.length ())
         a  +=  "<span color=\"green\">New entries</span>\n" + additions;

    if (removals.length ())
         a  +=  "<span color=\"red\">Dropped entries</span>\n" + removals;

    Gtk::MessageDialog {*window, a, 1/*use mark-up*/}  .  run ();

    return  true;
  }



void  start_tracking  (Markets&  markets,  DB&  db,  const string&  symbol)
  {
    const Markets::iterator  m  {find_symbol  (markets,  symbol)};
    
    if  (m  ==  markets.end ())   return;
    
    m->second.tracked  =  1;

    db.instruction  ("update market  set tracked=1  where seqid=%d",  m->first);
  }
    

}  /* End of namespace DMBCS::Trader_Desk. */
