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


#include "VirtualNetwork.h"


/* ************************************************************************** */
/* Virtual Network :: Constructor/Destructor                                  */
/* ************************************************************************** */

VirtualNetwork::VirtualNetwork(int    id,
                               string _name,                            
                               string _bridge):
                PoolObjectSQL(id),
                name(_name),
                bidge(_bridge)
{
}

VirtualNetwork::~VirtualNetwork()
{
}

/* ************************************************************************** */
/* Virtual Network :: Database Access Functions                               */
/* ************************************************************************** */

const char * VirtualNetwork::table               = "network_pool";

const char * VirtualNetwork::db_names            = "(oid,name,bridge");

const char * VirtualNetwork::db_bootstrap        = "CREATE TABLE network_pool ("
        "oid INTEGER PRIMARY KEY,name TEXT, bridge TEXT)";
        
const char * VirtualNetwork::table_leases        = "leases";

const char * VirtualNetwork::db_names_leases     = "(oid,ip,vid)";

const char * VirtualNetwork::db_bootstrap_leases = "CREATE TABLE leases ("
        "oid INTEGER, ip INTEGER, vid INTEGER TEXT, PRIMARY KEY(oid,ip))";

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int VirtualNetwork::unmarshall(int num, char **names, char ** values)
{
 // TODO
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

extern "C" int vnw_select_cb (
        void *                  _vm,
        int                     num,
        char **                 values,
        char **                 names)
{
    VirtualNetwork *    vnw;

    vnw = static_cast<VirtualNetwork *>(_vnw);

    if (vnw == 0)
    {
        return -1;
    }

    return vnw->unmarshall(num,names,values);
};

/* -------------------------------------------------------------------------- */

int VirtualNetwork::select(SqliteDB * db)
{
    ostringstream   oss;
    ostringstream   ose;
    
    int             rc;
    int             boid;
    
    string          filename;
    const char *    nl;
    
    oss << "SELECT * FROM " << table << " WHERE oid = " << oid;

    boid = oid;
    oid  = -1;

    rc = db->exec(oss,vnw_select_cb,(void *) this);


    if ((rc != 0) || (oid != boid ))
    {
        goto error_id;
    }

    //Get the template
    rc = vnw_template.select(db);

    if (rc != 0)
    {
        goto error_template;
    }


    return 0;


error_id:
    ose << "Error getting Virtual Network nid: " << oid;
    return -1;

error_template:
    ose << "Can not get template for VM id: " << oid;
    return -1;

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int VirtualNetwork::insert(SqliteDB * db)
{
   // TODO  

    // Insert the template first
    rc = vnw_template.insert(db);

    if ( rc != 0 )
    {
        //log("ONE", Log::ERROR, "Can not insert template");
        return -1;
    }

    //Insert the Virtual Network
    rc = update(db);

    if ( rc != 0 )
    {
        //log("ONE", Log::ERROR, "Can not update vm");
        vnw_template.drop(db);

        return -1;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int VirtualNetwork::update(SqliteDB * db)
{
    ostringstream   oss;
    int             rc;

    oss << "INSERT OR REPLACE INTO " << table << " "<< db_names <<" VALUES ("<<
        oid << "," <<
        name << "," <<
        bridge << ")";

    rc = db->exec(oss);

    return rc;
}


/* ************************************************************************** */
/* Virtual Network :: Misc                                                    */
/* ************************************************************************** */

ostream& operator<<(ostream& os, VirtualNetwork& vnw)
{
    os << "NID               : " << vnw.oid << endl;
    os << "Network Name      : " << vnw.name << endl;
    os << "Bridge            : " << vnw.bridge << endl;

    os << "Template" << endl << vnw.vnw_template << endl;
    
    return os;
};
