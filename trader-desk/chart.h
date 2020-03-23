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


#ifndef DMBCS__TRADER_DESK__CHART__H
#define DMBCS__TRADER_DESK__CHART__H


#include <functional>
#include <trader-desk/analyzer.h>
#include <trader-desk/date-axis.h>


/** \file
 *
 *  Declaration of the \c Chart class. */


namespace DMBCS::Trader_Desk {


  /** This is a GTK widget which displays a chart complete with all its
   *  trimmings.  Most of the bulk of the implementation deals with the
   *  gooey details of actually rendering all the parts into the part of
   *  the screen occupied by the widget, but there is also some code to
   *  support a stack of analyzers, which will pass along mouse events to
   *  them if appropriate. */


  struct Chart : Gtk::DrawingArea
  {
  private:
    /** Set of selectable ‘trimmings’ which may adorn the chart. */
    struct Feature
    {
      /** The company name near the top-left corner. */
      uint32_t static constexpr const COMPANY_NAME = 1 << 0;

      /** Titles and tick indicators outside the chart itself. */
      uint32_t static constexpr const AXIS_LABELS  = 1 << 1;

      /** Live cursors which follow the mouse around and display numerical
       *  data at all points where the cross-hairs intersect an
       *  interesting point. */
      uint32_t static constexpr const CROSS_HAIRS  = 1 << 2;

      /** Lines and numerical data highlighting interesting parts of a
       *  chart. */
      uint32_t static constexpr const TIDE_MARKS   = 1 << 3;

      /** Display output of full analysis stack. */
      uint32_t static constexpr const ANALYZERS    = 1 << 4;
    };
    
  public:
    /** Combinations of the above features which are enabled for each
     *  ‘personality’ in which charts appear. */
    struct Style
    {
      /** The appearance of charts on the market-wide thumbnail
       *  display. */
      uint32_t static constexpr const THUMB = Feature::COMPANY_NAME;

      /** The appearance of the chart shown in the detailed ‘hand
       *  analysis’ widget. */
      uint32_t static constexpr const HAND_ANALYSIS = Feature::AXIS_LABELS
                                                         + Feature::CROSS_HAIRS
                                                         + Feature::TIDE_MARKS
                                                         + Feature::ANALYZERS;
    };


    /** The actual data we are presenting; our actions are driven to a
     *  large extent from the data_changed signal emitted by this
     *  object. */
    Chart_Data data;

    /** An object which will embellish our chart with extra analytical
     *  tools and features. */
    unique_ptr<Analyzer_Stack> analyzer;

  private:
    /** The features we want to embellish our display with: bit-field of \c
     *  Feature's via \c Style. */
    uint32_t const features;

    /** The last known coordinates of the mouse cursor, when it was over
     *  our chart; used to convey mouse place into expose() method. */
    int pointer_x, pointer_y;
  public:

    /** Sole constructor which partially initializes an object (note in
     *  particular that no company is specified here). */
    Chart (uint32_t const features_,  Preferences&);

    /** A \c Chart is a very heavy object and we do not want to be copying
     *  or even moving these around. */
    Chart (Chart const &)          = delete;
    Chart (Chart&&)                = delete;
    void operator= (Chart const &) = delete;
    void operator= (Chart &&)      = delete;

  private:
    /** Run all of the analyzers, and then render them, the chart, and all
     *  of its selected trimmings. */
    bool on_draw                (const Cairo::RefPtr<Cairo::Context>&) override;
    
    /** If cross-hairs are live, send the button event to the analyzer
     *  stack. */
    bool on_button_press_event   (GdkEventButton *const) override;

    /** If cross-hairs are live, send the button event to the analyzer
     *  stack. */
    bool on_button_release_event (GdkEventButton *const) override;

    /** If cross-hairs are live, update all of them and get the chart
     *  re-drawn.  Finally pass the motion event to the analyzers. */
    bool on_motion_notify_event  (GdkEventMotion *const motion) override;

    /** If cross-hairs are live, remove them by re-drawing the chart
     *  without. */
    bool on_leave_notify_event   (GdkEventCrossing *const) override;


  };  /* End of class Chart. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif /* Undefined DMBCS__TRADER_DESK__CHART__H. */
