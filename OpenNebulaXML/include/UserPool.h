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

#ifndef USER_POOL_H_
#define USER_POOL_H_

#include "PoolSQL.h"
#include "User.h"

#include <time.h>
#include <sstream>

#include <iostream>

#include <vector>

using namespace std;

/**
 *  The User Pool class. ...
 */
class UserPool : public PoolSQL
{
public:

	UserPool(SqliteDB * db):PoolSQL(db,Host::table){};

    ~UserPool(){};

    /**
     *  Function to allocate a new User object
     *    @param oid the id assigned to the User
     *    @return 0 on success
     */
    int allocate (
        int *  oid,
        string hostname,
        string password,
        bool   enabled);        

    /**
     *  Function to get a User from the pool, if the object is not in memory
     *  it is loaded from the DB
     *    @param oid User unique id
     *    @param lock locks the User mutex
     *    @return a pointer to the Host, 0 if the User could not be loaded
     */
    User * get(
        int     oid,
        bool    lock)
    {
        return static_cast<User *>(PoolSQL::get(oid,lock));
    };
    
    /** Update a particular User 
     *    @param user pointer to User
     *    @return 0 on success
     */
    int update(User * user)
    {
		return user->update(db);
    };
    
    
    /** Drops a user from the cache & DB, the user mutex MUST BE locked
     *    @param user pointer to User
     */
    int drop(User * user)
    {
    	int rc = user->drop(db);
    	
    	if ( rc == 0)
    	{
    		remove(static_cast<PoolObjectSQL *>(user));	
    	}
        
        return rc;
    };

    /**
     *  Bootstraps the database table(s) associated to the User pool
     */
    void bootstrap()
    {
        User::bootstrap(db);
    };
        
private:
    /**
     *  Factory method to produce User objects
     *    @return a pointer to the new User
     */
    PoolObjectSQL * create()
    {
        return new User;
    };

};

#endif /*USER_POOL_H_*/
