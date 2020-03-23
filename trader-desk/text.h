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


#ifndef DMBCS__TRADER_DESK__TEXT__H
#define DMBCS__TRADER_DESK__TEXT__H


#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include <trader-desk/colour.h>


/** \file
 *
 *  Declaration of the \c Text class. */


namespace DMBCS::Trader_Desk {


    /** The primary purpose of this class is to arrange short items of
     *  text in a rectangle of the screen in a way that avoids overlaps
     *  and keeps the items as close to their intended positions as
     *  possible.
     *
     *  Thus the public interface is nice and simple: a method to add text
     *  items, \c operator+=, and a method to cause those items to be
     *  arranged optimally, \c arrange.
     *
     *  The strategy is to move all items so that they fall inside the
     *  screen rectangle, and then for increasingly small deltas determine
     *  which movement, in any of the four cardinal directions, of which
     *  item leads to the best improvement in score: the total amount of
     *  area overlap of all the items plus a small number times the sum of
     *  the itemsʼ displacement from their starting values--we want to get
     *  rid of the overlaps at the expense of the displacements.  The
     *  exercise continues until the overlaps are eliminated and delta is
     *  not more than the size of a pixel. */

    struct Text
    {
      /** A structure to carry the details of a rectangular region on the
       *  screen. */
      struct Boundary   {  double left, right, top, bottom;  };


      /** A structure to carry coordinates of a point on the screen, with
       *  some basic arithmetic where it serves our needs in the
       *  implementation of the \c Text class. */
      struct Place
      {
        double x {0.0};
        double y {0.0};

        Place () = default;

        Place (double const &x_, double const &y_) : x (x_), y (y_) {}

        Place &operator+= (Place const &a)
        {   x += a.x;   y += a.y;   return *this;   }

        Place &operator-= (Place const &a)
        {   x -= a.x;   y -= a.y;   return *this;   }
      };


      
      /** An item of text to be placed on the screen. */

      struct Item
      {
        /** The actual text string. */
        string text;

        /** The colour in which the text should be rendered (not actually
         *  needed in this class, but we carry the information along for
         *  the convenience of the application). */
        Colour colour;

        /** The place where we would like the text to appear. */
        Place  native_position;

        /** The size of the box which encloses the text. */
        Place  size;

        /** The computed place on the screen where the text should be
         *  printed so that it does not overlap with any other of the
         *  items. */
        Place  placement;

        
        /** Sole class constructor creates a fully populated object,
         *  except for the \c placement member which is set by subsequent
         *  optimization of a set of objects relative to each other. */
        Item (string const &t, Colour const &c, Place const &p,
              Place const &s) 
          : text {t}, colour {c}, native_position {p}, size {s},
            placement {p}
        {}
        
      };  /* End of class Item. */


    private:

      /* The actual collection of text items we accumulate and
       * arrange. */
      vector<Item> items;


      /* Compute a score, to be minimized, for the current arrangement of
       * text \c Item \C placements. */
      double compute_tension (double const &critical_value,
                              Boundary const &boundary) const;


      /* Randomly move some items of text around. */
      void shuffle (double shift_size,
                    double const &shift_limit,
                    Boundary const &boundary);


    public:

      /** Return a vector of items whose \c placement's indicate the
       *  optimal positions to render each text item. */
      vector<Item> const &arrange (Boundary const &b)
      {
        for (auto &i : items)  i.placement = i.native_position;
        shuffle (10.0, 0.5, b);
        return items;
      }


      /** Add a new text item to the list of items that need to be
       *  optimally arranged. */
      Text &operator+= (Item &&d)  {  items.emplace_back (move (d)); 
                                      return *this;  }
      

    } ;  /* End of class Text. */


  }  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__TEXT__H. */
