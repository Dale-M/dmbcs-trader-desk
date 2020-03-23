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


#ifndef DMBCS__TRADER_DESK__DB__H
#define DMBCS__TRADER_DESK__DB__H


#include <trader-desk/mysql.h>


/** \file
 *
 *  Declaration of the DB class. */


namespace DMBCS::Trader_Desk {


    /** Wrapper around a database connection which ensures that the tables
     *  we need are in place. */
    /*  Currently hard-wired around MySQL, but the intention is to
     *  abstract the database back-end. */

    struct DB  :  Mysql::DB_Connection
    {
        Preferences&  current_preferences;
        Preferences   last_preferences;


        /** Establish a connection to the RDBMS, check for our database,
         *  create and pre-populate tables if necessary.  May throw an \c
         *  Exception object. */
        DB (Preferences&);


        DB&  check_connection  ();


    } ;  /* End of class DB. */


}  /* End of namespace DMBCS::Trader_Desk. */


#endif  /* Undefined DMBCS__TRADER_DESK__DB__H. */
