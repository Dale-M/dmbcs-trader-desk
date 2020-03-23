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


#ifndef DMBCS__TRADER_DESK__SCALE__H
#define DMBCS__TRADER_DESK__SCALE__H


#include <trader-desk/chart-data.h>
#include <gtkmm.h>


/** \file
 *
 *  Declaration of \c Scale, \c Linear_Scale, and \c Exponential_Scale
 *  classes. */


namespace DMBCS::Trader_Desk {


  /** A \c Scale is a prototypical vertical slider control which gets
   *  stacked to the right of a detail (\c Hand_Analysis_Widget) chart
   *  allowing the user some control over the display of information on
   *  top of that chart.
   *
   *  Note that the rest of the program interacts with these objects via
   *  the \c Adjustments, in particular listening for the \c value_changed
   *  signals on those objects. */

  class Scale : public Gtk::HBox
  {
  protected:
    
    /** The data which the user is currently observing, which should be
     *  regarded as a read-only object which may influence the operation
     *  of the \c Scale slider (although the--extremely special--\c
     *  Date_Range_Scale does cause changes to happen in the \c data). */
    Chart_Data &data;

    /** A graphical component of the \c Scale widget: displays an
     *  identifying text string according to the \c label_format. */
    Gtk::Label label;

    /** A \c printf-style format string which may include a place-holder
     *  for a floating-point value; this is used as the identifying label
     *  on the slider and will display the slider's current value instead
     *  of the placeholder. */
    string const label_format;

    /** The actual GTK widget which implements our \c Scale. */
    Gtk::VScale scale;

    /** Called internally whenever the user moves the slider on the
     *  screen.  This is the place to transform the slider's internal \c
     *  Adjustment into the adjustment we present to the application. */
    virtual void on_slider_changed ()   { setup_label (); }

    /** Called when an externally generated signal fires on the \c data
     *  object to indicate that some change has taken place in those
     *  data. */
    virtual void on_data_changed   () {}

    
  public:

    /** The sole constructor, which provides a fully populated object
     *  ready for operations (though often the range of values covered by
     *  the slider is not known until later). */
    Scale (Chart_Data &d,
           string const &label_format,
           Gdk::RGBA const normal,
           Gdk::RGBA const active,
           Gdk::RGBA const prelight,
           double const min, double const max,
           double const initial_value = -1.0);

    /* The underlying GTK widget machinery severely restricts any hope we
     * may have of copying and moving this type of object. */

    /** Obligatory destructor for a pure virtual interface. */
    virtual ~Scale () = default;
    
    /** Provide an adjustment.  This may be the actual \c Adjustment under
     *  control of the slider, but sometimes it is another object managed
     *  by a derived class which apparently provides a more sophisticated
     *  level of control. */
    virtual Glib::RefPtr<Gtk::Adjustment> value () = 0;

    /* /\** Ditto above. *\/ */
    virtual Glib::RefPtr<const Gtk::Adjustment>  value () const = 0;
    
    /** Apply the `value' of the slider to the \c label_format string and
     *  put the result as the text displayed by the \c label. */
    void setup_label ();


  };  /* End of class Scale. */

  

  /** Most basic concrete \c Scale, basically a wrapper around the base
   *  class which returns the slider's own (directly controlled)
   *  adjustment as the value given to the user.  Even though this is a
   *  concrete type, it should only be used as the base of a more
   *  specialized \c Scale which represents some useful quantity. */

  struct Linear_Scale : Scale
  {
    /** Sole constructor which creates a fully populated object. */
    Linear_Scale (Chart_Data &d,
                  string const &label_,
                  Gdk::RGBA const &normal,
                  Gdk::RGBA const &active,
                  Gdk::RGBA const &prelight,
                  double const min, double const max,
                  double const initial_value)
      : Scale (d, label_, normal, active, prelight, min, max, initial_value)
    {
      setup_label ();
    }

    /** Simply return the slider's directly-controlled adjustment
     *  object. */
    Glib::RefPtr<Gtk::Adjustment> value ()  override   { return scale.get_adjustment (); }

    /* /\** Ditto. *\/ */
    Glib::RefPtr<const Gtk::Adjustment>  value () const   override
                                          { return scale.get_adjustment (); }
  };

  

  /** A \c Scale which is exponential (or logarithmic, depending on which
   *  way you like to look at things), allowing for higher values to be
   *  more tightly packed together.
   *
   *  The scale is parameterized at user level by the values of the
   *  end-points and (linear) mid-point, and in all other respects acts
   *  just like any other \C Scale object would with the exception that
   *  new values are assigned through the \c set_value method; the
   *  application should *not* try to set the value directly in the
   *  adjustment which is given out.
   *
   *  Although this is a concrete class, it is anticipated that the
   *  application will only see further specializations derived from this
   *  one. */

  struct Exponential_Scale : Scale
  {
    /** The value at the linear mid-point of the scale. */
    double mid_value;

    /** The \c Adjustment which we present to the user, exhibiting the
     *  exponentially varying values. */
    Glib::RefPtr<Gtk::Adjustment>  value_adjustment;
    
    /** When the user moves the graphical slider. */
    void on_slider_changed ()  override;
    

    /** The sole constructor, which fully specifies an \c
     *  Exponential_Scale object. */
    Exponential_Scale (Chart_Data &d,
                       string const &label,
                       Gdk::RGBA const normal,
                       Gdk::RGBA const active,
                       Gdk::RGBA const prelight,
                       double const lower, double const mid, double const upper,
                       double const initial_value)
      : Scale (d, label, normal, active, prelight, 0.0, 1.0),
        mid_value (mid),
        value_adjustment {Gtk::Adjustment::create (mid, lower, upper)}
    {
      value_adjustment -> signal_value_changed ()
                       . connect ([this] { setup_label (); });

      set_value (initial_value);
    }


    /** The means by which the application imposes a value on the
     *  slider. */
    void set_value (double const x);


    /** Give the logical, i.e. not the linear, value to the user. */
    Glib::RefPtr<Gtk::Adjustment> value ()  override  { return value_adjustment; }


    /* /\** Ditto, but for \c const objects. *\/ */
    Glib::RefPtr<const Gtk::Adjustment>  value () const  override
                                                  { return value_adjustment; }


  };  /* End of class Exponential_Scale. */

    
}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /*  Undefined DMBCS__TRADER_DESK__SCALE__H. */
