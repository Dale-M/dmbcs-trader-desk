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


#include <trader-desk/chart-context.h>



namespace DMBCS::Trader_Desk {



  double Chart_Context::x (Time_Point const &t) const
  {
    return left_border 
           + (width - left_border - right_border)
             * (t - outline.start_time).count ()
             / (double) ((outline.end_time - outline.start_time).count ());
  }


  Time_Point Chart_Context::date (int const &x) const
  {
    return outline.start_time
             + chrono::duration_cast<chrono::seconds>
                         ((x - left_border) 
                          * (outline.end_time - outline.start_time)
                          / (double) (width - left_border - right_border));
  }


  double Chart_Context::y (Currency_Value const &value) const
  {
    return top_border + (height - bottom_border - top_border)
                        * (1.0 - (value - outline.min_value)
                                     / (outline.max_value - outline.min_value));
  }



  Currency_Value Chart_Context::value (int const &y) const
  {
    return outline.max_value
              - (outline.max_value - outline.min_value) 
                * (y - top_border)
                / (double) (height - top_border - bottom_border);
  }



  void Chart_Context::draw_time_series (Time_Series const &series,
                                        Colour const &colour,
                                        double const &alpha) const
  {
    if (series.size () < 2)
      return;

    set_source_rgb (colour, alpha);

    auto i  =  begin (series);

    move_to (*i++);

    while (i != end (series)  &&  i->time >= outline.start_time)
      line_to (*i++);

    cairo->stroke ();
  }



  void Chart_Context::add (Text &text,
                           string const &message,
                           Colour const &colour,
                           Text::Place const &position)
  {
    pango->set_text (message);

    Pango::Rectangle const r {pango->get_pixel_logical_extents ()};

    text += Text::Item {message,
                        colour,
                        position,
                        {(double) r.get_width (), (double) r.get_height ()}};
  }



  void Chart_Context::render (Text &text)
  {
    for (auto const &t : text.arrange ({left_border, width - right_border,
                                        top_border,  height - bottom_border}))
      {
        cairo->rectangle (t.placement.x, t.placement.y,
                          t.size.x, t.size.y);

        set_source_rgb (Colour::MARK_LABEL_BACK, 0.5);

        cairo->fill ();

        pango->set_text (t.text);

        cairo->move_to (t.placement.x, t.placement.y);

        pango->add_to_cairo_context (cairo);

        set_source_rgb (t.colour,  0.7);

        cairo->fill ();
      }
  }


}  /* End of namespace DMBCS::Trader_Desk. */
