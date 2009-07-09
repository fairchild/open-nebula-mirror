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

void RequestManager::VirtualMachineAction::execute(
    xmlrpc_c::paramList const& paramList,
    xmlrpc_c::value *   const  retval)
{
    string  session;
    string  action;
    int     vid;
    int     rc;

    int     uid;

    vector<xmlrpc_c::value> arrayData;
    xmlrpc_c::value_array * arrayresult;

    Nebula&             nd = Nebula::instance();
    DispatchManager *   dm = nd.get_dm();

    VirtualMachine *    vm;

    ostringstream       oss;

    Nebula::log("ReM",Log::DEBUG,"VirtualMachineAction invoked");

    session = xmlrpc_c::value_string(paramList.getString(0));
    action  = xmlrpc_c::value_string(paramList.getString(1));
    vid     = xmlrpc_c::value_int(paramList.getInt(2));


    // Get the VM
    vm  = VirtualMachineAction::vmpool->get(vid,true);

    if ( vm == 0 )
    {
        goto error_vm_get;
    }

    uid = vm->get_uid(); 
    
    vm->unlock();

    // Only oneadmin or the VM owner can perform operations upon the VM
    rc = VirtualMachineAction::upool->authenticate(session);
    
    if ( rc != 0 && rc != uid)                             
    {                                            
        goto error_authenticate;                     
    }
    
    if (action == "shutdown")
    {
        rc = dm->shutdown(vid);    
    }
    else if (action == "hold")
    {
        rc = dm->hold(vid);
    }
    else if (action == "release")
    {
        rc = dm->release(vid);
    }
    else if (action == "stop")
    {
        rc = dm->stop(vid);
    }
    else if (action == "cancel")
    {
        rc = dm->cancel(vid);
    }
    else if (action == "suspend")
    {
        rc = dm->suspend(vid);
    }
    else if (action == "resume")
    {
        rc = dm->resume(vid);
    }
    else if (action == "restart")
    {
        rc = dm->restart(vid);
    }
    else if (action == "finalize")
    {
        rc = dm->finalize(vid);
    }
    else
    {
        rc = -3;   
    }

    if (rc != 0)
    {
        goto error_operation;

    }

    arrayData.push_back(xmlrpc_c::value_boolean(true));
    arrayresult = new xmlrpc_c::value_array(arrayData);
    *retval = *arrayresult;
    delete arrayresult;
    return;

error_operation:
    if (rc == -1)
    {
        oss << "Virtual machine does not exist";
    }
    else if ( rc == -2 )
    {
        oss << "Wrong state to perform action";
    }
    else if ( rc == -3 )
    {
        oss << "Unknown action";
    }
    goto error_common;

error_vm_get:
    oss << "The virtual machine " << vid << " does not exists";
    goto error_common;

error_authenticate:
    oss << "User not authorized to perform operation upon VirtualMachine [" << vid << "]";
    goto error_common;

error_common:

    arrayData.push_back(xmlrpc_c::value_boolean(false));
    arrayData.push_back(xmlrpc_c::value_string(oss.str()));

    xmlrpc_c::value_array arrayresult_error(arrayData);

    *retval = arrayresult_error;

    return;
 
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
