/* -------------------------------------------------------------------------- */
/* Copyright 2002-2008, Distributed Systems Architecture Group, Universidad   */
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

#ifndef POOL_SQL_H_
#define POOL_SQL_H_

#include <map>
#include <string>
#include <queue>

#include "SqliteDB.h"
#include "PoolObjectSQL.h"
#include "Log.h"

using namespace std;

/**
 * PoolSQL class. Provides a base class to implement persistent generic pools.
 * The PoolSQL provides a synchronization mechanism (mutex) to operate in 
 * multithreaded applications. Any modification or access function to the pool 
 * SHOULD block the mutex.
 */
class PoolSQL
{
public:

    /**
     * Initializes the oid counter. This function sets lastOID to
     * the last used Object identifier by querying the corresponding database
     * table. This function SHOULD be called before any pool related function.
     *   @param _db a pointer to the database
     *   @param table the name of the table supporting the pool (to set the oid 
     *   counter). If null the OID counter is not updated.
     */
    PoolSQL(SqliteDB * _db, const char * table=0);

    virtual ~PoolSQL();

    /**
     *  Allocates a new object, writting it in the pool database. No memory is
     *  allocated for the object.
     *   @param objsql an initialized ObjectSQL
     *   @return the oid assigned to the object or -1 in case of failure
     */
    virtual int allocate(
        PoolObjectSQL   *objsql);

    /**
     *  Gets an object from the pool (if needed the object is loaded from the 
     *  database).
     *   @param oid the object unique identifier
     *   @param lock locks the object if true
     * 
     *   @return a pointer to the object, 0 in case of failure
     */
    virtual PoolObjectSQL * get(
        int     oid,
        bool    lock);

    /**
     *  Finds a set objects that satisfies a given condition
     *   @param oids a vector with the oids of the objects.
     *   @param the name of the DB table.
     *   @param where condition in SQL format.
     *   
     *   @return 0 on success 
     */
    virtual int search(
        vector<int>&    oids,
        const char *    table,
        const string&   where);
                    
    /**
     *  Updates the object's data in the data base. The object mutex SHOULD be
     *  locked.
     *    @param objsql a pointer to the object
     * 
     *    @return 0 on success.
     */
    virtual int update(
        PoolObjectSQL * objsql)
    {
        return objsql->update(db);
    };

    /**
     *  Removes an object from the pool cache. The object mutex MUST be locked.
     *  The resources of the object are freed, but its record is kept in the DB.
     *    @param objsql a pointer to the object
     */
    void remove(
        PoolObjectSQL * objsql);
        
    /**
     *  Removes all the elements from the pool
     */
    void clean();
     
    /**
     *  Bootstraps the database table(s) associated to the pool
     */
    virtual void bootstrap() = 0;
        
protected:

    /**
     *  Pointer to the database.
     */
    SqliteDB *  db;
    
    /**
     *  Function to lock the pool
     */
    void lock()
    {
        pthread_mutex_lock(&mutex);
    };

    /**
     *  Function to unlock the pool
     */
    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    };
        
private:

    pthread_mutex_t             mutex;

    /**
     *  Max size for the pool, to control the memory footprint of the pool. This 
     *  number MUST be greater than the max. number of objects that are
     *  accessed simultaneously.
     */
    static const unsigned int   MAX_POOL_SIZE;

    /**
     *  Last object ID assigned to an object. It must be initialized by the
     *  target pool.
     */
    int                         lastOID;

    /**
     *  The pool is implemented with a Map, of SQL object pointers, using the 
     *  OID as key.
     */
    map<int,PoolObjectSQL *>	pool;

    /**
     *  Factory method, must return an ObjectSQL pointer to an allocated pool
     *  specific object.
     */
    virtual PoolObjectSQL * create() = 0;

    /**
     *  OID queue to implement a FIFO-like replacement policy for the pool 
     *  cache. 
     */
    queue<int>					oid_queue;

    /**
     *  FIFO-like replacement policy function. Before removing an object (pop)
     *  from  the cache it's lock is checked. The object is removed only if 
     *  the associated mutex IS NOT blocked. Otherwise the oid is sent to the 
     *  back of the queue.  
     */
    void replace();
};

#endif /*POOL_SQL_H_*/
