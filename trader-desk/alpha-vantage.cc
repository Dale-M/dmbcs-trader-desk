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


#include <trader-desk/alpha-vantage.h>
#include <trader-desk/time-series.h>  /* For time utilities. */
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>
#include <fmt/format.h>
#include <regex>


namespace DMBCS::Trader_Desk {


static  bool  throttle  ()
    {
        namespace  C  =  chrono;
        using  clock  =  C::system_clock;

        static  clock::time_point  LAST_CALL;

        const  C::duration  wait  {clock::now ()  -  LAST_CALL};
        if  (wait < C::seconds {12})
          {
            usleep  (min (number<C::microseconds> (C::seconds {12} - wait),
                          number<C::microseconds> (C::milliseconds {500})));
            return 1;
          }

        LAST_CALL  =  clock::now ();
        return  0;
    }
      


static  void  throw_error  (const string&  query,  const string&  json)
    {
         static const  regex  re  {"\"(Note|Error Message)\": +\"([^\"]*)"};
         smatch  match;
         if (! regex_search  (json, match, re))
               throw  Alpha_Vantage::Error
                          {"AlphaVantage returned garbled error message"};
         if (match [1] == "Note")   throw  Alpha_Vantage::Throttled {match [2]};
         static const  regex  re2  {"apikey.*(invalid|missing)", regex::icase};
         if  (regex_search  (match [2].str (), re2))
                 throw  Alpha_Vantage::Bad_API_Key  {match [2]};
         throw  Alpha_Vantage::Error
                        {match [2].str ()
                            +   "\n\n[The query was ‘"   +   query   +   "’.]"};
    }



static  string  get_curl_response  (const string&  query)
  {
    namespace Curl  =  cURLpp;
    namespace Opt   =  Curl::Options;

    string      response;
    Curl::Easy  request;
            
    request.setOpt (Opt::Url {query});

    request.setOpt (Opt::WriteFunction 
                       {[&response] (char* buffer,  size_t size,  size_t n) 
                                    { response  +=  string {buffer, 
                                                            buffer + size * n};
                                      return  size * n; }});

    request.setOpt (Opt::SslVerifyPeer {0});

    request.perform ();

    return  response;
  }



    static  time_t  to_time_t  (const Update_Closing_Prices::Data&  D)
                     {   return  t (D.year, D.month, D.day);   }

    
    inline  string  maybe_dot  (const string&  X)
                     {   return  X.length ()  ?  '.' + X  :  X;    }
    


    static  string  get_timeseries_csv
                             (const Update_Closing_Prices::Company&  company,
                              string  market_component_extension,
                              const string&  api_key,
                              const bool  over_100)
      {
        const string  query
          {fmt::format ("https://www.alphavantage.co/query"
                                       "?function=TIME_SERIES_DAILY_ADJUSTED"
                                       "&symbol={}"
                                       "&apikey={}"
                                       "&datatype=csv"
                                       "&outputsize={}",
                        company.symbol + maybe_dot (market_component_extension),
                        api_key,
                        over_100 ? "full" : "compact")  };

        const string  ret  {get_curl_response (query)};

        if  (ret [0] == '{')   throw_error (query, ret);

        return ret;
      }

    static  vector<string>  fields  (const string&  csv,  const char  separator)
    {
      vector<string>  ret;
      for  (size_t cursor {0}; ; )
        {
          const size_t  next_cursor  {csv.find (separator, cursor)};
          ret.push_back (csv.substr (cursor, next_cursor - cursor));
          if  (next_cursor == csv.npos)   return  ret;
          cursor = next_cursor + 1;
        }
    }

     static  istream&  operator>> (istream& I,  Update_Closing_Prices::Data&  D)
          {
                 static  char  comma;
                 static  double  dividend_amount,  split_coefficient;

                 I >> D.year >> comma >> D.month >> comma >> D.day >> comma
                   >> D.open >> comma >> D.high >> comma
                   >> D.low >> comma >> D.close >> comma
                   >> D.adj_close >> comma >> D.volume >> comma
                   >> dividend_amount >> comma >> split_coefficient;

                 return  I;
          }

    static  vector<Update_Closing_Prices::Data>  parse_csv
                     (const string&  results,  const int  company_seqid)
          {
               istringstream  O  {results.substr  (results.find  ('\n'))};
               vector<Update_Closing_Prices::Data>  data;

               for (;;)    {    Update_Closing_Prices::Data  datum;
                                datum.company_seqid  =  company_seqid;
                                O >> datum;
                                if  (! O.good ())  return data;
                                data.push_back (move (datum));    }
          }

    static  int  days_ago  (const time_t  T)
          {
             using  Clock  =  chrono::system_clock;
             return  number<chrono::hours> (Clock::now ()
                                                 -  Clock::from_time_t (T))
                               / 24;
          }

auto  Alpha_Vantage::get_closing_prices
          (const Update_Closing_Prices::Company&  company,
           const string&  market_component_extension,
           const Preferences&  P,
           const function <void (const Update_Closing_Prices::Data&)>  injector)
  ->  TO_DO
   {
      if  (throttle  ())   return MORE_WORK;

      vector<Update_Closing_Prices::Data>  data
              {parse_csv  (get_timeseries_csv
                                    (company,
                                     market_component_extension,
                                     P.market_data_service_key,
                                     days_ago (company.last_close_date) > 100),
                           company.seqid)};

      auto  i  {  find_if  (data.rbegin (),  data.rend (),
                            [L = company.last_close_date]
                                        (const Update_Closing_Prices::Data&  D)
                                  {    return  to_time_t (D) > L;    })};

      if  (i != data.rbegin())  --i;

      for  (auto j {i};  j != data.rend ();  ++j)
                   injector  (*j);

      return  FINISHED;
   }



  static  string  get_snap_csv  (const Update_Latest_Prices::Company&  company,
                                 const string&  market_component_extension,
                                 const string&  api_key)
  {
    const auto  query  
    { fmt::format  ("https://www.alphavantage.co/query"
                                     "?function=GLOBAL_QUOTE"
                                     "&symbol={}"
                                     "&datatype=csv"
                                     "&apikey={}",
                    company.symbol + maybe_dot (market_component_extension),
                    api_key)  };

    const string  ret  {get_curl_response (query)};
    if  (ret [0]  ==  '{')   throw_error  (query, ret);
    return  ret;
  }

      inline  double  extract_price_csv  (const string&  X)
      {
        return  strtod  (fields  (fields  (X, '\n') [1],  ',') [4].data (),
                         nullptr);
      }
    


Update_Latest_Prices::Data  Alpha_Vantage::get_latest_data
                               (const Update_Latest_Prices::Company&  company,
                                const string&  market_component_extension,
                                const Preferences&  P)
   {
      while  (throttle ())   ;

      return
        {  .company_seqid  =  company.seqid,
           .time  =  chrono::system_clock::now (),
           .price  =  extract_price_csv
                             (get_snap_csv  (company,
                                             market_component_extension,
                                             P.market_data_service_key))   };
   }


}  /* End of namespace DMBCS::Trader_Desk. */
