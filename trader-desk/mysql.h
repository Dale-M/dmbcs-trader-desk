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


#ifndef DMBCS__TRADER_DESK__MYSQL__H
#define DMBCS__TRADER_DESK__MYSQL__H


#include <chrono>
#include <string>
#include <sstream>
#include <cstdarg>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <trader-desk/preferences.h>

#if HAVE_MYSQL
#   include <mysql/mysql.h>
#endif

#if HAVE_MARIADB
#   include <mariadb/mysql.h>
#endif


/** \file
 *
 *  Definition of \c Mysql::DB_Connection, \c Mysql::Instruction, \c
 *  Mysql::Quick_Instruction, \c Mysql::Simple_Query and \c
 *  Mysql::Row_Query, objects which capture between them all the specifics
 *  of operating a MySQL/MariaDB database. */

/*  Note that it is the intention to base these classes on abstract ones,
 *  and thence to provide alternative database back-ends. */


namespace DMBCS::Trader_Desk { 


 using namespace std;


 /** This namespace encaptures everything which is specific to a
  *  MySQL/MariaDB database back-end (specifically \c libmysqlclient).
  *  Nothing outside should have any inkling of the fact that we are using
  *  such a database. */
 namespace Mysql {


  /** Object which provides std::ostream-type features to develop an SQL
   *  query string (should be one which produces no useful results) and
   *  then allows for its execution.  If this produces some unique \c
   *  seqid (e.g. by inserting into a table with an automatic index
   *  column), then that value can subsequently be obtained with the \c
   *  insert_id method. */

  struct Instruction
  {
    /** The connection to the database which we will be using. */
    MYSQL *mysql;

    /** Flag to indicate if we should print any error messages (\c FALSE)
     *  or not. */
    bool const no_error;

    /** Place where we accumulate the query string. */
    unique_ptr <ostringstream> buffer;

    
    /** Flag for the constructor which allows to specify that no error
     *  messages should be printed. */
    enum : bool { NO_ERROR = true };

    /** Sole constructor which takes a connection to the database and an
     *  optional flag (see above) to indicate no error messages should
     *  appear. */
    explicit Instruction (MYSQL *const m, bool const ne = false)
      : mysql {m},
        no_error {ne},
        buffer {make_unique<ostringstream> ()} 
    {}
    
    
    /** We can't copy these objects as that would wreak havoc with the \c
     *  buffer, but we want to be able to move them so they can be passed
     *  back from factory functions. */
    Instruction (Instruction const &) = delete;
    Instruction (Instruction &&) = default;
    Instruction &operator= (Instruction const &) = delete;
    Instruction &operator= (Instruction &&) = default;
    
    
    /** This is the method which makes our class look like an
     *  std::ostream. */
    template <typename T>
    Instruction &operator<< (T const &i)  { *buffer << i; return *this; }

    /** Send the query string, which should by now have been sent into the
     *  \c buffer, to the database and return the resulting status (zero
     *  is success) of the operation. */
    int execute ();

    /** If the query caused an auto-incrementing table column to be
     *  updated, this method will return the last value assigned. */
    int insert_id ()   {  return mysql_insert_id (mysql);  }

  } ;   /* End of class Instruction. */



  /** A \c Quick_Instruction is just a \c Instruction which
   *  executes the query on object destruction, allowing for one-line
   *  instructions to make modifications to the database,
   *  e.g. `Quick_Instruction {} << "update table ...";'. */
  struct Quick_Instruction : Instruction
  {
    /** All construction, move, no-copying construction is exactly as \c
     *  Instruction. */
    using Instruction::Instruction;

    Quick_Instruction (Quick_Instruction const &) = delete;
    Quick_Instruction (Quick_Instruction &&m) = default;
    Quick_Instruction &operator= (Quick_Instruction const &) = delete;
    Quick_Instruction &operator= (Quick_Instruction &&) = delete;


    /** Do the work in the destructor. */
    ~Quick_Instruction () { Instruction::execute (); }

    /*  Make sure the user can't accidentally call the execution
     *  directly. */
    int execute () = delete;
  };



  /** A \c Simple_Query is like a \c Instruction which returns
   *  a single meaningful value.  The usage pattern is to construct,
   *  assemble a query with the inherited \c operator<<, and then call \c
   *  return_scalar to get the solitary resulting value. */
  struct Simple_Query : Instruction
  {
    /** Construction, move, non-copy is exactly as for \c
     *  Instruction. */
    using Instruction::Instruction;
    
    Simple_Query (Simple_Query const &) = delete;
    Simple_Query (Simple_Query &&) = default;
    Simple_Query &operator= (Simple_Query const &) = delete;
    Simple_Query &operator= (Simple_Query &&) = default;
    
    /** Execute the assembled query and return the result, cast to type \c
     *  T.  The \a fallback both determines the return type and also the
     *  value that will be returned if the database fails to provide
     *  this. */
    template <typename T>
    T return_scalar (T const &fallback = T ());

    /** Make sure the user can't accidentally call for the execution
     *  directly. */
    int execute () = delete;
  } ;



  /** This class represents a database query which produces (selects)
   *  multiple rows of multiple columns of results.  It is used exactly as
   *  a \c Instruction, but after calling the \c execute method the
   *  class provides iterators and other convenience access methods for
   *  retrieving the data. */
  class Row_Query : public Instruction
  {
    /** The result from the database query.  Note that this is a resource
     *  we manage locally, so have to be careful with class move and
     *  destruction. */
    MYSQL_RES *result {nullptr};

    /** An index into the rows of the \c result. */
    MYSQL_ROW row;

    /** An index into the columns of the \c row we are currently
     *  examining. */
    int next_index;

    
  public:
    
    /** Construction, move and non-copy is the same as for the base
     *  classes, but we have to take care with the handling of our
     *  controlled resource: the \c result. */
    using Instruction::Instruction;
    
    Row_Query (Row_Query const &) = delete;

    Row_Query (Row_Query &&m) 
      : Instruction {move (m)}, 
        result {m.result}, row {m.row}, next_index {m.next_index}
    {  m.result = nullptr;  }
      
    Row_Query &operator= (Row_Query const &) = delete;

    Row_Query &operator= (Row_Query &&m)
    {
      result = m.result;  m.result = nullptr;
      row = m.row;
      next_index = m.next_index;
      return *this;
    }

    
    ~Row_Query ()   {  if (result)  mysql_free_result (result);  }

    
    /** Perform the database query, and then obtain a result manager,
     *  fetch the first row of results, and set up the result indexers to
     *  indicate that the first column of the first row will be the next
     *  available result value. */
    Row_Query &execute ()  {  Instruction::execute ();
                              result = mysql_store_result (mysql);
                              row = result ? mysql_fetch_row (result) : 0;
                              next_index = 0;
                              return *this;  }


    /** After \c execute has been called, return the number of rows of
     *  data that are available. */
    int number_rows () const   {  return mysql_num_rows (result);  }


    /** Skip over \a count columns in the current row. */
    Row_Query &skip_entry (const int count = 1)  {  next_index += count;
                                                    return *this;  }

    
    /** Read the next result value into \a ret, and advance the indexers
     *  to the next column. */
    template <typename T>
    Row_Query &operator>> (T &ret)
    {
      istringstream in (row [next_index++]);
      in >> ret;

      return *this;
    }
    
    
    /** Return the next value cast to type \c T, returning \a fallback if
     *  the database did not provide a valid value, and advancing the
     *  index along to the next column so that subsequent calls to this
     *  method automatically iterate through the values of the row.  This
     *  method can be use interchangeably with the previous one (\c
     *  operator>>), according to convenience. */
    template <typename T> T next_entry (const T&  fallback  =  {})
    {
      if (row [next_index] == 0)   {   ++next_index;
                                       return fallback;    }
      
      T ret;
      *this >> ret;
      return ret;
    }

    
    /** Return \c TRUE if there are more rows to reap data for. */
    operator bool () const  { return row; }

    
    /** Iterate to the next row in the result data set.  Combined with the
     *  above result this allows for the straight-forward implementation
     *  of \c for(;;) loops over all the rows in a result set. */
    void operator++ ()   {  row = mysql_fetch_row (result);
                            next_index = 0;  }


  } ;   /* End of class Row_Query. */



  /** Essentially just a MYSQL object which automatically opens and closes
   *  a connection to the \c trader-desk database on object construction
   *  and destruction.
   *
   *  The sole constructor may throw an exception if a database connection
   *  cannot be established.  The copy constructor is implicitly deleted,
   *  the move constructors implicitly defined.
   *
   *  Once the connection is established, the class provides factory
   *  methods for the above query object types, plus a couple of
   *  convenience functions which allow for one-line printf-style query
   *  specification and execution.
   *
   *  Note that most of the connection parameters come through the
   *  config.h file, except the password which is passed directly on the
   *  compiler command line, via makefile.am and configure.ac; we do not
   *  store it in any source files, hence it doesn't get into GIT, but do
   *  beware that the raw string is present in built binaries and, no
   *  doubt, in infrastructure files in the package build directory. */
  
  struct DB_Connection
  {
    /** Thrown if a connection to the RDBMS cannot be made. */
    struct Exception : runtime_error 
    { using runtime_error::runtime_error; };

    /** The real connection object which we are wrapping. */
    MYSQL mysql;
    bool  initialized  {0};


    /** Establish a connection to the RDBMS.  May throw an \c Exception
     *  object. */
    explicit  DB_Connection  (const Preferences&);


    /** We only want one of these for the application, so copy and move
     *  are irrelevant and are \c delete'd. */
    DB_Connection (DB_Connection const &) = delete;
    DB_Connection (DB_Connection &&m) = delete;
    DB_Connection &operator= (DB_Connection const &) = delete;
    DB_Connection &operator= (DB_Connection &&m)  = delete;

    
    /** Close the connection to the database. */
    ~DB_Connection ()   {  if (initialized)  mysql_close (&mysql);  }


    /** Manufacture an \c Instruction. */
    Instruction instruction () { return Instruction {&mysql}; }

    /** Manufacture a \c Quick_Instruction. */
    Quick_Instruction quick () { return Quick_Instruction {&mysql};}

    /** Manufacture a \c Simple_Query. */
    Simple_Query simple_query ()  { return Simple_Query {&mysql};  }

    /** Manufacture a \c Row_Query. */
    Row_Query row_query ()     { return Row_Query {&mysql};  }
    


    /** Implementation of following (\c instruction) method. */
    static void void_database_result (MYSQL *const mysql,
                                      string const &template_,
                                      va_list arguments);
  
    /** Execute a one-shot SQL statement on the database, expressed
     *  through the printf-style \a template_ and arguments.  There can be
     *  no return information, thus the query should be one which does not
     *  return any. */
    void instruction  (string const &template_,  ...);


  
    /** Implementation of following (\c scalar_result) method. */
    static string string_database_result (MYSQL *const mysql,
                                          string const &template_,
                                          va_list arguments);
  
    /** Make a query on the database which obtains a single-valued result.
     *  The \a fallback_value serves both to define the type of result
     *  returned, and sets the returned value in the case that the
     *  database is unable to furnish the information.  The SQL query
     *  itself is composed of the printf()-type string \a template_ with
     *  substitutions from the following arguments. */
    template <typename T>
    T scalar_result (T const &fallback_value,
                     string const &template_,
                     ...);


    /** Either make a connection to the database (\c mysql must *not*
     *  already be connected), or else throw an \c Exception. */
    void connect (const Preferences&);

    /** Close and re-make the connection to the database. */
    void reconnect (const Preferences&);


  } ;  /* End of class DB_Connection. */



  /*******************************************************************
   ******************  Template implementations   ********************
   *******************************************************************/


  template <typename T>
  inline T Simple_Query::return_scalar (T const &fallback)
  {
    string const hold = return_scalar (string {"@@"});

    if (hold.length () == 2  &&  hold == "@@")  return fallback;
    
    T ret; istringstream i {hold}; i >> ret;
    
    return ret;
  }



  template<> 
  inline string Simple_Query::return_scalar<string> (string const &fallback)
  {
    if (Instruction::execute () != 0)   return fallback;
    
    MYSQL_RES *const results = mysql_store_result (mysql);
    
    if (! results)   return fallback;
    
    MYSQL_ROW const row = mysql_fetch_row (results);
    
    string const ret   =   row   ?   string {row [0]}  :  fallback;
    
    mysql_free_result (results);
    
    return ret;
  }



  template <> 
  inline string Row_Query::next_entry<string> (string const &fallback)
  {
    auto const &ret = row [next_index++];
    return ret ? ret : fallback;
  }



  template <>
  inline Row_Query &Row_Query::operator>> <string> (string &ret)
  {
    ret = next_entry (string {});
    return *this;
  }



  template <>
  inline Row_Query &Row_Query::operator>> <chrono::system_clock::time_point>
                               (chrono::system_clock::time_point &ret)
  {
    time_t t;
    (*this) >> t;
    ret = chrono::system_clock::from_time_t (t);
    return *this;
  }



  template <typename T>
  inline T DB_Connection::scalar_result (T const &fallback_value,
                                         string const &template_,
                                         ...)
  {
    va_list args; va_start (args, template_);
    string const value {string_database_result (&mysql, template_, args)};
    va_end (args);

    if (! value.length ())
      return fallback_value;

    istringstream i (value);
    T ret;
    i >> ret;

    return ret;
  }



  template <>
inline string DB_Connection::scalar_result (const string&  fallback_value,
                                            const string&  template_,
                                              ...)
  {
    va_list args; va_start (args, template_);
    const string  value  {string_database_result (&mysql, template_, args)};
    va_end (args);

    if (! value.length ())   return fallback_value;

    return value;
  }


} }  /* End of namespace DMBCS::Trader_Desk::Mysql. */
 

#endif  /* Defined DMBCS__TRADER_DESK__MYSQL__H. */
