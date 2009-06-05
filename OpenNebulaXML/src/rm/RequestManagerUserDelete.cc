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

#include "RequestManager.h"
#include "Nebula.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void RequestManager::UserDelete::execute(
    xmlrpc_c::paramList const& paramList,
    xmlrpc_c::value *   const  retval)
{ 
    string              session;

    string              username;

    User *              user;

    int                 rc;     
    ostringstream       oss;

    /*   -- RPC specific vars --  */
    vector<xmlrpc_c::value> arrayData;
    xmlrpc_c::value_array * arrayresult;

    Nebula::log("ReM",Log::DEBUG,"UserDelete method invoked");

    // Get the parameters
    session      = xmlrpc_c::value_string(paramList.getString(0));
    username     = xmlrpc_c::value_string(paramList.getString(1));
    
    // Only oneadmin can delete users
    rc = UserDelete::upool->authenticate(session);
    
    if ( rc != 0 )                             
    {                                            
        goto error_authenticate;                     
    }


    // Now let's get the user 
    user = UserDelete::upool->get(username,true);
   
    if ( user == 0) 
    {
        goto error_get_user;
    }
    
    rc = UserDelete::upool->drop(user);
    
    if ( rc != 0 )                             
    {                                            
        goto error_delete;                     
    }    
    
    // All nice, return the new uid to client  
    arrayData.push_back(xmlrpc_c::value_boolean(true)); // SUCCESS
    arrayresult = new xmlrpc_c::value_array(arrayData);
    // Copy arrayresult into retval mem space
    *retval = *arrayresult;
    // and get rid of the original
    delete arrayresult;

    return;

error_authenticate:
    oss << "User not authorized to add new users";
    goto error_common;
    
error_get_user:
    oss << "Error retrieving user " << username;
    goto error_common;

error_delete:
    oss << "Error deleting user " << username;
    goto error_common;

error_common:

    arrayData.push_back(xmlrpc_c::value_boolean(false));  // FAILURE
    arrayData.push_back(xmlrpc_c::value_string(oss.str()));
    
    Nebula::log("ReM",Log::ERROR,oss); 
    
    xmlrpc_c::value_array arrayresult_error(arrayData);

    *retval = arrayresult_error;
    
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
