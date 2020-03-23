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


#include <trader-desk/scale.h>


/** \file
 *
 *  Implementation of the \c Scale and \c Exponential_Scale classes. */


namespace DMBCS::Trader_Desk {


  Scale::Scale (Chart_Data &d,
                string const &label_,
                Gdk::RGBA const normal,
                Gdk::RGBA const active,
                Gdk::RGBA const prelight,
                double const min,
                double const max,
                double const initial_value)
         : data (d),
           label_format (label_),
           scale (min, max, (max-min)/100)
  {
    label.set_angle (90.0);

    pack_start (label, Gtk::PACK_SHRINK, 0);
    pack_start (scale, Gtk::PACK_SHRINK, 0);

    scale.get_adjustment ()  ->  set_value (initial_value);

    scale.set_draw_value (0);

    scale.override_background_color  (normal,  Gtk::STATE_FLAG_NORMAL);
    scale.override_background_color  (active,  Gtk::STATE_FLAG_ACTIVE);
    scale.override_background_color  (prelight,  Gtk::STATE_FLAG_PRELIGHT);

    scale . signal_value_changed ()
          . connect ([this] { on_slider_changed (); });

    data . changed_signal
         . connect ([this] { on_data_changed (); });
  }



  void Scale::setup_label ()
  {
    char buffer [label_format.length () + 20];
    sprintf (buffer, label_format.c_str (), value ()->get_value ());
    label.set_text (buffer);
  }



  struct Exponential_Mapping
  {
    double a, b, c; 
   
    explicit Exponential_Mapping (Exponential_Scale const &e)
    {
      c = e.value_adjustment->get_lower ();
      
      double const l = e.mid_value - c;
      double const L = e.value_adjustment->get_upper () - c;
      
      a = sq (L / l - 1);
      b = l * l / (L - 2 * l);
    }
    
    double to_linear (double const &x) const
    { return log ((x - c) / b + 1) / log (a); }
      
    double from_linear (double const &x) const
    { return c + b * (pow (a, x) - 1); }
    
    static constexpr double sq (double const &x) {return x*x;}
  };



  void Exponential_Scale::on_slider_changed ()
  {
    value_adjustment
      ->set_value (Exponential_Mapping {*this}
                        .from_linear (scale.get_adjustment ()->get_value ()));

    value_adjustment->value_changed ();
  }



  void Exponential_Scale::set_value (double const x)
  {
    scale.get_adjustment () 
       -> set_value (Exponential_Mapping {*this} . to_linear (x));

    value_adjustment->set_value (x);
  }


}  /* End of namespace DMBCS::Trader_Desk. */
