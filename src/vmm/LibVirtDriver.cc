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

#include "LibVirtDriver.h"

#include "Nebula.h"
#include <sstream>
#include <fstream>


int LibVirtDriver::deployment_description(
    const VirtualMachine *  vm,
    const string&           file_name) const
{
    
     ofstream                    file;

     int                         num;
     vector<const Attribute *>   attrs;

     string  cpu;
     string  memory;
     
     int     memory_in_kb = 0;

     string  kernel  = "";
     string  initrd  = "";
     string  boot    = "";
     string  root    = "";

     const VectorAttribute *	disk;

     string  type    = "";
     string  source  = "";
     string  target  = "";
     string  bus     = "";
     string  ro      = "";
     bool    readonly;

     const VectorAttribute * nic;

     string  mac     = "";
     string  bridge  = "";
     string  script  = "";

     const VectorAttribute * graphics;
     
     string  listen  = "";
     string  port    = "";
     string  passwd  = "";
     
     const VectorAttribute * input;
     
     const VectorAttribute * features;
     
     string     pae  = "";
     string     acpi = "";

     const VectorAttribute * raw;
     string data;

 	// ------------------------------------------------------------------------

     file.open(file_name.c_str(), ios::out);

     if (file.fail() == true)
     {
     	goto error_file;
     }

     // ------------------------------------------------------------------------
     // Starting XML document
     // ------------------------------------------------------------------------     
     
     file << "<domain type='" << emulator << "'>" << endl;
     
     // ------------------------------------------------------------------------
     // Domain name
     // ------------------------------------------------------------------------  
     
     file << "\t<name>one-" << vm->get_oid() << "</name>" << endl;

     // ------------------------------------------------------------------------
     // CPU 
     // ------------------------------------------------------------------------       


     vm->get_template_attribute("CPU", cpu);

     if(cpu.empty())
     {
         get_default("CPU",cpu);
     }
     
     if (!cpu.empty())
     {
     	//file << "\t<vcpu>" << cpu << "</vcpu>" << endl;
     	// TODO decide about the vpcu value 
     }
     else
     {
     	goto error_cpu;
     }
     
     // ------------------------------------------------------------------------
     // Memory 
     // ------------------------------------------------------------------------

     vm->get_template_attribute("MEMORY",memory);

     if (memory.empty())
     {
    	get_default("MEMORY",memory);
     }

     if (!memory.empty())
     {
        memory_in_kb = atoi(memory.c_str()) * 1024;
         
     	file << "\t<memory>" << memory_in_kb << "</memory>" << endl;
     }
     else
     {
     	goto error_memory;
     }

     // ------------------------------------------------------------------------
     //  OS and boot options
     // ------------------------------------------------------------------------
     
     file << "\t<os>" << endl;
     
     if (emulator == "kvm")
     {
         file << "\t\t<type>hvm</type>" << endl; 
     }

     num = vm->get_template_attribute("OS",attrs);
     
     // Get values & defaults

     if ( num >= 0 ) 
     {
     	const VectorAttribute *	os;

     	os = static_cast<const VectorAttribute *>(attrs[0]);

     	kernel     = os->vector_value("KERNEL");
     	initrd     = os->vector_value("INITRD");
     	boot       = os->vector_value("BOOT");
     	root       = os->vector_value("ROOT");
     }

     if ( kernel.empty() )
     {
     	get_default("OS","KERNEL",kernel);
     }

     if ( initrd.empty() )
     {
     	get_default("OS","INITRD",initrd);
     }

     if ( boot.empty() )
     {
     	get_default("OS","BOOT",boot);
     	
     	if ( boot.empty() )
        {
        	goto error_boot;
        }
     }
     
     if ( root.empty() )
     {
     	get_default("OS","ROOT",root);
     }
     
     // Start writing to the file with the info we got

     if ( !kernel.empty() )
     {
     	file << "\t\t<kernel>" << kernel << "</kernel>" << endl;
     }
     
     if ( !initrd.empty() )
     {
     	file << "\t\t<initrd>" << initrd << "</initrd>" << endl;
     }
     
     file << "\t\t<boot dev='" << boot << "'/>" << endl;
     
     if ( !root.empty() )
     {
     	file << "\t\t<cmdline>root=/dev/" << root << " ro</cmdline>" << endl;	
     }     	  
    
     
     file << "\t</os>" << endl;

     attrs.clear();

     // ------------------------------------------------------------------------
     // Disks
     // ------------------------------------------------------------------------      
     
     file << "\t<devices>" << endl; 
     
     if (emulator == "kvm")
     {
         file << "\t\t<emulator>/usr/bin/kvm</emulator>" << endl; 
     }

     num = vm->get_template_attribute("DISK",attrs);

     for (int i=0; i < num ;i++,source="",target="",ro="")
     {
         disk = static_cast<const VectorAttribute *>(attrs[i]);

         type   = disk->vector_value("TYPE");
         source = disk->vector_value("SOURCE");
         target = disk->vector_value("TARGET");
         ro     = disk->vector_value("READONLY");
         bus    = disk->vector_value("BUS");

         if ( source.empty() | target.empty())
         {
         	goto error_disk;
         }

         readonly = false;

         if ( !ro.empty() )
         {
         	transform(ro.begin(),ro.end(),ro.begin(),(int(*)(int))toupper);

         	if ( ro == "YES" )
         	{
         		readonly = true;
         	}
         }

         if ( type.empty() )
         {
        	 type = "disk";
         }
         
         file << "\t\t<disk type='file' device='" << type << "'>" << endl;
         file << "\t\t\t<source file='" << source << "'/>" << endl;
                  
         file << "\t\t\t<target dev='" << target << "'"; 
         
         if (!bus.empty())
         { 
             file << " bus='" << bus << "'/>" << endl; 
         }
         else
         {
        	 file << "/>" << endl;
         }
         
         if (readonly)
         {
             file << "\t\t\t<readonly/>" << endl; 
         }
         
         file << "\t\t</disk>" << endl;
     }

     attrs.clear();

     // ------------------------------------------------------------------------
     // Network interfaces
     // ------------------------------------------------------------------------

     num = vm->get_template_attribute("NIC",attrs);

     for(int i=0; i<num;i++,mac="",bridge="",target="",script="")
     {
         nic = static_cast<const VectorAttribute *>(attrs[i]);

         bridge = nic->vector_value("BRIDGE");

         if ( bridge.empty() )
         {
           file << "\t\t<interface type='ethernet'>" << endl;
         }
         else
         {
           file << "\t\t<interface type='bridge'>" << endl;
           file << "\t\t\t<source bridge='" << bridge << "'/>" << endl;
         }    

         mac = nic->vector_value("MAC");

         if( !mac.empty() )
         {
         	file << "\t\t\t<mac address='" << mac << "'/>" << endl;
         }
         
         target = nic->vector_value("TARGET");
         
         if( !target.empty() )
         {
             file << "\t\t\t<target dev='" << target << "'/>" << endl;
         }
         
         script = nic->vector_value("SCRIPT");
     
         if( !script.empty() )
         {
             file << "\t\t\t<script path='" << script << "'/>" << endl;
         }
         
         file << "\t\t</interface>" << endl;

     }

     attrs.clear();

     // ------------------------------------------------------------------------
     // Graphics
     // ------------------------------------------------------------------------

     if ( vm->get_template_attribute("GRAPHICS",attrs) > 0 )
     {
     	graphics = static_cast<const VectorAttribute *>(attrs[0]);

     	type   = graphics->vector_value("TYPE");
     	listen = graphics->vector_value("LISTEN");
     	port   = graphics->vector_value("PORT");
     	passwd = graphics->vector_value("PASSWD");

     	if ( type == "vnc" || type == "VNC" )
     	{
            file << "\t\t<graphics type='vnc'";

     		if ( !listen.empty() )
     		{
                file << " listen='" << listen << "'";
     		}

     		if ( !port.empty() )
     		{
                file << " port='" << port << "'";
     		}

     		if ( !passwd.empty() )
     		{
     			file << " password='" << passwd << "'";
     		}

     		file << "/>" << endl;    		 	
     	}
     	else
     	{
     		vm->log("VMM", Log::WARNING, "Not supported graphics type, ignored.");
     	}
     }

     attrs.clear();
     
     // ------------------------------------------------------------------------
     // Input
     // ------------------------------------------------------------------------

     if ( vm->get_template_attribute("INPUT",attrs) > 0 )
     {
     	input = static_cast<const VectorAttribute *>(attrs[0]);

     	type   = graphics->vector_value("TYPE");
     	bus    = graphics->vector_value("BUS");
     	
 		if ( !type.empty() )
 		{
            file << "\t\t<input type='" << type << "'";
            
            if ( !bus.empty() )
     		{
                file << " bus='" << bus << "'";
     		}
 		}	  
 		
        file << "/>" << endl; 
     }  	  	
     	
     attrs.clear();
     
     file << "\t</devices>" << endl;
     
     // ------------------------------------------------------------------------
     // Features
     // ------------------------------------------------------------------------

     num = vm->get_template_attribute("FEATURES",attrs);

     if ( num > 0 ) 
     {         
     	features = static_cast<const VectorAttribute *>(attrs[0]);

     	pae      = features->vector_value("PAE");
     	acpi     = features->vector_value("ACPI");
     	
        file << "\t<features>" << endl;
     	
     	if ( pae == "yes" )
 		{
 			file << "\t\t<pae/>" << endl;
 		}
 		
 		if ( acpi == "no" )
 		{
 			file << "\t\t<acpi/>" << endl;
 		}
 
         file << "\t</features>" << endl;
     }
     
     attrs.clear();

     // ------------------------------------------------------------------------
     // Raw XEN attributes
     // ------------------------------------------------------------------------

     num = vm->get_template_attribute("RAW",attrs);

     for(int i=0; i<num;i++)
     {
     	raw = static_cast<const VectorAttribute *>(attrs[i]);

     	type = raw->vector_value("TYPE");

     	transform(type.begin(),type.end(),type.begin(),(int(*)(int))toupper);

     	if ( type == "KVM" )
     	{   
     		data = raw->vector_value("DATA");
     		file << "\t" << data << endl;
     	}
     }
     
     file << "</domain>" << endl;

     file.close();

     return 0;

error_file:
	vm->log("VMM", Log::ERROR, "Could not open KVM deployment file.");
	return -1;
	
error_cpu:
	vm->log("VMM", Log::ERROR, "No CPU defined and no default provided.");
    file.close();	
    return -1;

error_memory:
	vm->log("VMM", Log::ERROR, "No MEMORY defined and no default provided.");
	file.close();	
	return -1;

error_boot:
	vm->log("VMM", Log::ERROR, "No BOOT device defined and no default provided.");
	file.close();	
	return -1;

error_disk:
	vm->log("VMM", Log::ERROR, "Wrong source or target value in DISK.");
	file.close();	
	return -1;
}