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

    string  vcpu;
    string  memory;

    int     memory_in_kb = 0;

    string  kernel     = "";
    string  initrd     = "";
    string  boot       = "";
    string  root       = "";
    string  kernel_cmd = "";
    string  bootloader = "";

    const VectorAttribute * disk;
    const VectorAttribute * context;

    string  type       = "";
    string  target     = "";
    string  bus        = "";
    string  ro         = "";
    bool    readonly;

    const VectorAttribute * nic;

    string  mac        = "";
    string  bridge     = "";
    string  script     = "";
    string  model      = "";

    const VectorAttribute * graphics;

    string  listen     = "";
    string  port       = "";
    string  passwd     = "";
    string  keymap     = "";

    const VectorAttribute * input;

    const VectorAttribute * features;

    string     pae     = "";
    string     acpi    = "";

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

    vm->get_template_attribute("VCPU", vcpu);

    if(vcpu.empty())
    {
        get_default("VCPU", vcpu);
    }

    if (!vcpu.empty())
    {
        file << "\t<vcpu>" << vcpu << "</vcpu>" << endl;
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

    if ( num > 0 )
    {
        const VectorAttribute * os;

        os = dynamic_cast<const VectorAttribute *>(attrs[0]);

        if( os != 0 )
        {
            kernel     = os->vector_value("KERNEL");
            initrd     = os->vector_value("INITRD");
            boot       = os->vector_value("BOOT");
            root       = os->vector_value("ROOT");
            kernel_cmd = os->vector_value("KERNEL_CMD");
            bootloader = os->vector_value("BOOTLOADER");
        }
    }

    if ( kernel.empty() )
    {
        get_default("OS","KERNEL",kernel);
    }

    if ( initrd.empty() )
    {
        get_default("OS","INITRD",initrd);
    }

    if ( bootloader.empty() )
    {
        get_default("OS","BOOTLOADER",bootloader);
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

    if ( kernel_cmd.empty() )
    {
        get_default("OS","KERNEL_CMD",kernel_cmd);
    }

    // Start writing to the file with the info we got

    if ( !kernel.empty() )
    {
        file << "\t\t<kernel>" << kernel << "</kernel>" << endl;

        if ( !initrd.empty() )
        {
            file << "\t\t<initrd>" << initrd << "</initrd>" << endl;
        }

        if ( !root.empty() )
        {
            kernel_cmd = "root=/dev/" + root + " " + kernel_cmd;
        }

        if (!kernel_cmd.empty())
        {
            file << "\t\t<cmdline>" << kernel_cmd << "</cmdline>" << endl;
        }
    }
    else if ( !bootloader.empty() )
    {
        file << "\t\t<bootloader>" << bootloader << "</bootloader>" << endl;
    }


    file << "\t\t<boot dev='" << boot << "'/>" << endl;

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

    for (int i=0; i < num ;i++)
    {
        disk = dynamic_cast<const VectorAttribute *>(attrs[i]);

        if ( disk == 0 )
        {
         continue;
        }

        type   = disk->vector_value("TYPE");
        target = disk->vector_value("TARGET");
        ro     = disk->vector_value("READONLY");
        bus    = disk->vector_value("BUS");

        if (target.empty())
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

        if (type.empty() == false)
        {
            transform(type.begin(),type.end(),type.begin(),(int(*)(int))toupper);
        }

        if ( type == "BLOCK" )
        {
            file << "\t\t<disk type='block' device='disk'>" << endl
                 << "\t\t\t<source dev='" << vm->get_remote_dir() << "/disk." 
                 << i << "'/>" << endl;
        }
        else
        {
            file << "\t\t<disk type='file' device='disk'>" << endl
                 << "\t\t\t<source file='" << vm->get_remote_dir() << "/disk." 
                 << i << "'/>" << endl;
        }

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
    // Context Device
    // ------------------------------------------------------------------------

    if ( vm->get_template_attribute("CONTEXT",attrs) == 1 )
    {
        context = dynamic_cast<const VectorAttribute *>(attrs[0]);
        target  = context->vector_value("TARGET");

        if ( !target.empty() )
        {
            file << "\t\t<disk type='file' device='disk'>" << endl;
            file << "\t\t\t<source file='" << vm->get_remote_dir() << "/disk."
                 << num << "'/>" << endl;
            file << "\t\t\t<target dev='" << target << "'/>" << endl;
            file << "\t\t\t<readonly/>" << endl;
            file << "\t\t</disk>" << endl;
        }
        else
        {
            vm->log("VMM", Log::WARNING, "Could not find target device to"
                " attach context, will continue without it.");
        }
    }

    attrs.clear();

    // ------------------------------------------------------------------------
    // Network interfaces
    // ------------------------------------------------------------------------

    num = vm->get_template_attribute("NIC",attrs);

    for(int i=0; i<num; i++)
    {
        nic = dynamic_cast<const VectorAttribute *>(attrs[i]);

        if ( nic == 0 )
        {
            continue;
        }

        bridge = nic->vector_value("BRIDGE");
        mac    = nic->vector_value("MAC");
        target = nic->vector_value("TARGET");
        script = nic->vector_value("SCRIPT");
        model  = nic->vector_value("MODEL");

        if ( bridge.empty() )
        {
            file << "\t\t<interface type='ethernet'>" << endl;
        }
        else
        {
            file << "\t\t<interface type='bridge'>" << endl;
            file << "\t\t\t<source bridge='" << bridge << "'/>" << endl;
        }

        if( !mac.empty() )
        {
            file << "\t\t\t<mac address='" << mac << "'/>" << endl;
        }

        if( !target.empty() )
        {
            file << "\t\t\t<target dev='" << target << "'/>" << endl;
        }

        if( !script.empty() )
        {
            file << "\t\t\t<script path='" << script << "'/>" << endl;
        }

        if( !model.empty() )
        {
            file << "\t\t\t<model type='" << model << "'/>" << endl;
        }

        file << "\t\t</interface>" << endl;

    }

    attrs.clear();

    // ------------------------------------------------------------------------
    // Graphics
    // ------------------------------------------------------------------------

    if ( vm->get_template_attribute("GRAPHICS",attrs) > 0 )
    {
        graphics = dynamic_cast<const VectorAttribute *>(attrs[0]);

        if ( graphics != 0 )
        {
            type   = graphics->vector_value("TYPE");
            listen = graphics->vector_value("LISTEN");
            port   = graphics->vector_value("PORT");
            passwd = graphics->vector_value("PASSWD");
            keymap = graphics->vector_value("KEYMAP");

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

                if ( !keymap.empty() )
                {
                    file << " keymap='" << keymap << "'";
                }

                file << "/>" << endl;
            }
            else
            {
                vm->log("VMM", Log::WARNING,
                        "Not supported graphics type, ignored.");
            }
        }
    }

    attrs.clear();

    // ------------------------------------------------------------------------
    // Input
    // ------------------------------------------------------------------------

    if ( vm->get_template_attribute("INPUT",attrs) > 0 )
    {
        input = dynamic_cast<const VectorAttribute *>(attrs[0]);

        if ( input != 0 )
        {
            type = input->vector_value("TYPE");
            bus  = input->vector_value("BUS");

            if ( !type.empty() )
            {
                file << "\t\t<input type='" << type << "'";

                if ( !bus.empty() )
                {
                    file << " bus='" << bus << "'";
                }

                file << "/>" << endl;
            }
        }
    }

    attrs.clear();

    file << "\t</devices>" << endl;

    // ------------------------------------------------------------------------
    // Features
    // ------------------------------------------------------------------------

    num = vm->get_template_attribute("FEATURES",attrs);

    if ( num > 0 )
    {
        features = dynamic_cast<const VectorAttribute *>(attrs[0]);

        if ( features != 0 )
        {
            pae  = features->vector_value("PAE");
            acpi = features->vector_value("ACPI");
        }
    }
    
    if ( pae.empty() )
    {
        get_default("FEATURES", "PAE", pae);
    }
    
    if ( acpi.empty() )
    {
        get_default("FEATURES", "ACPI", acpi);
    }
    
    if( acpi == "yes" || pae == "yes" )
    {
        file << "\t<features>" << endl;

        if ( pae == "yes" )
        {
            file << "\t\t<pae/>" << endl;
        }

        if ( acpi == "yes" )
        {
            file << "\t\t<acpi/>" << endl;
        }

        file << "\t</features>" << endl;
    }

    attrs.clear();

    // ------------------------------------------------------------------------
    // Raw KVM attributes
    // ------------------------------------------------------------------------

    num = vm->get_template_attribute("RAW",attrs);

    for(int i=0; i<num;i++)
    {
        raw = dynamic_cast<const VectorAttribute *>(attrs[i]);

        if ( raw == 0 )
        {
            continue;
        }

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

error_memory:
    vm->log("VMM", Log::ERROR, "No MEMORY defined and no default provided.");
    file.close();
    return -1;

error_boot:
    vm->log("VMM", Log::ERROR, "No BOOT device defined and no default provided.");
    file.close();
    return -1;

error_disk:
    vm->log("VMM", Log::ERROR, "Wrong target value in DISK.");
    file.close();
    return -1;
}
