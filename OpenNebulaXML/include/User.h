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

#ifndef USER_H_
#define USER_H_

#include "PoolSQL.h"
#include "HostShare.h"
#include "HostTemplate.h"

using namespace std;

extern "C" int host_select_cb (void * _user, int num,char ** values, char ** names);

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 *  The User class. It represents a User...
 */
class User : public PoolObjectSQL
{
public:
    
    /**
     *  Function to write a User on an output stream
     */
     friend ostream& operator<<(ostream& os, User& u);

    /**
     * Get the User unique identifier UID, that matches the OID of the object
     *    @return UID User identifier
     */
    int get_uid() const
    {
        return oid;
    };
    
	// ----- Getters ---------------------------------
    
    /**
     *  Check if the user is enabled
     *    @return true if the user is enabled
     */
     bool isEnabled() const
     {
        return enabled;
     }
    
    /**
     *  Returns user username
     *     @return username User's hostname
     */
 	const string& get_username() const
    {
	    return username;
	};
	
    /**
     *  Returns user password
     *     @return username User's hostname
     */
 	const string& get_password() const
    {
	    return password;
	};
	
	// ----- Setters ---------------------------------
	
	/**
     *   Enables the current user
     */    
    void enable()
    {
        enabled = true;
    };
    
    /**
     *   Disables the current user
     */    
    void disable()
    {
        enabled = false;
    };
	
	/**
     *  Sets user username
     */
 	void set_username(String _username) 
    {
	    return username = _username;
	};
	
	/**
     *  Sets user password
     */
 	void set_password(String _password) 
    {
	    return password = _password;
	};
	
    
private:
    // -------------------------------------------------------------------------
    // Friends
    // -------------------------------------------------------------------------
    
	friend class UserPool;
    
    friend int user_select_cb (
        void *  _user, 
        int     num, 
        char ** values, 
        char ** names);

    // -------------------------------------------------------------------------
    // User Attributes
    // -------------------------------------------------------------------------

    /**
     *  User's username
     */
    string      username;

    /**
     *  User's password
     */
    string      password;

    /**
     * Flag marking user enabled/disabled
     */
    bool        enabled;
 
  
    // *************************************************************************
    // DataBase implementation (Private)
    // *************************************************************************

    /**
     *  Function to unmarshall a User object
     *    @param num the number of columns read from the DB
     *    @para names the column names
     *    @para vaues the column values
     *    @return 0 on success
     */    
    int unmarshall(int num, char **names, char ** values);

    /**
     *  Bootstraps the database table(s) associated to the User
     */
    static void bootstrap(SqliteDB * db)
    {         
        db->exec(User::db_bootstrap);
    };

protected:
	
    // *************************************************************************
    // Constructor
    // *************************************************************************

    User(int     id=-1,
         string _username="",
         string _password="",
         bool   _enabled);

    virtual ~User();
    
    // *************************************************************************
    // DataBase implementation
    // *************************************************************************
    
    enum ColNames
    {
        OID             = 0, 
        USERNAME        = 1, 
        PASSWORD        = 2,  
        ENABLED         = 3,     // 0 = false, 1 = true
        LIMIT           = 4
    };

    static const char * db_names;

    static const char * db_bootstrap;
    
    static const char * table;

    /**
     *  Reads the User (identified with its OID=UID) from the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    virtual int select(SqliteDB *db);

    /**
     *  Writes the User in the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    virtual int insert(SqliteDB *db);

    /**
     *  Writes/updates the User data fields in the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    virtual int update(SqliteDB *db);
    
    /**
     *  Drops USer from the database
     *    @param db pointer to the db
     *    @return 0 on success
     */
    virtual int drop(SqliteDB *db);    
};

#endif /*USER_H_*/
