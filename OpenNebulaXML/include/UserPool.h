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

	UserPool(SqliteDB * db);

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
        User * user = static_cast<User *>(PoolSQL::get(oid,lock));
    
        return user;
    }
    

    /**
     *  Function to get a User from the pool, if the object is not in memory
     *  it is loaded from the DB
     *    @param username
     *    @param lock locks the User mutex
     *    @return a pointer to the User, 0 if the User could not be loaded
     */
    User * get(
        string  username,
        bool    lock)
    {
        map<string, int>::iterator     index;
    
        index = known_users.find(username);

        if ( index != known_users.end() )
        {
            return get((int)index->second,lock);
        }

        return 0;
    }

    /** Update a particular User 
     *    @param user pointer to User
     *    @return 0 on success
     */
    int update(User * user)
    {
		return user->update(db);
    };
    
    
    /** Drops a user from the DB, the user mutex MUST BE locked
     *    @param user pointer to User
     */
    int drop(User * user)
    {
    	int rc = user->drop(db);
    	
    	if ( rc == 0)
    	{
            known_users.erase(user->get_username());
    	}
        
        return rc;
    };

    /**
     *  Bootstraps the database table(s) associated to the User pool
     */
    static void bootstrap(SqliteDB * _db)
    {
        User::bootstrap(_db);
    };
    
    /**
     * Returns whether there is a user with given username/password or not
     *   @param session, colon separated username and password string
     *   @return -1 if there is no such a user, uid of the user if it exists
     */
    int authenticate(string& session);
        
    /**
     *  Dumps the User pool in XML format. A filter can be also added to the
     *  query
     *  @param oss the output stream to dump the pool contents
     *  @param where filter for the objects, defaults to all
     *
     *  @return 0 on success
     */
    int dump(ostringstream& oss, const string& where)
    {
        int rc;

        oss << "<USER_POOL>";

        rc = User::dump(db,oss,where);

        oss << "</USER_POOL>";

        return rc;
    }

private:
    /**
     *  Factory method to produce User objects
     *    @return a pointer to the new User
     */
    PoolObjectSQL * create()
    {
        return new User;
    };
    
    /**
     *  This map stores the association between UIDs and Usernames
     */
    map<string, int>	known_users;

};

#endif /*USER_POOL_H_*/
