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

#ifndef LEASES_H_
#define LEASES_H_

#include <sqlite3.h>
#include "ObjectSQL.h"

#include <sstream>

using namespace std;

extern "C" int leases_select_cb (
        void *                  _lease,
        int                     num,
        char **                 values,
        char **                 names);

class Leases : public ObjectSQL
{
protected:
    /**
     *  The Lease class, it represents a pair of IP and MAC assigned to
     *  a Virtual Machine
     */
    class Lease
    {
    public:        
        Lease(
            string&	_ip,
            string&	_mac,
            int    	_vid,
            bool    _used=true);

        ~Lease(){};

        /**
         * Converts this lease's IP and MAC to string
         * @param ip ip of the lease in string
         * @param mac mac of the lease in string
         */

        void to_string(string& _ip,
                       string& _mac);

    private:
    	
    	enum MACIndex
    	{
    		SUFFIX	= 0,
    		PREFIX  = 1
    	};
    	
        unsigned int    ip;
        
        unsigned int	mac[2];
        
        int             vid;
        
        bool            used;

        /**
         * Conversion from string IP to unsigned int IP
         * @return 0 if success
         */
        static int ip_to_number(const string& ip, unsigned int& i_ip);

        /**
         * Conversion from unsigned int IP to string IP
         * @return 0 if success
         */
        static void ip_to_string(const unsigned int i_ip, string& ip);

        /**
         * Conversion from string IP to unsigned int IP
         * @return 0 if success
         */
        static int mac_to_number(const string& mac, unsigned int i_mac[]);
        
        /**
         * Conversion from string IP to unsigned int IP
         * @return 0 if success
         */        
        static void mac_to_string(const unsigned int i_mac[], string& mac);
    }
    
    // -------------------------------------------------------------------------
    // Leases fields
    // -------------------------------------------------------------------------
    
    /**
     * Leases indentifier. Connects it to a Virtual Network
     */
    int             oid;

    /**
     * Hash of leases, indexed by lease.ip
     */
    map<unsigned int, Lease *> leases;

    /**
     * Number of possible leases (free + asigned)
     */
    int             size;

    // ----------------------------------------
    // DataBase implementation variables
    // ----------------------------------------
    enum ColNames
    {
        OID             = 0,
        IP              = 1,
        MAC             = 2,
        VID             = 3,
        USED			= 4,
        LIMIT           = 5
    };

    static const char * table;

    static const char * db_names;

    static const char * db_bootstrap;

    // ----------------------------------------
    // Leases methods
    // ----------------------------------------
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
     * @param ip ip of the lease to be deleted
     * @return 0 if success
     */
    int del(string ip);

    /**
     * Check if the passed ip corresponds with a given lease
     * @param ip of the lease to be checked
     * @return 0 if success
     */
    bool check(string ip);

    /**
     * Returns an unused lease, which becomes used
     * @param vid identifier of the VM getting this lease
     * @param ip ip of the returned lease
     * @param mac mac of  the returned lease
     * @return
     */
    virtual int getLease(int          vid,
                         string&      ip,
                         string&	  mac) = 0;

private:

    friend int leases_select_cb (
        void *                  _history,
        int                     num,
        char **                 values,
        char **                 names);

    /**
     *  Writes the leases in the DB
     *    @param db pointer to the database.
     *    @return 0 on success.
     */
    int insert(SqliteDB * db);

    /**
     *  Reads the leases from the DB
     *    @param db pointer to the database.
     *    @return 0 on success.
     */
    int select(SqliteDB * db);

    /**
     *  Removes the leases from the DB
     *    @param db pointer to the database.
     *    @return 0 on success.
     */
    int drop(SqliteDB * db);

    /**
     *  Updates the leases record
     *    @param db pointer to the database.
     *    @return 0 on success.
     */
    int update(SqliteDB * db)
    {}
    ;

    /**
        *  Gets the value of a column in the pool for a given object
        *    @param db pointer to Database
        *    @param column to be selected
        *    @param where condition to select the column
        *    @param value of the column
        *    @return 0 on success
        */
    int select_column(
        SqliteDB *      db,
        const string&   column,
        const string&   where,
        string *        value)
    {
        return ObjectSQL::select_column(db,table,column,where,value);
    }

    /**
     *  Sets the value of a column in the pool for a given object
     *    @param db pointer to Database
     *    @param column to be selected
     *    @param where condition to select the column
     *    @param value of the column
     *    @return 0 on success
     */
    int update_column(
        SqliteDB *      db,
        const string&   column,
        const string&   where,
        const string&   value)
    {
        return ObjectSQL::update_column(db,table,column,where,value);
    }


    /**
     *  Function to unmarshall a leases object
     *    @param num the number of columns read from the DB
     *    @para names the column names
     *    @para vaues the column values
     *    @return 0 on success
     */
    int unmarshall(int num, char **names, char ** values);
public:

    // *************************************************************************
    // Constructor
    // *************************************************************************
    Leases(int _oid,
           int _size):
            oid(_oid),
            size(_size);

    ~Leases();
};

class RangedLeases : public Leases
{
private:
    unsigned int network_address,
    unsigned int network_mask;

public:

    // *************************************************************************
    // Constructor
    // *************************************************************************
    RangedLeases(int    _oid,
                 int    _size,
                 string _network_address,
                 string _network_mask);

    ~RangedLeases()
    {}
    ;

    /**
      * Returns an unused lease, which becomes used
      * @param vid identifier of the VM getting this lease
      * @param ip ip of the returned lease
      * @param mac mac of  the returned lease
      * @return
      */
    int getLease(int       	vid,
                 string&  	ip,
                 string& 	mac);
};

class FixedLeases : public Leases
{
private:

public:

    // *************************************************************************
    // Constructor
    // *************************************************************************
    FixedLeases(int    _oid,
                int    _size,
                string list_of_ips_and_macs);

    ~FixedLeases()
    {}
    ;

    /**
      * Returns an unused lease, which becomes used
      * @param vid identifier of the VM getting this lease
      * @param ip ip of the returned lease
      * @param mac mac of  the returned lease
      * @return
      */
    int getLease(int       vid,
                 string&  	ip,
                 string& 	mac);
};

#endif /*LEASES_H_*/
