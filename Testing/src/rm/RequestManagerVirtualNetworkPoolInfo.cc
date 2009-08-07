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

void RequestManager::VirtualNetworkPoolInfo::execute(
    xmlrpc_c::paramList const& paramList,
    xmlrpc_c::value *   const  retval)
{ 
    string        session;
    string        username;
    string        password;

    ostringstream oss;
    ostringstream where_string;

	int			  rc;
    int           filter_flag;

    User *        user;
	
    /*   -- RPC specific vars --  */
    vector<xmlrpc_c::value> arrayData;
    xmlrpc_c::value_array * arrayresult;

    Nebula::log("ReM",Log::DEBUG,"VirtualNetworkPoolInfo method invoked");

    // Get the parameters
    session     = xmlrpc_c::value_string(paramList.getString(0));
    filter_flag = xmlrpc_c::value_int(paramList.getInt(1));

    // Check if it is a valid user
    rc = VirtualNetworkPoolInfo::upool->authenticate(session);

    if ( rc == -1 )
    {
        goto error_authenticate;
    }

    where_string.str("");
    
    /** Filter flag meaning table
     *    <=-2 :: ALL VMs
     *     -1  :: User's VMs
     *    >=0  :: UID User's VMs
     **/
    if (filter_flag == -1)
    {
        User::split_secret(session,username,password);

        // Now let's get the user
        user = VirtualNetworkPoolInfo::upool->get(username,true);

        if ( user == 0 )
        {
            goto error_get_user;
        }

        where_string << "UID=" << user->get_uid();

        user->unlock();
    }
    else if (filter_flag>=0)
         {
            where_string << "UID=" << filter_flag;
         }

    // Perform the allocation in the vmpool 
    rc = VirtualNetworkPoolInfo::vnpool->dump(oss,where_string.str());
      
    if ( rc != 0 )
    {                                            
        goto error_dump;
    }
    
    //All nice, return the host info to the client  
    arrayData.push_back(xmlrpc_c::value_boolean(true)); // SUCCESS   
    arrayData.push_back(xmlrpc_c::value_string(oss.str()));

    arrayresult = new xmlrpc_c::value_array(arrayData);

    // Copy arrayresult into retval mem space
    *retval = *arrayresult;

    // and get rid of the original
    delete arrayresult;

    return;

error_authenticate:
    oss << "User not authenticated, NetworkPoolInfo call aborted.";
    goto error_common;

error_get_user:
    oss << "Error getting user, aborting NetworkPoolInfo call";
    goto error_common;

error_dump:
    oss << "Error getting virtual network pool"; 
    goto error_common;

error_common:

    arrayData.push_back(xmlrpc_c::value_boolean(false)); // FAILURE
    arrayData.push_back(xmlrpc_c::value_string(oss.str()));
    
    Nebula::log("ReM",Log::ERROR,oss); 
    
    xmlrpc_c::value_array arrayresult_error(arrayData);

    *retval = arrayresult_error;
    
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
