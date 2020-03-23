#include <trader-desk/alpha-vantage--monitor.h>


namespace DMBCS::Trader_Desk {

  

  unique_ptr<Alpha_Vantage__Monitor::Clocks>  Alpha_Vantage__Monitor::clocks;



  void  Alpha_Vantage__Monitor::make_clock_widgets  (Preferences&  P,
                                                     Gtk::HBox&  C)
  {
    initialize_clocks (P);
    Clocks&  c  {*clocks};

    C.pack_start (c.starter);
    C.pack_start (c.count_down);
    C.pack_start (c.spacer);
    C.pack_start (c.strikes_);
    C.pack_start (c.finisher);
  }



  void  Alpha_Vantage__Monitor::remove_clock_widgets  (Gtk::Container&  C)
  {
    Clocks&  c  {*clocks};
    C.remove  (c.starter);
    C.remove  (c.count_down);
    C.remove  (c.spacer);
    C.remove  (c.strikes_);
    C.remove  (c.finisher);
    clocks.reset ();
  }



        using  system_clock  =  chrono::system_clock;

void  Alpha_Vantage__Monitor::Clocks::hit  ()
  {
    try
      {
        db.quick()  <<  "insert ignore into alphavantage_ticks (time) value ("
                    <<  (system_clock::to_time_t  (system_clock::now ()))
                    <<  ")";
      }
    catch  (Mysql::DB_Connection::Exception&)
      {
        db.reconnect (db.current_preferences);
      }
    
    last_time  =  chrono::steady_clock::now ();
  }



      static  int  seconds_since  (const chrono::steady_clock::time_point&  T)
                 {    return  chrono::duration_cast<chrono::seconds>
                                      (chrono::steady_clock::now () - T)
                                 .count ();   }

      using  C  =  Alpha_Vantage__Monitor::Clocks;

const  C&  C::update  ()
{
  const time_t  now  {system_clock::to_time_t  (system_clock::now ())};
 
  try
    {
      auto  I  {db.instruction ()};
      I  <<  "delete from alphavantage_ticks where time<"
         <<  (now - 24 * 60 * 60);
      I.execute ();
      strikes
        =  db.scalar_result<int> (0, "select count(*) from alphavantage_ticks");
      strikes_.set_text  (to_string (strikes));
      count_down.set_text  (to_string (max (0,
                                            12 - seconds_since (last_time))));
    }
  catch  (Mysql::DB_Connection::Exception&)  {}
  
  return  *this;
}
  

}  /* End of namespace DMBCS::Trader_Desk. */
