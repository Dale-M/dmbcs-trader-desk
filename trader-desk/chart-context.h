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


#ifndef DMBCS__TRADER_DESK__CHART_CONTEXT__H
#define DMBCS__TRADER_DESK__CHART_CONTEXT__H


#include <trader-desk/text.h>
#include <trader-desk/time-series.h>
#include <pangomm.h>
#include <cairomm/cairomm.h>


/** \file
 *
 *  Declaration of the \c Chart_Context class. */


namespace DMBCS::Trader_Desk {


  /** A wrapper around a Cairo context: a canvas on which to draw things.
   *  Methods are provided for drawing lines and filling areas represented
   *  relative to real times and prices, and for laying down and rendering
   *  small items of text.
   *
   *  The class is really just an implementation detail of \c Chart
   *  (though references are passed out and used in several other
   *  classes), and is instantiated only transiently in that class; there
   *  is not a proper constructor but provision for the \c Chart class to
   *  get an uninitialized object and for that class to then perform full
   *  initialization. */

  struct Chart_Context
  {
    /** The `canvas' on which the chart is painted. */
    Cairo::RefPtr <Cairo::Context> cairo;

    /** An object which helps with painting text onto the above canvas. */
    Glib::RefPtr <Pango::Layout> pango;

    /** The number of pixels between the left edge of the window and the
     *  side of the chart. */
    double left_border;

    /** The number of pixels between the right-hand side of the chart and
     *  the window edge. */
    double right_border;

    /** The number of pixels above the chart. */
    double top_border;

    /** The number of pixels below the chart. */
    double bottom_border;

    /** The width of the chart, in pixels. */
    double width;

    /** The height of the chart, in pixels. */
    double height;

    /** The (date, price) extremes of the chart. */
    Time_Series::Range outline;

    /** The collection of text strings which will be placed on the chart
     *  when it is rendered. */
    Text text;


    /** The application has a single (transient) instance of this class,
     *  created and initialized (constructed) within Chart::on_expose.  We
     *  provide that method with the means to make an uninitialized
     *  object, and then prohibit any copying or moving of that object. */
    Chart_Context () = default;
    
    Chart_Context (Chart_Context const &) = delete;
    Chart_Context &operator= (Chart_Context const &) = delete;
    Chart_Context (Chart_Context &&) = delete;
    Chart_Context &operator= (Chart_Context &&) = delete;



    /** Plot the chart. */
    void draw_time_series (Time_Series const &series,
                           Colour const &colour,
                           double const &alpha) const;
        

    /** Move the `pen' to the position on the chart corresponding to the
     *  \a event. */
    void move_to  (Event const &event) const
    {  cairo->move_to  (x (event.time),  y (event.price));  }

    /** Draw a line on the chart to the position corresponding to \a
     *  event. */
    void line_to  (Event const &event) const
    {  cairo->line_to  (x (event.time),  y (event.price));  }


    /** Set the colour for the subsequent drawing operations. */
    void set_source_rgb (Colour const &c, double const &alpha) const
    { cairo->set_source_rgba (c.red, c.green, c.blue, alpha); }

    /** Ditto. */
    void set_source_rgb (Colour const &c) const
    { cairo->set_source_rgb (c.red, c.green, c.blue); }


    /*  While this class is designed to present the application with an
     *  interface expressed entirely in terms of stock market \c Event
     *  points, when we are placing text onto the charts it is necessary
     *  to expose the raw coordinates, for reasons involving the
     *  displacement of the text position to allow for size and anchor
     *  orientation. */

    Time_Point     date  (int const &x) const;
    Currency_Value value (int const &y) const;

    double x (Time_Point const &t) const;
    double y (Currency_Value const &value) const;

    Text::Place x (Event const &e) const  
    {  return {x (e.time), y (e.price)};  }
    
    

    /** Add the \a message into the \a text object. */
    void add (Text &text,
              string const &message,
              Colour const &,
              Text::Place const &);

    /** Render all the messages which have previously been added to the \a
     *  text object (these will be re-arranged so that there are no
     *  overlaps). */
    void render (Text &);


  };  /* End of class Chart_Context. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__CHART_CONTEXT__H. */
