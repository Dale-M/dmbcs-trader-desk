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


#include <trader-desk/mysql.h>
#include <iostream>
#include <cstdio>


/** \file
 *
 *  Implementation of class methods in \c Mysql namespace. */


namespace DMBCS::Trader_Desk { namespace Mysql {


  int Instruction::execute ()
  {
    const string  query  {buffer->str ()};
    const int  test  {mysql_real_query (mysql, query.c_str(), query.length())};

    if (test  &&  ! no_error)
	  {
		cerr << "MYSQL ERROR: " << mysql_error (mysql) << endl;
		throw  DB_Connection::Exception  {mysql_error (mysql)};
	  }
    
    return test;
  }


    
  void DB_Connection::connect (const Preferences&  P)
  {
    if (! mysql_real_connect (&mysql,
                              P.database_host.data (),
                              P.database_user.data (),
                              P.database_password.data (),
                              P.database_instance.data (),
                              P.database_port,
                              P.database_socket.data (),
                              0/*flags*/))
      throw Exception {mysql_error (&mysql)};

    initialized  =  1;
  }



  void DB_Connection::reconnect (const Preferences&  P)
  {
    if  (initialized)  mysql_close (&mysql);
    initialized  =  0;
    connect (P);
  }



  DB_Connection::DB_Connection  (const Preferences&  P)
  {
    mysql_init (&mysql);
    connect (P);
  }



  static int _run_query (MYSQL *const mysql,
                         string const &_template,
                         va_list arguments)
  {
    int buffer_length = _template.length () + 500;
    
    char *buffer = new char [buffer_length];

    int length;
    
    for (;;)
      {
        length = vsnprintf 
                    (buffer, buffer_length, _template.c_str (), arguments);

        if (length == buffer_length)
          {
            buffer_length += 1000;
            delete[] buffer;
            buffer = new char [buffer_length];
          }
        
        else
          break;
      }
    
    int const test = mysql_real_query (mysql, buffer, length);

    delete[] buffer;
    
    return test;
  }



  void DB_Connection::void_database_result (MYSQL *const mysql,
                                            string const &template_,
                                            va_list arguments)
  {
    if (_run_query (mysql, template_, arguments) != 0)
      cerr << mysql_error (mysql) << endl;
  }



  string DB_Connection::string_database_result (MYSQL *const mysql,
                                                string const &template_,
                                                va_list arguments)
  {
    if (_run_query (mysql, template_, arguments) != 0)
      {
        cerr << mysql_error (mysql) << endl;
        return string {};
      }

    MYSQL_RES *const results = mysql_store_result (mysql);

    if (! results)
      return string {};

    MYSQL_ROW const row = mysql_fetch_row (results);

    string const ret  =  (row  &&  row [0])  ?  string {row [0]}  :  string {};

    mysql_free_result (results);

    return ret;
  }



  void DB_Connection::instruction  (string const &template_,  ...)
  {
    va_list args; va_start (args, template_);
    void_database_result (&mysql, template_, args);
    va_end (args);
  }


} }  /* End of namespace DMBCS::Trader_Desk::Mysql. */
