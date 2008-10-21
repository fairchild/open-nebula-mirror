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

Lease::Lease(
        string&	_ip,
        string&	_mac,
        int    	_vid,
        bool    _used=true):
        	vid(_vid),
        	used(_used)
{
    if ( mac_to_number(_mac, mac) != 0 )
    {
    	runtime_error("Wrong MAC format");
    }
    else if ( ip_to_number(_ip, ip) != 0 )
    {
    	runtime_error("Wrong IP format");
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void Lease::to_string(string &_ip, 
                      string &_mac)
{
	mac_to_string(mac,_mac);
	
	ip_to_string(ip,_ip);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static int Lease::ip_to_number(const string& ip, unsigned int& i_ip)
{
    istringstream iss;    
    size_t        pos=0;
    int           count = 0;
  	unsigned int  tmp;
  	
  	string ip = _ip;
  	
  	while ( (pos = ip.find('.')) !=  string::npos )
  	{
  		ip.replace(pos,1," ");
	    count++;
  	}

	if (count != 3)
	{
	    return -1;
	}

	iss.str(ip);

	i_ip = 0;
	
	for (int i=0;i<4;i++)
	{
		iss >> dec >> tmp >> ws;
		
	    i_ip <<= 8;
	    i_ip += tmp;
	}
	
	return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static void Lease::ip_to_string(const unsigned int i_ip, string& ip)
{
    unsigned int    temp_byte;
    ostringstream	oss;

    // Convert the IP from unsigned int to string
   
    for (int index=0;index<4;index++)
    {
        temp_byte =   i_ip;
        temp_byte >>= (24-index*8);
        temp_byte &=  255;
        
        oss << temp_byte;
        
        if(index!=3) 
        {
            oss << ".";
        } 
    }

    ip = oss.str();
    
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static int Lease::mac_to_number(const string& _mac, unsigned int i_mac[])
{
    istringstream iss;    
    size_t        pos=0;
    int           count = 0;
  	unsigned int  tmp;
  	
  	string mac = _mac;
  	
  	while ( (pos = mac.find(':')) !=  string::npos )
  	{
  		mac.replace(pos,1," ");
	    count++;
  	}

	if (count != 5)
	{
	    return -1;
	}

	iss.str(mac);
	
	i_mac[PREFIX] = 0;
	i_mac[SUFFIX] = 0;
	
	iss >> hex >> i_mac[PREFIX] >> ws >> hex >> tmp >> ws;
	i_mac[PREFIX] <<= 8;
	i_mac[PREFIX] += tmp;

	for (int i=0;i<4;i++)
	{
		iss >> hex >> tmp >> ws;
		
	    i_mac[SUFFIX] <<= 8;
	    i_mac[SUFFIX] += tmp;
	}
	
	return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static void Lease::mac_to_string(const unsigned int i_mac[], string& mac)
{
	ostringstream	oss;
	unsigned int	temp_byte;
	
    oss.str("");

    for (int i=5;i>=0;i--)
    {         
        if ( i < 4 )
        {
            temp_byte =   mac[SUFFIX];
            temp_byte >>= i*8;
        }
        else
        {
            temp_byte  = mac[PREFIX];
            temp_byte  >>= (i%4)*8;
        }
           
        temp_byte  &= 255;
        
        oss.width(2);
        oss.fill('0');
        oss << hex << temp_byte;
        
        if(i!=0)
        {
            oss << ":";  
        } 
    }
    
    mac = oss.str();	
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


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


