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

#ifndef VIRTUAL_NETWORK_POOL_H_
#define VIRTUAL_NETWORK_POOL_H_

#include "PoolSQL.h"
#include "VirtualNetwork.h"

#include <time.h>

using namespace std;


/**
 *  The Virtual Network Pool class. ...
 */
class VirtualNetworkPool : public PoolSQL
{
public:

    VirtualNetworkPool(SqliteDB * db):PoolSQL(db,VirtualNetwork::table){};

    ~VirtualNetworkPool(){};

    /**
     *  Function to allocate a new VN object
     *    @param stemplate a string describing the VM
     *    @param oid the id assigned to the VM (output)
     *    @return 0 on success, -1 error inserting in DB or -2 error parsing 
     *     the template
     */
    int allocate (
        int     uid,
        const  string& nwtemplate,
        int *  oid,
        );

    /**
     *  Function to get a VN from the pool, if the object is not in memory
     *  it is loade from the DB
     *    @param oid VN unique id
     *    @param lock locks the VN mutex
     *    @return a pointer to the VN, 0 if the VN could not be loaded
     */
    VirtualNetwork * get(
        int     oid,
        bool    lock)
    {
        return static_cast<VirtualNetwork *>(PoolSQL::get(oid,lock));
    };

    //--------------------------------------------------------------------------
    // Virtual Netowrk DB access functions
    //--------------------------------------------------------------------------
    
    /**
     *  Updates the template of a VN, adding a new attribute (replacing it if 
     *  already defined), the vn's mutex SHOULD be locked
     *    @param vn pointer to the virtual network object
     *    @param name of the new attribute
     *    @param value of the new attribute
     *    @return 0 on success
     */
    int update_template_attribute(
        VirtualNetwork *	vn,
        string&			 	name,
        string&			 	value)
    {
    	return vn->update_template_attribute(db,name,value);
    }
    
    /**
     *  Updates the history record of a VN, the vm's mutex SHOULD be locked
     *    @param vm pointer to the virtual machine object
     *    @return 0 on success
     */
    int update_history(
        VirtualNetwork * vn)
    {
        return vn->update_history(db);
    }

    /**
     *  Updates the previous history record, the VN's mutex SHOULD be locked
     *    @param vm pointer to the virtual network object
     *    @return 0 on success
     */
    int update_previous_history(
        VirtualNetwork * vn)
    {
        return vn->update_previous_history(db);
    }
    
    /**
     *  Bootstraps the database table(s) associated to the VirtualNetwork pool
     */
    void bootstrap()
    {
        VirtualMachine::bootstrap(db);
    };
    
private:

    /**
     *  Factory method to produce VN objects
     *    @return a pointer to the new VN
     */
    PoolObjectSQL * create()
    {
        return new VirtualNetwork;
    };
};
 
#endif /*VIRTUAL_NETWORK_POOL_H_*/
