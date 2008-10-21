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

#ifndef VIRTUAL_NETWORK_H_
#define VIRTUAL_NETWORK_H_


#include "PoolSQL.h"
#include <vector.h>
#include <string>
#include <map>

#include <time.h>
#include <sstream>

using namespace std;

extern "C" int vnw_select_cb (void * _vnw, int num,char ** values, char ** names);

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 *  The Virtual Network class. It represents a Virtual Network at manages its leases.
 *  One lease is formed by one IP and one MAC address.
 *  MAC address are derived from IP addresses.
 */
class VirtualMachine : public PoolObjectSQL
{
public:
    
    // *************************************************************************
    // Virtual Network Public Methods
    // *************************************************************************
    /**
     *    Gets a new lease for a specific VM
     *    @param vid VM identifier
     *    @param IP pointer to string for IP to be stored into
     *    @param MAC pointer to string for MAC to be stored into
     *    @param bridge name of the physical bridge this VN binds to 
     *    @return true if success, false if failure (no more leases or otherwise)
     */
    bool getLease(int vid, string& IP, string& MAC, string& bridge);
 
        
private:

    // -------------------------------------------------------------------------
    // Friends
    // -------------------------------------------------------------------------
    friend class VirtualNetworkPool;

    friend int vnw_select_cb (
        void *  _vm,
        int     num,
        char ** values,
        char ** names);

    // *************************************************************************
    // Virtual Network Private Attributes
    // *************************************************************************

    // -------------------------------------------------------------------------
    // Identification variables
    // -------------------------------------------------------------------------
    /**
     *  Name of the Virtual Network
     */
    string         name;
    
    /**
     *  Owner of the Virtual Network
     */
    string         uid;

    // -------------------------------------------------------------------------
    // Binded physical attributes
    // -------------------------------------------------------------------------
    
    /**
     *  Name of the bridge this VNW binds to
     */
    string         bridge;

    // -------------------------------------------------------------------------
    // Virtual Network Description
    // -------------------------------------------------------------------------
    /**
     *  Pointer to leases class, can be fixed or ranged.
     *  Holds information on given (and, optionally, possible) leases
     */
    Leases     *   leases;
    
    /**
     * Possible types of networks
     */ 
    
    enum NetworkType
    {
        RANGED          = 0,
        FIXED           = 1,
    };
    
    /**
     * Holds the type of this network
     */
    NetworkType             type;
    
    
    /**
     *  The Virtual Network template, holds the VNW attributes.
     */
    VirtualNetworkTemplate  vnw_template;
    
    // *************************************************************************
    // Virtual Network Private Methods
    // *************************************************************************    
    

    // *************************************************************************
    // DataBase implementation (Private)
    // *************************************************************************

    /**
     *  Bootstraps the database table(s) associated to the Virtual Network
     */
    static void bootstrap(SqliteDB * db)
    {
        db->exec(VirtualNetwork::db_bootstrap);
        
        db->exec(Leases::db_bootstrap);

        db->exec(VirtualNetworkTemplate::db_bootstrap);
    };
    
    /**
     *  Function to unmarshall a VNW object, and associated classes.
     *    @param num the number of columns read from the DB
     *    @para names the column names
     *    @para vaues the column values
     *    @return 0 on success
     */
    int unmarshall(int num, char **names, char ** values);
    
    /**
     *  Updates the template of a VNW, adding a new attribute (replacing it if 
     *  already defined), the vm's mutex SHOULD be locked
     *    @param vm pointer to the virtual machine object
     *    @param name of the new attribute
     *    @param value of the new attribute
     *    @return 0 on success
     */
    int update_template_attribute(
    	SqliteDB * 			db,
        string&			 	name,
        string&			 	value)
    {
    	return vnw_template.replace_attribute(db,name,value);
    }    
    
protected:
	
    //**************************************************************************
    // Constructor
    //**************************************************************************
    
    VirtualNetwork(int         oid    = -1,
                   int         uid    = -1,
                   string      name   = "",
                   string      bridge = "",
                   NetworkType type );

    virtual ~VirtualNetwork();
    
    // *************************************************************************
    // DataBase implementation
    // *************************************************************************

	enum ColNames
    {
        OID             = 0,
        UID             = 1,
        NAME            = 2,
        TYPE            = 3,
        SIZE            = 4,
        BRIDGE          = 5,
        LIMIT           = 6
    };

    static const char * table;

    static const char * db_names;

    static const char * db_bootstrap;
    
    /**
     *  Reads the Virtual Network (identified with its OID) from the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    int select(SqliteDB * db);

    /**
     *  Writes the Virtual Network and its associated template and leases in the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    int insert(SqliteDB * db);

    /**
     *  Writes/updates the Virtual Network data fields in the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    int update(SqliteDB * db);
    
    /**
     * Deletes a VNW from the database and all its associated information:
     *   - VNW template
     *   - given leases
     *   @param db pointer to the db
     *   @return 0 on success
     */
    int drop(SqliteDB * db)
    { 
    	int rc;
    	
    	rc =  vnw_template.drop(db);
    	
    	if (rc != 0)
    	{
            return rc;
    	}
    	    
    	
        rc = leases.drop(db);
    	
    	return rc;
    }
};

#endif /*VIRTUAL_NETWORK_H_*/
