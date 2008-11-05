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
#include "Nebula.h"


/* ************************************************************************** */
/* Virtual Network :: Constructor/Destructor                                  */
/* ************************************************************************** */

VirtualNetwork::VirtualNetwork(int  id):                 
                PoolObjectSQL(id),
                name(""),
                uid(-1),
                bridge(""),
                type(UNINITIALIZED),
                leases(0)
{
}

VirtualNetwork::~VirtualNetwork()
{    
    if (leases != 0)
    {
        delete leases;
    }      
}

/* ************************************************************************** */
/* Virtual Network :: Database Access Functions                               */
/* ************************************************************************** */

const char * VirtualNetwork::table               = "network_pool";

const char * VirtualNetwork::db_names            = "(oid,uid,name,type,bridge)";

const char * VirtualNetwork::db_bootstrap        = "CREATE TABLE network_pool ("
        "oid INTEGER PRIMARY KEY,uid INTEGER, name TEXT,type INTEGER, bridge TEXT)";

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int VirtualNetwork::unmarshall(int num, char **names, char ** values)
{
    if     ((values[OID]    == 0)  ||
            (values[UID]    == 0)  ||
            (values[NAME]   == 0)  ||
            (values[TYPE]   == 0)  ||
            (values[BRIDGE] == 0)  ||                 
            (num            != LIMIT ))
    {
        return -1;
    }

    oid    = atoi(values[OID]);
    uid    = atoi(values[UID]);
    
    name   = values[NAME];
    
    type   = (NetworkType)atoi(values[TYPE]);
    
    bridge = values[BRIDGE];
    
    // Virtual Network template ID is the Network ID    
    vn_template.id = oid;

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

extern "C" int vn_select_cb (
        void *                  _vn,
        int                     num,
        char **                 values,
        char **                 names)
{
    VirtualNetwork *    vn;

    vn = static_cast<VirtualNetwork *>(_vn);

    if (vn == 0)
    {
        return -1;
    }

    return vn->unmarshall(num,names,values);
};

/* -------------------------------------------------------------------------- */

int VirtualNetwork::select(SqliteDB * db)
{
    ostringstream   oss;
    ostringstream   ose;
    
    int             rc;
    int             boid;
    
    string          str_mac_prefix;    
    unsigned int    mac_prefix;
    string          network_address;
    
    string                        str_size;
    unsigned int                  size;
        
    oss << "SELECT * FROM " << table << " WHERE oid = " << oid;

    boid = oid;
    oid  = -1;

    rc = db->exec(oss,vn_select_cb,(void *) this);


    if ((rc != 0) || (oid != boid ))
    {
        goto error_id;
    }
    
    //Get the template
    rc = vn_template.select(db);

    if (rc != 0)
    {
        goto error_template;
    }
    
    get_template_attribute("SIZE",str_size);
    
    if(!str_size.empty())
    {
        size = atoi(str_size.c_str());
    }
    

    get_template_attribute("MAC_PREFIX",str_mac_prefix);    
    Leases::Lease::mac_to_number(str_mac_prefix,&mac_prefix);
    
    //Get the leases
    if (type == RANGED)
    {
        // retrieve specific information from template
        get_template_attribute("NET_ADDRESS",network_address);  
        
         // If size wasn't defined, we need to calculate it    
        if ( str_size.empty() )
        {
             string net_class;
             get_template_attribute("NET_CLASS",net_class);
             
             if ( net_class == "B" )
             {
                 size = 65534;	    
                 
             }
             else if ( net_class == "C" )
                  {
                      size = 254;
                  }
                  else
                  {
                      //TODO what would be the default?
                      size = 65534;
                  }
         }
        
        leases = new RangedLeases::RangedLeases(db,
                                                oid,
                                                size,
                                                mac_prefix,
                                                network_address);
    }
    else if(type == FIXED)
         {
             leases = new  FixedLeases(db,
                                       oid,
                                       mac_prefix);                                                                           
         }
         else
         {
             goto error_leases;
         }


    leases->select(db);
        
    return 0;


error_id:
    ose << "Error getting Virtual Network nid: " << oid;
    Nebula::log("VNM", Log::ERROR, ose); 
    return -1;

error_template:
    ose << "Can not get template for Virtual Network nid: " << oid;
    Nebula::log("VNM", Log::ERROR, ose); 
    return -1;
    
error_leases:
    ose << "Error getting Virtual Network leases nid: " << oid;
    Nebula::log("VNM", Log::ERROR, ose);   
    return -1;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int VirtualNetwork::insert(SqliteDB * db)
{
    ostringstream   ose;
    int             rc;
    
    if ( vn_template.id == -1 )
    {
        vn_template.id = oid;
    }

    // Insert the template first
    rc = vn_template.insert(db);

    if ( rc != 0 )
    {
        ose << "Can not insert in DB template for Virtual Network id " << oid; 
        Nebula::log("VNM", Log::ERROR, ose);
        return -1;
    }

    // Insert the Virtual Network
    rc = update(db);

    if ( rc != 0 )
    {
        ose << "Can not update Virtual Network id " << oid; 
        Nebula::log("VNM", Log::ERROR, ose);
        vn_template.drop(db);

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
        uid << "," <<
        "'" << name << "',"  <<
        type << "," <<
        "'" << bridge << "')";
        
    rc = db->exec(oss);

    return rc;
}


/* ************************************************************************** */
/* Virtual Network :: Misc                                                    */
/* ************************************************************************** */

ostream& operator<<(ostream& os, VirtualNetwork& vn)
{
    os << "NID               : " << vn.oid << endl;
    os << "UID               : " << vn.uid << endl;
    os << "Network Name      : " << vn.name << endl;
    os << "Type              : ";
    if ( vn.type==VirtualNetwork::RANGED )
    {
        os << "Ranged" << endl;
    }
    else
    {
       os << "Fixed" << endl;
    }
    os << "Size              : " << vn.get_size() << endl;
    os << "Bridge            : " << vn.bridge << endl;

    os << "Template" << endl << vn.vn_template << endl;
    
    return os;
};
