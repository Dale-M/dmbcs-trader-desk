#ifndef DMBCS__TRADER_DESK__ALPHA_VANTAGE__MONITOR__H
#define DMBCS__TRADER_DESK__ALPHA_VANTAGE__MONITOR__H


#include  <trader-desk/alpha-vantage.h>


namespace DMBCS::Trader_Desk {


    struct Alpha_Vantage__Monitor
    {
      using  Error       =  Alpha_Vantage::Error;
      using  Throttled   =  Alpha_Vantage::Throttled;
      using  Bad_API_Key =  Alpha_Vantage::Bad_API_Key;
      using  TO_DO       =  Alpha_Vantage::TO_DO;

      struct  Clocks
      {
        DB  db;
        chrono::steady_clock::time_point  last_time;
        int strikes {0};

        Gtk::Label  starter     {"AlphaVantage charge: "};
        Gtk::Label  count_down  {"0"};
        Gtk::Label  spacer      {"  "};
        Gtk::Label  strikes_    {"0"};
        Gtk::Label  finisher    {"/500"};
        

        explicit  Clocks  (Preferences&  P)  :  db {P}
        {  update ();  }

        void  hit ();
        const Clocks&  update  ();
      };



      static  unique_ptr<Clocks>  clocks;



      static  void  initialize_clocks  (Preferences&  P)
      {
        if  (! Alpha_Vantage__Monitor::clocks)
               Alpha_Vantage__Monitor::clocks  =  make_unique<Clocks> (P);
      }
      

      static  void  make_clock_widgets  (Preferences&,  Gtk::HBox&  container);

      static  void  remove_clock_widgets  (Gtk::Container&);


      static  TO_DO  get_closing_prices   /* override */
         (const Update_Closing_Prices::Company&  C,
          const string&  market_component_extension,
          Preferences&  P,
          const function <void (const Update_Closing_Prices::Data&)>  injector)
      {
        initialize_clocks  (P);
        const TO_DO  ret  {Alpha_Vantage::get_closing_prices
                                (C, market_component_extension, P, injector)};
        if (ret == TO_DO::FINISHED)  clocks->hit ();
        return  ret;
      }
      


      static  Update_Latest_Prices::Data  get_latest_data  /* override */
                              (const Update_Latest_Prices::Company&  C,
                               const string&  market_component_extension,
                               Preferences&  P)
      {
        initialize_clocks  (P);
        auto  ret  {Alpha_Vantage::get_latest_data
                                        (C, market_component_extension, P)};
        clocks->hit ();
        return  ret;
      }
      

    } ;  /* End of class Alpha_Vantage__Monitor. */


  }  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__ALPHA_VANTAGE__MONITOR__H. */
