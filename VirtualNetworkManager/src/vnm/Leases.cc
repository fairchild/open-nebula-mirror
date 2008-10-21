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


#include "Leases.h"


/* ************************************************************************** */
/* Lease class                                                                */
/* ************************************************************************** */

Lease(
	string        	    _ip,
	string           	_mac,
	int               	_vid
    bool              	_used=true):
        	vid(_vid),
        	used(_used)
{
    
    ip_string_to_number(_ip,&ip);    
    
    // MAC from string to unsigned int [2]
    
    char * cstr = new char [_mac.size()+1];
    strcpy (cstr, _mac.c_str());
    
    *byte  = strtok(cstr, ".");
    
    int  mac_index = SUFFIX;
    int  index     = 0;

    while(byte!=NULL)
    {
        if (index++>1)  
        {
            mac_index = PREFIX;
        }
        mac[mac_index]   <<= 8;
        mac[mac_index]   += atoi((const char *)byte);
        byte             =  strtok(NULL, ".");
    }  
    
    delete[] cstr; 
       
}
    
    /* ************************************************************************** */
    /* Lease Public Methods                                                       */
    /* ************************************************************************** */

void Lease::to_string(string &_ip, 
                      string &_mac)
{
     int                      index=0;
     unsigned int             temp_byte;
     std::ostringstream       oss;

     ip_number_to_string(ip,_ip);
     
     // Calculate the MAC
     
     oss.str("")=;


     for (i=5;i>=0;i--)
     {         
         if ( i < 4 )
         {
             temp_byte =   mac[SUFFIX];
             temp_byte >>= i*8;
         }
         else
         {
             temp_byte  =   mac[PREFIX];
             temp_byte  >>= (i%4)*8;
         }
            
         temp_byte  &= 255;
         oss        << temp_byte;
         
         if(i!=0)
         {
             oss << ".";  
         } 
     }
    
     _mac = strdup(oss.str());

     return 0;
}

static int Lease::ip_number_to_string(unsigned int num_ip,
                                      string&      str_ip)
{
    int                      index;
    unsigned int             temp_byte;
    std::ostringstream       oss;

    // Convert the IP from unsigned int to string
   
    for (index=0;index<4;index++)
    {
        temp_byte   =   num_ip;
        temp_byte   >>= (24-index*8);
        temp_byte   &=  255;
        
        oss         <<  temp_byte;
        
        if(index!=3) 
        {
            oss << ".";
        } 
    }

    str_ip = strdup(oss.str());
    
    return 0;
}             

static int Lease::ip_string_to_number(string        str_ip,
                                      unsigned int& num_ip)
{
    // Convert IP from string to unsigned int

    char * cstr = new char [str_ip.size()+1];
    strcpy (cstr, str_ip.c_str());

    char *byte  = strtok(cstr, ".");

    while(byte!=NULL)
    {

      *num_ip   <<= 8;
      *num_ip   += atoi((const char *)byte);
      byte = strtok(NULL, ".");
    }
    
    delete[] cstr;
}                       

/* ************************************************************************** */
/* Leases class                                                               */
/* ************************************************************************** */

    /* ************************************************************************** */
    /* Leases :: Constructor                                                      */
    /* ************************************************************************** */
    
    ~Leases()
    {
        
        map<int, Lease *>::iterator  it;

        // Delete all the leases

        for(it=leases.begin();it!=leases.end();it++)
        {
            delete it->second;         
        }
    }

    /* ************************************************************************** */
    /* Leases :: Database Access Functions                                        */
    /* ************************************************************************** */

const char * Leases::table               = "leases";

const char * Leases::db_names            = "(oid,ip,mac,vid,used)";

const char * Leases::db_bootstrap        = "CREATE TABLE leases ("
        "oid INTEGER,ip TEXT, mac TEXT, vid INTEGER, used INTEGER,"
        "PRIMARY KEY(oid,ip))";

    /* -------------------------------------------------------------------------- */
    /* -------------------------------------------------------------------------- */

int Leases::unmarshall(int num, char **names, char ** values)
{
    if (      (values[OID] == 0) ||
              (values[IP]  == 0) ||
              (values[MAC] == 0) ||
              (values[VID] == 0) ||
              (values[USED]== 0) ||
              (num != LIMIT ))
      {
          return -1;
      }

      string ip   = values[IP];
      string mac  = values[MAC];
      int    vid  = atoi(values[VID]);
      bool   used = atoi(values[USED])==0?false:true;
      
      add(ip,mac,vid,used);

      return 0;
}

    /* -------------------------------------------------------------------------- */
    /* -------------------------------------------------------------------------- */

extern "C" int leases_select_cb (
        void *                  _leases,
        int                     num,
        char **                 values,
        char **                 names)
{
    Leases *    leases;

    leases = static_cast<Leases *>(_leases);

    if (leases == 0)
    {
        return -1;
    }

    return leases->unmarshall(num,names,values);
};

    /* -------------------------------------------------------------------------- */
    /* -------------------------------------------------------------------------- */

int Leases::select(SqliteDB * db)
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

    rc = db->exec(oss,leases_select_cb,(void *) this);


    if ((rc != 0) || (oid != boid ))
    {
        goto error_id;
    }


    return 0;


error_id:
    ose << "Error getting leases for network nid: " << oid;
    return -1;

}

    /* -------------------------------------------------------------------------- */
    /* -------------------------------------------------------------------------- */

int Leases::insert(SqliteDB * db)
{
    // Insert the Leases in the DB
    rc = update(db);

    return 0;
}

    /* -------------------------------------------------------------------------- */
    /* -------------------------------------------------------------------------- */

int Leases::update(SqliteDB * db)
{
    ostringstream   oss;
    int             rc;
   
    map<int, Lease *>::iterator  it;
    
    // Iterate all the leases
    
    for(it=leases.begin();it!=leases.end();it++)
    {
        oss << "INSERT OR REPLACE INTO " << table << " "<< db_names <<" VALUES ("<<
            oid << "," <<
            (it->second).ip << "," <<
            (it->second).mac << "," <<
            (it->second).vid << "," <<
            (it->second).used?1:0 << ")";
    }

    rc = db->exec(oss);

    return rc;
}

    /* ************************************************************************** */
    /* Leases :: Interface Methods                                                */
    /* ************************************************************************** */
    
    int Leases::add(string       ip,
                    string       mac,
                    int          vid,
                    bool         used=true)
    {
        leases.insert(make_pair(ip,new Lease(ip,mac,vid,used)));
        
        return 0;
    }
    
    int Leases::del(string ip)
    {
        leases.erase(ip);
        
        // Erase it from DB

        oss << "DELETE FROM " << table << " WHERE oid=" << oid << " AND ip=" << ip;

        return db->exec(oss);
        
        return 0;
    }
    
    bool Leases::check(string ip)
    {
        map<int,Leases *>::iterator it;
        
        
        leases.find(ip);
        
        if (it!=leases.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    /* ************************************************************************** */
    /* Leases :: Misc                                                             */
    /* ************************************************************************** */

    ostream& operator<<(ostream& os, VirtualNetwork& vnw)
    {
        os << "NID               : " << leases.oid << endl;
        
        os << "Leases:" << endl;
        
        map<int, Lease *>::iterator  it;

        // Iterate all the leases

        for(it=leases.begin();it!=leases.end();it++)
        {
            os <<  "IP:"   << (it->second).ip <<  
                  " MAC: " << (it->second).mac <<  
                  " VID: " << (it->second).vid <<  
                  " USED: " << (it->second).used?"Yes":"No";           
        }
    
        return os;
    };
    
    
/* ************************************************************************** */
/* Ranged Leases class                                                        */
/* ************************************************************************** */


    /* ************************************************************************** */
    /* Ranged Leases :: Constructor                                               */
    /* ************************************************************************** */
    
    RangedLeases(int    _oid,
                 int    _size,
                 string _network_address,
                 string _network_mask)
    {
        Leases(_oid,_size);
        
        Lease::ip_string_to_number(_network_address,&network_address);
        
        Lease::ip_string_to_number(_network_mask,&network_mask);
    }
    
/* ************************************************************************** */
/* Fixed Leases class                                                         */
/* ************************************************************************** */


    /* ************************************************************************** */
    /* Fixed Leases :: Constructor                                                */
    /* ************************************************************************** */
    
    FixedLeases(int    _oid,
				int    _size,
                string list_of_ips_and_macs);
    {
        Leases(_oid,_size);
        
        // TODO --> Convert from list_of_ips_and_macs to leases
    }


