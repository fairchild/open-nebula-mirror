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

#include "Nebula.h"
#include "VirtualNetworkPool.h"
#include <sstream>

int VirtualNetworkPool::allocate (
    int            uid,
    const  string& stemplate,
    int *          oid)
{
    VirtualNetwork *              vn;
    char *                        error_msg;
    int                           rc;
    ostringstream oss;            
                                  
    string                        str_mac_prefix;    
    unsigned int                  mac_prefix;
                                  
    string                        network_address;
    string                        name;
    string                        bridge;
    
    string                        str_type;                               
    
    string                        str_size;
    unsigned int                  size;
    
    // Build a new Virtual Network object
    vn = new VirtualNetwork;
    
    vn->uid = uid;
    
    rc = vn->vn_template.parse(stemplate,&error_msg);
    
    if ( rc != 0 )
    {
        oss << error_msg;        
        Nebula::log("VNM", Log::ERROR, oss);
        free(error_msg);

        return -2;
    }
   
    // Information about the VN needs to be extracted from the template
    // TODO maybe not all in the template??? cross it with NebulaTemplate???
    vn->get_template_attribute("TYPE",str_type);
    
    if ( str_type == "RANGED")
    {
        vn->type = VirtualNetwork::RANGED;
    }
    else
    {
        vn->type = VirtualNetwork::FIXED;
    }

    vn->get_template_attribute("NAME",name);
    vn->name = name;
    
    vn->get_template_attribute("BRIDGE",bridge);
    vn->bridge = bridge;
    
    vn->get_template_attribute("SIZE",str_size);
    
    if(!str_size.empty())
    {
        size = atoi(str_size.c_str());
    }
    
    // Insert the VN in the pool so we have a valid OID

    *oid = PoolSQL::allocate(vn);
    
    vn = dynamic_cast<VirtualNetwork *>(PoolSQL::get(*oid,true));
         
    vn->get_template_attribute("MAC_PREFIX",str_mac_prefix); 
    
    if(str_mac_prefix.empty())
    {
        // TODO assign global nebula mac_prefix
        str_mac_prefix="FF:FF";
    }
    
       
    Leases::Lease::mac_to_number(str_mac_prefix,&mac_prefix);

     //Get the leases
     if (vn->type == VirtualNetwork::RANGED)
     {
         // retrieve specific information from template
         vn->get_template_attribute("NET_ADDRESS",network_address);
         
         // If size wasn't defined, we need to calculate it    
         if ( str_size.empty() )
         {
             string net_class;
             vn->get_template_attribute("NET_CLASS",net_class);
             
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
        
         vn->leases = new RangedLeases::RangedLeases(db,
                                                     *oid,
                                                     size,
                                                     mac_prefix,
                                                     network_address);
     }
     else if(vn->type == VirtualNetwork::FIXED)
          {
              // retrieve specific information from template

              vector<const Attribute *>   vector_leases;
              vn->get_template_attribute("LEASES",vector_leases);  

              vn->leases = new FixedLeases::FixedLeases(db,
                                                        *oid,
                                                        mac_prefix,
                                                        vector_leases);
          }
          else
          {
              oss << "Wrong type of Virtual Network: " << vn->type << endl;
              Nebula::log("VNM", Log::ERROR, oss);
              
              return -3;
          }
          
    vn->unlock();

    return 0;
}


