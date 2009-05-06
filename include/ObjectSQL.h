/* -------------------------------------------------------------------------- */
/* Copyright 2002-2009, Distributed Systems Architecture Group, Universidad   */
/* Complutense de Madrid (dsa-research.org)                                   */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#ifndef OBJECT_SQL_H_
#define OBJECT_SQL_H_

#include "SqliteDB.h"

using namespace std;

/**
 * ObjectSQL class. Provides a SQL backend interface, it should be implemented
 * by persistent objects.
 */

class ObjectSQL
{
public:

    ObjectSQL(){};

    virtual ~ObjectSQL(){};

protected:
	/**
     *  Gets the value of a column in the pool for a given object
     *    @param db pointer to Database
     *    @param table supporting the object
     *    @param column to be selected
     *    @param where contidtion to select the column
     *    @param value of the column
     *    @return 0 on success
     */
    int select_column(
        SqliteDB *      db,
        const string&   table,
        const string&   column,
        const string&   where,
        string *        value);

    /**
     *  Reads the ObjectSQL (identified with its OID) from the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    virtual int select(
        SqliteDB * db) = 0;

    /**
     *  Writes the ObjectSQL in the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    virtual int insert(
        SqliteDB * db) = 0;

    /**
     *  Updates the ObjectSQL in the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    virtual int update(
        SqliteDB * db) = 0;

    /**
     *  Removes the ObjectSQL from the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    virtual int drop(
        SqliteDB * db) = 0;
};

#endif /*OBJECT_SQL_H_*/
