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


#ifndef DMBCS__TRADER_DESK__ANALYZER__H
#define DMBCS__TRADER_DESK__ANALYZER__H


#include <trader-desk/chart-context.h>
#include <trader-desk/chart-data.h>
#include <trader-desk/tide-mark.h>
#include <gtkmm.h>


/** \file
 *
 *  Declaration of the \c Analyzer pure interface, and of the \c
 *  Analyzer_Stack class. */


namespace DMBCS::Trader_Desk {


  /** Abstract idea of an object which can draw anything it wants onto a
   *  chart, hopefully to provide some useful illumination of the data to
   *  the user.  Additionally a control widget can be given which will
   *  appear at the right-hand side of the screen. */

  struct Analyzer
  {
    /** Obligatory null destructor for a purely virtual base class. */
    virtual ~Analyzer () = default;

    /** The returned object, if not empty, will be stacked up at the right
     *  edge of the window.  This is optional functionality for analyzers,
     *  so we provide a default implementation which adds no controls. */
    virtual vector <Gtk::Widget *> make_control_widgets ()
    {  return  {};  }
    

    /** Draw whatever into the \a chart_context.  The tide_marks are the
     *  points in time at which tide-lines need adding, if they are to
     *  show on the \c hand_analysis_widget. */
    virtual void graph_draw_hook
                   (Chart_Context &canvas,
                    Tide_Mark::List &notes,
                    unsigned number_shares,
                    vector <Tide_Mark::Price_Marker> const &) = 0;

    /** An opportunity for the analyzer to increase the area in which
     *  time-series are plotted, if this is necessary to display all of
     *  the auxiliary data, for example.  Most analyzers donʼt need this,
     *  so we provide a null default. */
    virtual void stretch_outline (Time_Series::Range &)  {}

    /** Emitted internally to signal to the world that analytical results
     *  have changed (and probably need re-rendering on-screen). */
    virtual sigc::signal<void> &signal_redraw_needed () = 0;

    /** Allow an analyzer to respond to mouse presses on the chart canvas.
     *
     *  As it stands, the system only allows for one of the analyzers to
     *  respond to this, and that privilege is currently taken by the \c
     *  Delta_Analyzer. */
    virtual bool button_down (int const /*x*/, int const /*y*/)   { return 0; }

    /** Allow an analyzer to respond to mouse button releases. */
    virtual bool button_up (int const /*x*/, int const /*y*/)   { return 0; }

    /** Allow an analyzer to respond to mouse drags on the chart canvas. */
    virtual bool button_move (int const /*x*/, int const /*y*/)   { return 0; }


  };  /* End of class Analyzer. */



  /** Whether there is one analyzer in the system or twenty, this class
   *  makes it appear to the rest of the application as if there were just
   *  one, thus simplifying matters.  It is simply a wrapper around a
   *  container of analyzers.  This also removes any dependencies of the
   *  application on the specifics of the analyzers keeping the
   *  abstraction purely abstract: the \c Analyzer_Stack constructor is a
   *  shrink-wrapped analyzer factory. */

  struct Analyzer_Stack : Analyzer
  {
    /** The individual analyzers that we provide a home for. */
    vector <unique_ptr <Analyzer>>  analyzers;
    
    /** If any analyzer indicates the need for a graphics re-draw, we will
     *  fire this signal which the application needs to listen out for. */
    sigc::signal<void> redraw_needed_signal;

    /** If not \c nullptr, this analyzer is working with the mouse. */
    Analyzer *mouse_user {nullptr};


    /** The class constructor is actually a comprehensive analyzer
     *  factory, and returns a fully populated object ready to run the
     *  full gamut of analysis at the userʼs behest. */
    explicit Analyzer_Stack (Chart_Data &,  Preferences&);


    /** Provide a container-load of widgets which the various analyzers
     *  need to function.  These are not at all life-time managed and the
     *  application must take responsibility to destroy the objects when
     *  necessary.  The application is expected to allocate space
     *  horizontally at the right-hand edge of the data charts. */
    vector <Gtk::Widget *> make_control_widgets ()  override
    {
      vector <Gtk::Widget*> ret;
      for (auto &a : analyzers)
        {
          auto const w = a->make_control_widgets ();
          ret.insert (end (ret), begin (w), end (w));
        }
      return ret;
    }


    /** Give every analyzer an opportunity to draw onto the chart
     *  canvas. */
    void graph_draw_hook 
             (Chart_Context &c,
              Tide_Mark::List &t,
              unsigned number_shares,
              vector <Tide_Mark::Price_Marker> const &p) override
    {
      for (auto &a : analyzers)  
          a->graph_draw_hook (c, t, number_shares, p);
    }


    /** Give every analyzer a go at stretching the size of the chart
     *  canvas. */
    void stretch_outline (Time_Series::Range &r) override
    {  for (auto &a : analyzers)  a->stretch_outline (r);  }


    /** Return our re-draw signal. */
    sigc::signal<void> &signal_redraw_needed () override
    { return redraw_needed_signal;  }


    /** Give each analyzer notification of a button press event until one
     *  responds and acts on the event. */
    bool button_down (int const x, int const y) override
    {
      for (auto &a : analyzers)
        if (a->button_down (x, y))
          {
            mouse_user = a.get ();
            return 1;
          }
      
      return 0;
    }


    /** Give the ‘active’ analyzer information about a mouse button-up
     *  event. */
    bool button_up (int const x, int const y) override
    {  return mouse_user  &&  mouse_user->button_up (x, y);  }


    /** Give the ‘active’ analyzer information about a mouse move
     *  event. */
    bool button_move (int const x, int const y) override
    {  return mouse_user  &&  mouse_user->button_move (x, y);  }
    

  };  /* End of class Analyzer_Stack. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__ANALYZER__H. */
