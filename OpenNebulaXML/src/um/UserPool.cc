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

/* ************************************************************************** */
/* User Pool                                                    			  */
/* ************************************************************************** */

#include "UserPool.h"
#include "Nebula.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

extern "C"
{
    static int getuids_cb(
        void *                  _known_users,
        int                     num,
        char **                 values,
        char **                 names)
    {
        map<string, int> *   known_users;
    
        known_users = static_cast<map<string, int> *>(_known_users);
    
        if ( num == 0 || values == 0 || values[0] == 0 )
        {
            return -1;
        }
    
        known_users->insert(make_pair(values[1],atoi(values[0])));
    
        return 0;
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

UserPool::UserPool(SqliteDB * db):PoolSQL(db,User::table)
{
    // The known_users table needs to be rebuilt
    
    ostringstream   sql;
    
    sql  << "SELECT oid,user_name FROM " <<  User::table;
         
    db->exec(sql, getuids_cb, (void *) &known_users);        
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int UserPool::allocate (
    int *  oid,
    string username,
    string password,
    bool   enabled)
{
    User *        user;

    // Build a new User object

    user = new User(-1,
        username,
        password,
        enabled);

    // Insert the Object in the pool

    *oid = PoolSQL::allocate(user);
    
    // Add the user to the map of known_users
    known_users.insert(make_pair(username,*oid));

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

User * UserPool::get(int  oid,  bool lock)
{
    User * user = static_cast<User *>(PoolSQL::get(oid,lock));
    
    if (user != 0)
    {
        known_users.insert(make_pair(user->get_username(),user->oid));
    }
    
    return user;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int UserPool::authenticate(string username, string password)
{
    map<string, int>::iterator     index;
    
    int                            user_id = -1; 
    
    index = known_users.find(username);

    if ( index != known_users.end() )
    {
        User * user = get((int)index->second,false);
        user_id     = user->get_uid();
    }
    
    return user_id;
}

