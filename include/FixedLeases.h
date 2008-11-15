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

#ifndef FIXED_LEASES_H_
#define FIXED_LEASES_H_

#include "Leases.h"

using namespace std;

/**
 *  The FixedLeases class represents a pool of fixed IP-MAC leases. A lease can 
 *  be either a IP (the MAC is then PREFIX:IP) or the IP and MAC addresses. The
 *  pool is read from a template file, each lease is in the form:
 *    LEASE = [ IP = "<the ip>", MAC = "<the mac>"]
 */ 
class FixedLeases : public Leases
{
public:
	/**
	 *  Create a FixedLeases from template
	 */
    FixedLeases(SqliteDB * 					db,
                int    						_oid,
                unsigned int				_mac_prefix,
                vector<const Attribute*>& 	vector_leases);
	/**
	 *  Create a plain FixedLeases, you can populate the lease pool using
	 *  select()
	 */
    FixedLeases(SqliteDB * 					db,
                int    						_oid,
                unsigned int				_mac_prefix):
                	Leases(db,_oid,0),
                	mac_prefix(_mac_prefix),
                	current(leases.begin()){};

    ~FixedLeases(){};

    /**
     * Returns an unused lease, which becomes used
     *   @param vid identifier of the VM getting this lease
     *   @param ip ip of the returned lease
     *   @param mac mac of  the returned lease
     *   @return 0 if success
     */
    int get(int vid, string&  ip, string&  mac);
    
    /**
     * Ask for a specific lease in the network
     *  @param vid identifier of the VM getting this lease
     *  @param ip ip of lease requested
     *  @param mac mac of the lease
     *  @return 0 if success
     */
    int set(int vid, const string&  ip, string&  mac);

    /**
     * Release an used lease, which becomes unused
     *   @param ip of the lease in use
     */
    void release(const string& ip)
    {
    	del(ip);
    }
    
    /**
     *  Loads the leases from the DB.
     */
    int select(SqliteDB * db)
    {
    	//Read the leases from the DB
    	int rc = Leases::select(db);
    	//Update the size
    	size = leases.size();
    	
    	return rc;
    }

private:
	
	/**
	 *  The default MAC prefix for the OpenNebula cluster
	 */
	unsigned int mac_prefix;
	
	/**
	 *  Current lease pointer
	 */
	map<unsigned int, Lease *>::iterator  current;
	
    /**
     * Add a lease, from the Lease interface
     * @param ip ip of the lease
     * @param mac mac of the lease
     * @param vid identifier of the VM getting this lease
     * @return 0 if success
     */
     int add(const string&	ip, const string& mac, int vid, bool used=true);
       
    /**
     * Remove a lease, from the Lease interface
     * @param db pointer to DB
     * @param ip ip of the lease to be deleted
     * @return 0 if success
     */
     int del(const string& ip);
};

#endif /*FIXED_LEASES_H_*/
