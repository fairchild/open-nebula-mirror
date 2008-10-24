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

 Leases::Lease::Lease(
        string&	_ip,
        string&	_mac,
        int    	_vid,
        bool    _used):
        	vid(_vid),
        	used(_used)
{
    if ( mac_to_number(_mac, mac) )
    {
    	throw runtime_error("Wrong MAC format");
    }
    else if ( ip_to_number(_ip, ip) )
    {
    	throw runtime_error("Wrong IP format");
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void Leases::Lease::to_string(string &_ip, 
                              string &_mac,
                              string &_vid,
                              string &_used)
{   
    mac_to_string(mac, _mac);
    ip_to_string(ip, _ip); 
    _vid=vid;
    
    if( used )
    {
        used="Yes";
    }
    else
    {
        _used="No";
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int Leases::Lease::ip_to_number(const string& _ip, unsigned int& i_ip)
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

void Leases::Lease::ip_to_string(const unsigned int i_ip, string& ip)
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
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int Leases::Lease::mac_to_number(const string& _mac, unsigned int i_mac[])
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

void Leases::Lease::mac_to_string(const unsigned int i_mac[], string& mac)
{
	ostringstream	oss;
	unsigned int	temp_byte;
	
    oss.str("");

    for (int i=5;i>=0;i--)
    {         
        if ( i < 4 )
        {
            temp_byte =   i_mac[SUFFIX];
            temp_byte >>= i*8;
        }
        else
        {
            temp_byte  = i_mac[PREFIX];
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
    
    Leases::~Leases()
    {
        
        map<unsigned int, Lease *>::iterator  it;

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

const char * Leases::db_names            = "(oid,ip,mac_prefix,mac_suffix,vid,used)";

const char * Leases::db_bootstrap        = "CREATE TABLE leases ("
    "oid INTEGER,ip INTEGER, mac_prefix INTEGER,mac_suffix INTEGER," 
    "vid INTEGER, used INTEGER, PRIMARY KEY(oid,ip))";

  /* -------------------------------------------------------------------------- */
  /* -------------------------------------------------------------------------- */

int Leases::unmarshall(int num, char **names, char ** values)
{
   if (      (values[OID] == 0) ||
             (values[IP]  == 0) ||
             (values[MAC_PREFIX] == 0) ||
             (values[MAC_SUFFIX] == 0) ||
             (values[VID] == 0) ||
             (values[USED]== 0) ||
             (num != LIMIT ))
     {
         return -1;
     }
     
     unsigned int mac [2];

     unsigned int ip      = (unsigned int)atol(values[IP]);
     mac[Lease::PREFIX]   = (unsigned int)atol(values[MAC_PREFIX]);
     mac[Lease::SUFFIX]   = (unsigned int)atol(values[MAC_SUFFIX]);
     int          vid     = atoi(values[VID]);
     bool         used    = atoi(values[USED])?true:false;
     
     leases.insert(make_pair(ip,new Lease(ip,mac,vid,used)));

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
   oss << "Error getting leases for network nid: " << oid;
   return -1;

}


   /* -------------------------------------------------------------------------- */
   /* -------------------------------------------------------------------------- */

int Leases::update(SqliteDB * db)
{
// TODO return -1 and log error message
    return -1;
}

   /* ************************************************************************** */
   /* Leases :: Interface Methods                                                */
   /* ************************************************************************** */

   bool Leases::check(string ip)
   {
       map<unsigned int,Lease *>::iterator it;
       
       unsigned int _ip;
       
       Leases::Lease::ip_to_number(ip,_ip);


       it=leases.find(_ip);

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

   ostream& operator<<(ostream& os, Leases& _leases)
   {
       os << "NID               : " << _leases.oid << endl;

       os << "Leases:" << endl;

       map<unsigned int, Leases::Lease *>::iterator  it;

       // Iterate all the leases

       for(it=_leases.leases.begin();it!=_leases.leases.end();it++)
       {   
           string ip,mac,vid,used;
           
           (it->second)->to_string(ip,mac,vid,used);
           
           os << "IP:"   << ip <<
                 "MAC:"  << mac <<
                 "VID:"  << vid <<
                 "USED:" << used << endl;
       }

       return os;
   };


/* ************************************************************************** */
/* Ranged Leases class                                                        */
/* ************************************************************************** */


   /* ************************************************************************** */
   /* Ranged Leases :: Constructor                                               */
   /* ************************************************************************** */

   RangedLeases::RangedLeases(
                SqliteDB * db,
                int    _oid,
                int    _size,
                string _network_address,
                string _network_mask):
                   Leases(db,_oid,_size)
   {
       Leases::Lease::ip_to_number(_network_address,network_address);

       Leases::Lease::ip_to_number(_network_mask,network_mask);
   }
   
   /* ************************************************************************** */
   /* Ranged Leases :: Methods                                                   */
   /* ************************************************************************** */
   
   
      int RangedLeases::add( string       ip,
                             string       mac,
                             int          vid,
                             bool         used)
      {
          ostringstream    oss;
          
          unsigned int     _ip;
          unsigned int     _mac [2];
          
          int              _used;
          
          leases.insert(make_pair(_ip,new Lease(ip,mac,vid,used)));
         
          
          if (Lease::mac_to_number(mac,_mac)  )
          {
          	throw runtime_error("Wrong MAC format");
          }
          else if ( Lease::ip_to_number(ip,_ip) )
          {
          	throw runtime_error("Wrong IP format");
          }
          
          if (used)
          {
              _used = 1;
          }
          else
          {
              _used = 0;
          }

          oss << "INSERT OR REPLACE INTO " << table << " "<< db_names <<" VALUES ("<<
                      oid << "," <<
                      _ip << "," <<
                      _mac[Lease::PREFIX] << "," <<
                      _mac[Lease::SUFFIX] << "," <<
                      vid << "," <<
                      _used << ")";
                       
          return db->exec(oss);

          return 0;
      }
      
 
      int  RangedLeases::del(string ip)
      {
          unsigned int     _ip;
          ostringstream    oss;
          
           // Remove lease from leases map
           
          if ( Lease::ip_to_number(ip,_ip) )
          {
             throw runtime_error("Wrong IP format");
          }
          
          leases.erase(_ip);

          // Erase it from DB

          oss << "DELETE FROM " << table << " WHERE oid=" << oid << " AND ip=" << _ip;

          return db->exec(oss);
      }
   
   

/* ************************************************************************** */
/* Fixed Leases class                                                         */
/* ************************************************************************** */


   /* ************************************************************************** */
   /* Fixed Leases :: Constructor                                                */
   /* ************************************************************************** */

   FixedLeases::FixedLeases(SqliteDB * db,
               int     _oid,
			   int     _size,
               vector<const Attribute*> vector_leases):
                  Leases(db,_oid,_size)
   { 
       const VectorAttribute *	single_attr_lease;
       
       string _mac;
       string _ip;
       
       for (int i=0; i < (int)vector_leases.size() ;i++)
       {

           single_attr_lease = dynamic_cast<const VectorAttribute *>(vector_leases[i]);
      	
           if( single_attr_lease )
           {
               _ip      = single_attr_lease->vector_value("IP");
               _mac     = single_attr_lease->vector_value("MAC");
               
               add(_ip,_mac,-1,false); 
	       }
       }
   }
   
   /* ************************************************************************** */
   /* Fixed Leases :: Methods                                                    */
   /* ************************************************************************** */
   
       int FixedLeases::add(string       ip,
                            string       mac,
                            int          vid,
                            bool         used)
       {
           ostringstream    oss;
           unsigned int     _ip;
           unsigned int     _mac [2];
           int              _used;
           
           // Insert lease into leases map

           if (Leases::Lease::mac_to_number(mac,_mac)  )
           {
           	throw runtime_error("Wrong MAC format");
           }
           else if ( Leases::Lease::ip_to_number(ip,_ip) )
           {
           	throw runtime_error("Wrong IP format");
           }
           
           leases.insert(make_pair(_ip,new Lease(ip,mac,vid,used)));
           
           // Flip used flag to true
           
           if (used)
           {
               _used = 1;
           }
           else
           {
               _used = 0;
           }

           oss << "INSERT OR REPLACE INTO " << table << " "<< db_names <<" VALUES ("<<
                       oid << "," <<
                       _ip << "," <<
                       _mac[Lease::PREFIX] << "," <<
                       _mac[Lease::SUFFIX] << "," <<
                       vid << "," <<
                       _used << ")";

           return db->exec(oss);
       }

       int FixedLeases::del(string ip)
       {
           unsigned int     _ip;
           ostringstream    oss;
           
           // Remove lease from leases map
           
           if ( Leases::Lease::ip_to_number(ip,_ip) )
           {
               throw runtime_error("Wrong IP format");
           }
           
           leases.erase(_ip);

           // Flip used flag to false

           oss << "INSERT OR REPLACE INTO " << table << " (oid,ip,used) VALUES ("<<
                       oid << "," <<
                       _ip << "," <<
                       0   << ")";

           return db->exec(oss);

           return 0;
       }


