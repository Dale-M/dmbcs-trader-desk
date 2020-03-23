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


#include <trader-desk/chart.h>
#include <iomanip>
#include <set>


/** \file
 *
 *  Implementation of the \c Chart class. */


namespace DMBCS::Trader_Desk {


  Chart::Chart (uint32_t const features_,  Preferences&  P)
    :  features (features_)
  {
    data . changed_signal . connect ([this] { queue_draw (); });
      
    /* Big enough for at least a thumb; we will take up more space if we're
     * offered it. */
    set_size_request  (40, 25);

    if (features & Feature::CROSS_HAIRS)
      add_events (Gdk::POINTER_MOTION_MASK | Gdk::LEAVE_NOTIFY_MASK
                      | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);

    if (features & Feature::ANALYZERS)
      analyzer =  make_unique<Analyzer_Stack> (data,  P);
  }
    


  bool Chart::on_motion_notify_event (GdkEventMotion *const motion)
  {
    if (features & Feature::CROSS_HAIRS)
      {
        if (analyzer)
          analyzer->button_move (motion->x, motion->y);
    
        pointer_x = (int) motion->x;
        pointer_y = (int) motion->y;

        queue_draw ();
      }

    return 1;
  }



  bool Chart::on_leave_notify_event (GdkEventCrossing *const)
  {
    pointer_x  =  pointer_y  =  -1;
    queue_draw ();
    return 1;
  }



  bool Chart::on_button_press_event (GdkEventButton *const event)
  {
    return  features & Feature::CROSS_HAIRS
                &&  analyzer
                &&  analyzer->button_down (event->x, event->y);
  }

    

  bool Chart::on_button_release_event (GdkEventButton *const event)
  {
    return  features & Feature::CROSS_HAIRS
                &&  analyzer
                &&  analyzer->button_up (event->x, event->y);
  }
    


  /* !! We really want to paint most of the chart into memory, and map it
   *    to the screen as required.  But we need to re-assess all the
   *    tide-marks on the cursor line whenever the cursor moves. */

  bool Chart::on_draw (const Cairo::RefPtr<Cairo::Context>&  cairo)
  {
    /* This object is constructed as we draw the various aspects of the
     * chart, and then it is rendered on top of everything else right at
     * the end of this method. */
    Tide_Mark::List tide_marks;



    /***************  Set up the canvas.  **************************/

    Chart_Context  canvas;

        {
             const Gtk::Allocation  allocation  {get_allocation ()};
             canvas.width   =  allocation.get_width ();
             canvas.height  =  allocation.get_height ();
        }

    canvas.left_border   = features & (int) Feature::AXIS_LABELS ? 45 : 4;
    canvas.bottom_border = features & (int) Feature::AXIS_LABELS ? 40 : 4;
    canvas.top_border    = 4;
    canvas.right_border  = 4;

    canvas.cairo  =   cairo;

    canvas.pango  =   Pango::Layout::create (canvas.cairo);
    canvas.pango  ->  set_font_description (Pango::FontDescription ("Sans 7"));

    canvas.set_source_rgb (data.unaccurate ? Colour::NO_DATA_REGION 
                                           : Colour::CHART_BACKGROUND);
    canvas.cairo  ->  paint ();

    canvas.outline  =  data.extremes;

    double const space  =  0.05  *  (canvas.outline.max_value
                                             - canvas.outline.min_value);

    canvas.outline.min_value -= space;
    canvas.outline.max_value += space;

    if (analyzer)
      analyzer->stretch_outline (canvas.outline);

    auto cursor_mark
      = Tide_Mark::price_marker
           (canvas.date (pointer_x),
            canvas.outline.contains ({ canvas.date (pointer_x),
                                       canvas.value (pointer_y)})
              ?  Colour::CURSOR_TIDES
              :  Colour::NO_DISPLAY);

    auto current_mark = Tide_Mark::price_marker (canvas.outline.end_time,
                                                 Colour::CHART_BACKGROUND);

    canvas.cairo->set_line_width (1.0);



    /************************ No-data region. *******************************/

    {
      auto const r = data.prices.empty () ? canvas.outline.end_time 
                                          : data.prices.back ().time;

      if (r > canvas.outline.start_time)
        {
          canvas.set_source_rgb (Colour::NO_DATA_REGION);

          canvas.move_to ({canvas.outline.start_time,
                           canvas.outline.min_value});

          canvas.line_to ({canvas.outline.start_time,
                           canvas.outline.max_value});

          canvas.line_to ({r, canvas.outline.max_value});

          canvas.line_to ({r, canvas.outline.min_value});

          canvas.cairo->fill ();
        }
    }


    /***********  Let the analyzers draw themselves.  *************/

    if (analyzer)
      analyzer->graph_draw_hook (canvas,
                                 tide_marks,
                                 data.number_shares,
                                 {cursor_mark, current_mark});


    /******** X-axis *******/

    if (features  &  Feature::AXIS_LABELS)
      {
        canvas.set_source_rgb (Colour::TIME_AXIS);

        char buffer [200];

        auto const *disc = Date_Axis::discretization;

        time_t const t = chrono::system_clock::to_time_t 
                                               (data.extremes.end_time);

        for (; disc->format; ++disc)
          {
            /* If the spacing between ticks is less than one pixel,
             * move on. */
            if (disc->real_interval.count ()
                    / double ((data.extremes.end_time
                                       - data.extremes.start_time).count ())
                      * (canvas.width - canvas.left_border
                                             - canvas.right_border)
                  <  1.0)
              continue;

            /* Find the box needed to enclose a label showing the
             * current time. */
            strftime (buffer, sizeof (buffer),
                      disc->format,
                      localtime (&t));

            canvas.pango->set_text (buffer);

            Pango::Rectangle const rect
                  = canvas.pango->get_pixel_logical_extents ();

            /* If the spacing between ticks is more than the space
             * required to display the current date-time, then this is
             * the tick spacing we will use. */
            if (disc->real_interval.count ()
                / double ((data.extremes.end_time
                                   - data.extremes.start_time).count ())
                  * (canvas.width - canvas.left_border
                                         - canvas.right_border)
                > rect.get_width () + 4)
              break;
          }



        auto show_ticks = [&canvas, this]
                             (Date_Axis::Discretization const &disc,
                              uint32_t const &line_offset)
          {
            char buffer [200];

            for (auto i = data.extremes.start_time;
                 i <= data.extremes.end_time;
                 i += disc.interval)
              {
                i = disc.round_down (i);

                if (i > data.extremes.start_time)
                  {
                    auto i_ = chrono::system_clock::to_time_t (i);

                    strftime (buffer, sizeof (buffer),
                              disc.format,
                              localtime (&i_));

                    canvas.pango->set_text (buffer);

                    canvas.move_to ({i, canvas.outline.min_value});

                    canvas.cairo->rel_move_to (0, line_offset + 1);

                    canvas.pango->add_to_cairo_context (canvas.cairo);

                    canvas.cairo->fill ();
                  }
              }
          };



        if (disc->format)
          {
            show_ticks (*disc, 0);

            if ((disc + 1)->format)
              {
                show_ticks (*(disc + 1), 11);

                if ((disc + 2)->format)
                  show_ticks (*(disc + 2), 22);
              }
          }
      }


    /******** Y-axis ********/

    canvas.pango
      ->set_markup (string {"<span size=\"large\">"}
                      + pgettext ("Label", "Position value (pounds)")
                      + "</span>");

    const Pango::Rectangle extents
                  = canvas.pango->get_pixel_logical_extents ();

    const int font_height = extents.get_height ();

    if (features & (int) Feature::AXIS_LABELS)
      {
        canvas.cairo->save ();
        canvas.cairo->rotate (- M_PI / 2.0);
        canvas.cairo->move_to (- (canvas.height - extents.get_width ()) 
                                    / 2,
                                1);
        canvas.pango->add_to_cairo_context (canvas.cairo);
        canvas.set_source_rgb (Colour::PRICE_AXIS);
        canvas.cairo->fill ();
        canvas.cairo->restore ();
      }

    if (features & (int) Feature::AXIS_LABELS)
      {
        double const share_scale = data.number_shares / 100.0;

        double inc = 0.01;

        for (inc = 0.01;
             (inc / share_scale) / (canvas.outline.max_value
                                    - canvas.outline.min_value)
               * (canvas.height - canvas.bottom_border 
                                           - canvas.top_border)
               < font_height * 1.0;
             inc *= 10.0) ;

        for (double b = inc * (floor (share_scale
                                             * canvas.outline.min_value 
                                             / inc)
                                 + 1);
             b < share_scale * canvas.outline.max_value;
             b += inc)
          if (b / share_scale > canvas.outline.min_value)
            {
              ostringstream out;
              out << setw (5) << b;
              canvas.pango->set_text (out.str ());
              Pango::Rectangle const extents
                            = canvas.pango->get_pixel_logical_extents ();
              canvas.move_to ({data.extremes.start_time,
                                b / share_scale});
              canvas.cairo->rel_move_to (- extents.get_width () - 2,
                                          - extents.get_height () / 2);
              canvas.pango->add_to_cairo_context (canvas.cairo);
              canvas.set_source_rgb (Colour::PRICE_AXIS);
              canvas.cairo->fill ();
            }
      }


    if (data.prices.empty ())
      {
        if (features & Feature::COMPANY_NAME)
           canvas.add (canvas.text, 
                       data.company_name,
                       Colour::COMPANY_NAME_TITLE,
                       { canvas.left_border, canvas.top_border });

        canvas.render (canvas.text);

        return 1;
      }


    canvas.set_source_rgb (Colour::PRICE_AXIS);
    canvas.move_to ({data.extremes.start_time, canvas.outline.max_value});
    canvas.line_to ({data.extremes.start_time, canvas.outline.min_value});
    canvas.cairo->stroke ();    

    canvas.set_source_rgb (Colour::TIME_AXIS);
    canvas.move_to ({data.extremes.start_time, canvas.outline.min_value});
    canvas.line_to ({data.extremes.end_time, canvas.outline.min_value});
    canvas.cairo->stroke ();

    {
      lock_guard<mutex> l {data.prices_mutex};

      canvas.draw_time_series (data.prices, Colour::PRICE_GRAPH, 1.0);

      tide_marks.emplace_back (current_mark (data.prices.front ().price,
                                             Colour::PRICE_TIDES));

      tide_marks.emplace_back (cursor_mark (data.prices.interpolated_value
                                             (canvas.date (pointer_x)),
                                            Colour::PRICE_TIDES));
    }

    if (features & Feature::CROSS_HAIRS)
      tide_marks.emplace_back (cursor_mark (canvas.value (pointer_y),
                                            Colour::CURSOR_TIDES));


    /****  Company name  ****/

    if (features & Feature::COMPANY_NAME)
      canvas.add (canvas.text, 
                  data.company_name,
                  Colour::COMPANY_NAME_TITLE,
                  canvas.x ({data.extremes.start_time,
                             canvas.outline.max_value}));


    /*****  Tide marks.  *****/

    if (features  &  Feature::TIDE_MARKS)
      {
        set <Time_Point> dates_shown;

        for (auto const &tide : tide_marks)
          if (tide.temporal_colour !=  Colour::NO_DISPLAY
                               &&  tide.time >= canvas.outline.start_time
                               &&  tide.time <= canvas.outline.end_time)
            {
              canvas.cairo
                    ->set_source_rgb 
                         (1.0 - 0.5 * (1.0 - tide.value_colour.red),
                          1.0 - 0.5 * (1.0 - tide.value_colour.green),
                          1.0 - 0.5 * (1.0 - tide.value_colour.blue));

              canvas.move_to ({canvas.outline.start_time, tide.price});
              canvas.line_to ({canvas.outline.end_time, tide.price});

              canvas.cairo->stroke ();

              ostringstream hold;
              hold << tide.price;

              canvas.add (canvas.text,
                          hold.str (),
                          tide.value_colour,
                          {canvas.x (tide.time) + 20,
                           canvas.y (tide.price) - 13});

              ostringstream hold_2;
              hold_2 << data.number_shares * tide.price / 100.0;

              canvas.add (canvas.text,
                          hold_2.str (),
                          tide.value_colour,
                          {canvas.x (tide.time) - 60,
                           canvas.y (tide.price) - 13});

              if (dates_shown.insert (tide.time).second)
                {
                  canvas.set_source_rgb (tide.temporal_colour);

                  canvas.move_to ({tide.time, canvas.outline.max_value});
                  canvas.line_to ({tide.time, canvas.outline.min_value});

                  canvas.cairo->stroke ();

                  struct tm tm;
                  time_t t = chrono::system_clock::to_time_t (tide.time);
                  localtime_r (&t, &tm);

                  char buffer [200];
                  strftime (buffer, sizeof (buffer), "%Y-%m-%d %H:%M", &tm);

                  canvas.add (canvas.text,
                              buffer,
                              Colour::CURSOR_TIDES,
                              {canvas.x (tide.time) + 2,
                               canvas.height - canvas.bottom_border
                                             - 12});
                }
            }
      }

    canvas.render (canvas.text);

    return 1;

  }  /* End of on_draw method. */

    
}  /* End of namespace DMBCS::Trader_Desk. */
