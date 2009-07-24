/* -------------------------------------------------------------------------- */
/* Copyright 2002-2009, Distributed Systems Architecture Group, Universidad   */
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

#include "InformationManagerDriver.h"
#include "Nebula.h"
#include <sstream>


/* ************************************************************************** */
/* Driver ASCII Protocol Implementation                                       */
/* ************************************************************************** */

void InformationManagerDriver::monitor (
    const int       oid,
    const string&   host) const
{
    ostringstream os;

    os << "MONITOR " << oid << " " << host << endl;

    write(os);
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void InformationManagerDriver::protocol(
    string&     message)
{       
    istringstream   is(message);   
    //stores the action name
    string          action;
    //stores the action result
    string          result;
    //stores the action id of the asociated HOSR
    int             id;

    ostringstream   ess;
    string          hinfo;
    Host *          host;
    
    // Parse the driver message

    if ( is.good() )
    {
        is >> action >> ws;
    }
    else
    {
        goto error_parse;
    }

    if ( is.good() )
    {
        is >> result >> ws;
    }
    else
    {
        goto error_parse;
    }

    if ( is.good() )
    {
        is >> id >> ws;
    }
    else
    {
        goto error_parse;
    }

    // -----------------------
    // Protocol implementation
    // -----------------------
    
    if ( action == "MONITOR" )
    {
        host = hpool->get(id,true);
        
        if ( host == 0 )
        {
            goto error_host;
        }
        
        if (result == "SUCCESS")
        {            
            size_t  pos;            
            int     rc;
            
            ostringstream oss;
         
            getline (is,hinfo);
               
            for (pos=hinfo.find(',');pos!=string::npos;pos=hinfo.find(','))
            {
                hinfo.replace(pos,1,"\n");
            }

            hinfo += "\n";
            
            oss << "Host " << id << " successfully monitored."; //, info: "<< hinfo;
            Nebula::log("InM",Log::DEBUG,oss);
                                  
            rc = host->update_info(hinfo);
            
            if (rc != 0)
            {
                goto error_parse_info;
            }                
        }
        else
        {
        	goto error_driver_info;        	            
        }
        
        host->touch(true);
        
        hpool->update(host);

        host->unlock();        
    }
    else if (action == "LOG")
    {
        string info;
        
        getline(is,info);
        Nebula::log("InM",Log::INFO,info.c_str());
    }
    
    return;

error_driver_info:
	ess << "Error monitoring host " << id << " : " << is.str();
	Nebula::log("InM", Log::ERROR, ess);

	goto  error_common_info;
	
error_parse_info:
    ess << "Error parsing host information: " << hinfo;
    Nebula::log("InM",Log::ERROR,ess); 
    
error_common_info:

    host->touch(false);
    
    hpool->update(host);
    
    host->unlock();
    
    return;

error_host:
    ess << "Could not get host " << id;
    Nebula::log("InM",Log::ERROR,ess); 
    
    return;
    
error_parse:

    ess << "Error while parsing driver message: " << message;
    Nebula::log("InM",Log::ERROR,ess); 

    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void InformationManagerDriver::recover()
{
    Nebula::log("InM", Log::ERROR, "Information driver crashed, recovering...");       
}

