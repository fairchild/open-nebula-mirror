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

void RequestManager::UserAllocate::execute(
    xmlrpc_c::paramList const& paramList,
    xmlrpc_c::value *   const  retval)
{ 
    string              session;

    string              username;
    string              password; 
    
    int                 uid;

    int                 rc;     
    ostringstream       oss;

    User              * user;

    /*   -- RPC specific vars --  */
    vector<xmlrpc_c::value> arrayData;
    xmlrpc_c::value_array * arrayresult;

    Nebula::log("ReM",Log::DEBUG,"UserAllocate method invoked");

    // Get the parameters
    session      = xmlrpc_c::value_string(paramList.getString(0));
    
    username     = xmlrpc_c::value_string(paramList.getString(1));
    password     = xmlrpc_c::value_string(paramList.getString(2));
    
    // Only oneadmin can add users
    rc = UserAllocate::upool->authenticate(session);
    
    if ( rc != 0 )                             
    {                                            
        goto error_authenticate;                     
    }

    // Let's make sure that the user doesn't exist in the database
    user = UserAllocate::upool->get(username,false);

    if (user != 0 )
    {
        goto error_duplicate;
    }
    
    // Now let's add the user
    rc = UserAllocate::upool->allocate(&uid,username,password,true);
    
    if ( rc != 0 )                             
    {                                            
        goto error_allocate;                     
    }    
    
    // All nice, return the new uid to client  
    arrayData.push_back(xmlrpc_c::value_boolean(true)); // SUCCESS
    arrayData.push_back(xmlrpc_c::value_int(uid));

    // Copy arrayresult into retval mem space
    arrayresult = new xmlrpc_c::value_array(arrayData);
    *retval = *arrayresult;
    
    delete arrayresult; // and get rid of the original

    return;

error_authenticate:
    oss << "User not authorized to add new users";
    goto error_common;
    
error_duplicate:
    oss << "Existing user, cannot duplicate";
    goto error_common;


error_allocate:
    oss << "Error allocating user";
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
