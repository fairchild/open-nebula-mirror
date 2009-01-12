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

#include "TransferManagerDriver.h"
#include "Nebula.h"
#include "LifeCycleManager.h"
#include <sstream>

TransferManagerDriver::TransferManagerDriver(
	int                         userid,
    const map<string,string>&   attrs,
    bool                        sudo,
    VirtualMachinePool *        pool): 
    	Mad(userid,attrs,sudo),driver_conf(true),vmpool(pool)
{
    map<string,string>::const_iterator	it;
    char *			error_msg = 0;
    const char *	cfile;
    string			file;
    int				rc;
                    
    it = attrs.find("DEFAULT");
            
    if ( it != attrs.end() )
    {        	        	
       	if (it->second[0] != '/') //Look in ONE_LOCATION/etc or in "/etc/one"
        {
      		Nebula& nd = Nebula::instance();
                                    
            file  = nd.get_defaults_location() + it->second;
            cfile = file.c_str();
        }
        else //Absolute Path
        {
          	cfile = it->second.c_str();        
        } 
                
        rc = driver_conf.parse(cfile, &error_msg);
        
        if ( rc != 0 )
        {
        	ostringstream   oss;
        	
        	if ( error_msg != 0 )
        	{            	
        		oss << "Error loading driver configuration file " << cfile << 
           		" : " << error_msg;
        		
        		free(error_msg);
        	}
        	else
        	{            	
        		oss << "Error loading driver configuration file " << cfile;
        	}
        	
           	Nebula::log("TM", Log::ERROR, oss);            		           	
        }
    }
}

/* ************************************************************************** */
/* Driver ASCII Protocol Implementation                                       */
/* ************************************************************************** */

void TransferManagerDriver::transfer (
		const int oid, 
		const string& xfr_file) const
{
    ostringstream os;

    os << "TRANSFER " << oid << " " << xfr_file << endl;

    write(os);
};


/* ************************************************************************** */
/* MAD Interface                                                              */
/* ************************************************************************** */

void TransferManagerDriver::protocol(
    string&     message)
{
    istringstream           is(message);
    ostringstream           os;

    string                  action;
    string                  result;

    int                     id;
    VirtualMachine *        vm;

    os << "Message received: " << message;
    Nebula::log("TM", Log::DEBUG, os);

    // Parse the driver message
    if ( is.good() )
        is >> action >> ws;
    else
        return;

    if ( is.good() )
        is >> result >> ws;
    else
        return;

    if ( is.good() )
        is >> id >> ws;
    else
        return;

    // Get the VM from the pool
    vm = vmpool->get(id,true);

    if ( vm == 0 )
    {
        return;
    }

    // Driver Actions
    if (action == "TRANSFER")
    {
        Nebula              &ne = Nebula::instance();
        LifeCycleManager *  lcm = ne.get_lcm();
        
        LifeCycleManager::Actions lcm_action;
    	
        if (result == "SUCCESS")
        {
        	switch (vm->get_lcm_state())
        	{
        		case VirtualMachine::PROLOG:
        		case VirtualMachine::PROLOG_MIGRATE:
        		case VirtualMachine::PROLOG_RESUME:
       	        	lcm_action = LifeCycleManager::PROLOG_SUCCESS; 
        			break;
        			
        		case VirtualMachine::EPILOG:
        		case VirtualMachine::EPILOG_STOP:
       	        	lcm_action = LifeCycleManager::EPILOG_SUCCESS; 
        			break;
        			
        		default:
        			goto error_state;
        	}
        }
        else
        {        	
            string info;
            
            getline(is,info);
            
            os.str("");
            os << "Error excuting image transfer script: " << info;
            
            vm->log("TM",Log::ERROR,os);
            
        	switch (vm->get_lcm_state())
        	{
        		case VirtualMachine::PROLOG:
        		case VirtualMachine::PROLOG_MIGRATE:
        		case VirtualMachine::PROLOG_RESUME:
       	        	lcm_action = LifeCycleManager::PROLOG_FAILURE; 
        			break;
        			
        		case VirtualMachine::EPILOG:
        		case VirtualMachine::EPILOG_STOP:
       	        	lcm_action = LifeCycleManager::EPILOG_FAILURE; 
        			break;
        			
        		default:
        			goto error_state;
        	}
        }
        
        lcm->trigger(lcm_action, id);
    }
    else if (action == "LOG")
    {
        string info;
        
        getline(is,info);
        vm->log("TM",Log::INFO,info.c_str());
    }

    vm->unlock();
    
    return;
    
error_state:
	os.str("");
	os << "Wrong state in TM answer for VM " << id;

	vm->log("TM",Log::ERROR,os);

	vm->unlock();
	
	return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void TransferManagerDriver::recover()
{
    Nebula::log("TM",Log::INFO,"Recovering TM drivers");
}

