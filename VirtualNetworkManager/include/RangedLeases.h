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

#ifndef RANGED_LEASES_H_
#define RANGED_LEASES_H_

#include "Leases.h"

using namespace std;

class RangedLeases : public Leases
{
private:
    unsigned int network_address;
    unsigned int network_mask;

public:

    // *************************************************************************
    // Constructor
    // *************************************************************************
    RangedLeases(SqliteDB * db,
                 int        _oid,
                 int        _size,
                 string     _network_address,
                 string     _network_mask);

    ~RangedLeases(){};

    /**
      * Returns an unused lease, which becomes used
      * @param vid identifier of the VM getting this lease
      * @param ip ip of the returned lease
      * @param mac mac of  the returned lease
      * @return
      */
    int getLease(int      vid,
                 string&  ip,
                 string&  mac){return 0;};
                 
    /**
     * Add a lease 
     * @param ip ip of the lease
     * @param mac mac of the lease
     * @param vid identifier of the VM getting this lease
     * @return 0 if success
     */
     int add(string       ip,
             string       mac,
             int          vid,
             bool         used=true);
                    

    /**
     * Remove a lease 
     * @param db pointer to DB
     * @param ip ip of the lease to be deleted
     * @return 0 if success
     */
     int del(string ip);
     
     int insert(SqliteDB * db){return 0;};
                              
};

#endif /*RANGED_LEASES_H_*/
