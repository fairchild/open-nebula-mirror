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

#include <limits.h>
#include <string.h>

#include <iostream>
#include <sstream>

#include "User.h"

/* ************************************************************************** */
/* User :: Constructor/Destructor                                  */
/* ************************************************************************** */

User::User(
    int     id,
    string  _username,
    string  _password,
    bool    _enabled):
        PoolObjectSQL(id),
        username     (_username),
        password     (_password),
        enabled      (_enabled)
        {};


User::~User(){};

/* ************************************************************************** */
/* User :: Database Access Functions                               */
/* ************************************************************************** */

const char * User::table = "user_pool";

const char * User::db_names = "(oid,user_name,password,enabled)";

const char * User::db_bootstrap = "CREATE TABLE user_pool ("
	"oid INTEGER PRIMARY KEY,user_name TEXT,password TEXT,"
	"enabled INTEGER)";

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int User::unmarshall(int num, char **names, char ** values)
{
    if ((!values[OID]) ||
        (!values[USERNAME]) ||
        (!values[PASSWORD]) ||
        (!values[ENABLED]) ||
        (num != LIMIT ))
    {
        return -1;
    }

    oid      = atoi(values[OID]);
    username = values[USERNAME];
    password = values[PASSWORD];
    enabled  = (atoi(values[ENABLED])==0)?false:true;
    
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

extern "C" int user_select_cb (
        void *                  _user,
        int                     num,
        char **                 values,
        char **                 names)
{
    User *    user;

    user = static_cast<User *>(_user);

    if (user == 0)
    {
        return -1;
    }

    return user->unmarshall(num,names,values);
};

/* -------------------------------------------------------------------------- */

int User::select(SqliteDB *db)
{
    ostringstream   oss;
    int             rc;
    int             boid;
    
    oss << "SELECT * FROM " << table << " WHERE oid = " << oid;

    boid = oid;
    oid  = -1;

    rc = db->exec(oss, user_select_cb, (void *) this);

    if ((rc != 0) || (oid != boid ))
    {
        return -1;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int User::insert(SqliteDB *db)
{
    int rc;
    
    //Insert the Host and its template
    rc = update(db);

    if ( rc != 0 )
    {
        return rc;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int User::update(SqliteDB *db)
{
    ostringstream   oss;
    
    int    rc;

    char * sql_username;
    char * sql_password;
    
    int    str_enabled = enabled?1:0;
    
    // Update the User
    
    sql_username = sqlite3_mprintf("%q",username.c_str());

    if ( sql_username == 0 )
    {
        goto error_username;
    }
    
    sql_password = sqlite3_mprintf("%q",password.c_str());

    if ( sql_password == 0 )
    {
        goto error_password;
    }
   
    // Construct the SQL statement to Insert or Replace (effectively, update)

    oss << "INSERT OR REPLACE INTO " << table << " "<< db_names <<" VALUES ("
        << oid << ","
        << "'" << sql_username << "',"
        << "'" << sql_password << "',"
        << str_enabled << ")";

    rc = db->exec(oss);

    sqlite3_free(sql_username);
    sqlite3_free(sql_password);

    return rc;
    
error_password:
    sqlite3_free(sql_username);
error_username:
    return -1;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int User::unmarshall(ostringstream& oss,
                     int            num,
                     char **        names,
                     char **        values)
{
    if ((!values[OID]) ||
        (!values[USERNAME]) ||
        (!values[PASSWORD]) ||
        (!values[ENABLED]) ||
        (num != LIMIT))
    {
        return -1;
    }
    
    string str_enabled = (atoi(values[ENABLED])==0)?"Fase":"True";

    oss <<
        "<USER>" <<
            "<UID>"          << values[OID]           <<"</UID>"       <<
            "<USER_NAME>"    << values[USERNAME]      <<"</USER_NAME>" <<
            "<PASSWORD>"     << values[PASSWORD]      <<"</PASSWORD>"  <<
            "<ENABLED>"      << str_enabled           <<"</ENABLED>"   <<
        "</USER>";

    return 0;

}

/* -------------------------------------------------------------------------- */

extern "C" int user_dump_cb (
        void *                  _oss,
        int                     num,
        char **                 values,
        char **                 names)
{
    ostringstream * oss;
    ostringstream dbg;

    oss = static_cast<ostringstream *>(_oss);

    if (oss == 0)
    {
        return -1;
    }

    return User::unmarshall(*oss,num,names,values);
};

/* -------------------------------------------------------------------------- */

int User::dump(SqliteDB * db, ostringstream& oss, const string& where)
{
    int             rc;
    ostringstream   cmd;

    cmd << "SELECT * FROM " << User::table;

    if ( !where.empty() )
    {
        cmd << " WHERE " << where;
    }

    rc = db->exec(cmd,user_dump_cb,(void *) &oss);

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int User::drop(SqliteDB * db)
{
    ostringstream   oss;
    
    // Third, drop the user itself
    oss << "DELETE FROM " << table << " WHERE oid=" << oid;

    return db->exec(oss);
}

/* ************************************************************************** */
/* User :: Misc                                                               */
/* ************************************************************************** */

ostream& operator<<(ostream& os, User& user)
{
	string user_str;
	
	os << user.to_xml(user_str);
	
    return os;
};


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

string& User::to_xml(string& xml) const
{
    ostringstream   oss;
    
    int  enabled_int = enabled?1:0;
 
    oss << 
    "<USER>"                             
         "<UID>"          << oid            <<"</UID>"       <<
         "<USER_NAME>"    << username       <<"</USER_NAME>" <<
         "<PASSWORD>"     << password       <<"</PASSWORD>"  <<
         "<ENABLED>"      << enabled_int    <<"</ENABLED>"   <<
    "</USER>";

    xml = oss.str();

    return xml;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

string& User::to_str(string& str) const
{
    ostringstream   os;

    string enabled_str = enabled?"True":"False";

    os << 
        "UID      = "  << oid            << endl <<
        "USERNAME = "  << username       << endl <<
        "PASSWORD = "  << password       << endl <<
        "ENABLED  = "  << enabled_str;

    str = os.str();
    
    return str;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int User::authenticate(string _password)
{
    if (enabled && _password==password)
    {
        return oid;
    }
    else
    {
        return -1; 
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
