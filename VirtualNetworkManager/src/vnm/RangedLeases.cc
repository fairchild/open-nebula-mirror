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


#include "RangedLeases.h"


/* ************************************************************************** */
/* Ranged Leases class                                                        */
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
   
   

