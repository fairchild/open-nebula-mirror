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

#include "VirtualMachineHook.h"
#include "VirtualMachine.h"
#include "Nebula.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void VirtualMachineAllocateHook::do_hook(void *arg)
{
    VirtualMachine * vm;
    int              rc;
    string           parsed_args;

    vm = static_cast<VirtualMachine *>(arg);

    if ( vm == 0 )
    {
        return;
    }

    rc = vm->parse_template_attribute(args, parsed_args);

    if ( rc == 0)
    {
        Nebula& ne                    = Nebula::instance();
        HookManager * hm              = ne.get_hm();
        const HookManagerDriver * hmd = hm->get();

        if ( hmd != 0 )
        {
            hmd->execute(vm->get_oid(),name,cmd,parsed_args);
        }
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int VirtualMachineStateMap::get_state(int id,
        VirtualMachine::LcmState &lcm_state)
{
    map<int,VirtualMachine::LcmState>::iterator it;

    it = vm_states.find(id);

    if ( it == vm_states.end() )
    {
        return -1;
    }

    lcm_state = it->second;

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void VirtualMachineStateMap::update_state (int id,
        VirtualMachine::LcmState lcm_state,
        VirtualMachine::VmState   vm_state)
{
    map<int,VirtualMachine::LcmState>::iterator it;

    it = vm_states.find(id);

    if ( it == vm_states.end() )
    {
        vm_states.insert(make_pair(id,lcm_state));
    }
    else
    {
        if ( vm_state == VirtualMachine::DONE )
        {
            vm_states.erase(it);
        }
        else
        {
            it->second = lcm_state;
        }
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void VirtualMachineStateHook::do_hook(void *arg)
{

    VirtualMachine * vm;
    int              rc;

    VirtualMachine::LcmState lcm_state;
    VirtualMachineStateMap&  vm_sm = VirtualMachineStateMap::instance();

    vm = static_cast<VirtualMachine *>(arg);

    if ( vm == 0 )
    {
        return;
    }

    rc = vm_sm.get_state(vm->get_oid(),lcm_state);

    if ( rc != 0 )
    {
        return;
    }

    if ((lcm_state != state) && (vm->get_lcm_state() == state))
    {
        string  parsed_args;
        int     rc = vm->parse_template_attribute(args, parsed_args);

        if ( rc == 0)
        {
            Nebula& ne        = Nebula::instance();
            HookManager * hm  = ne.get_hm();

            const HookManagerDriver * hmd = hm->get();

            if ( hmd != 0 )
            {
                if ( ! remote )
                {
                    hmd->execute(vm->get_oid(),name,cmd,parsed_args);
                }
                else if ( vm->hasHistory() )
                {
                    hmd->execute(vm->get_oid(),
                         name,
                         vm->get_hostname(),
                         cmd,
                         parsed_args);
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void VirtualMachineStateMapHook::do_hook(void *arg)
{
    VirtualMachine * vm = static_cast<VirtualMachine *>(arg);

    if ( vm == 0 )
    {
        return;
    }

    VirtualMachineStateMap& vm_sm = VirtualMachineStateMap::instance();

    vm_sm.update_state(vm->get_oid(), vm->get_lcm_state(), vm->get_state());
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
