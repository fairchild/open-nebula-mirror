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

#ifndef VIRTUAL_MACHINE_HOOK_H_
#define VIRTUAL_MACHINE_HOOK_H_

#include <vector>
#include <string>

#include "Hook.h"
#include "VirtualMachine.h"

using namespace std;

/**
 *  This class is general VM Allocate Hook that executes a command locally
 *  when the VM is inserted in the database. The VirtualMachine object is
 *  looked
 */
class VirtualMachineAllocateHook : public Hook
{
public:
    // -------------------------------------------------------------------------
    // Init a LOCAL hook of ALLOCATE type
    // -------------------------------------------------------------------------
    VirtualMachineAllocateHook(const string& name,
                               const string& cmd,
                               const string& args):
        Hook(name, cmd, args, Hook::ALLOCATE, false){};

    ~VirtualMachineAllocateHook(){};

    // -------------------------------------------------------------------------
    // Hook methods
    // -------------------------------------------------------------------------

    void do_hook(void *arg);
};

/**
 *  This class provides basic functionality to store VM states for state Hooks.
 *  The state Map is shared by all the State hooks. A maintenance hook that
 *  updates the map should be added.
 */
class VirtualMachineStateMap
{
public:

    static VirtualMachineStateMap& instance()
    {
        static VirtualMachineStateMap vm_sm;

        return vm_sm;
    };

    // -------------------------------------------------------------------------
    // Functions to handle the VM state map
    // -------------------------------------------------------------------------
    /**
     *  Gets the state associated to the VM
     *    @param id of the VM
     *    @param lcm_state (previous) of the VM
     *    @return 0 if the previous state for the VM has been recorded
     */
    int get_state(int id, VirtualMachine::LcmState &lcm_state);

    /**
     *  Updates the state associated to the VM
     *    @param id of the VM
     *    @param lcm_state (current) of the VM
     */
    void update_state (int                      id,
                       VirtualMachine::LcmState lcm_state,
                       VirtualMachine::VmState  vm_state);

private:
    // -------------------------------------------------------------------------
    // Init the Map
    // -------------------------------------------------------------------------
    VirtualMachineStateMap(){};

    ~VirtualMachineStateMap(){};

    /**
     *  The state Map for the VMs
     */
    map<int,VirtualMachine::LcmState> vm_states;
};

/**
 *  This class is a general VM State Hook that executes a command locally or
 *  remotelly when the VM gets into a given state (one shot). The VirtualMachine
 *  object is looked when the hook is invoked.
 */
class VirtualMachineStateHook : public Hook
{
public:
    // -------------------------------------------------------------------------
    // Init a LOCAL hook of ALLOCATE type
    // -------------------------------------------------------------------------
    /**
     *  Creates a VirtualMachineStateHook
     *    @param name of the hook
     *    @param cmd for the hook
     *    @param args for the hook
     *    @param _state the hook will be executed when the VM enters this state
     *    @param remote the hook will be executed on the target resource
     */
    VirtualMachineStateHook(const string&            name,
                            const string&            cmd,
                            const string&            args,
                            bool                     remote,
                            VirtualMachine::LcmState _state):
        Hook(name, cmd,args,Hook::UPDATE,remote), state(_state){};

    ~VirtualMachineStateHook(){};

    // -------------------------------------------------------------------------
    // Hook methods
    // -------------------------------------------------------------------------
    void do_hook(void *arg);

private:
    /**
     *  The target hook state
     */
    VirtualMachine::LcmState state;
};

/**
 *  This class implements a state Map updater, one hook of this type should be
 *  added in order to mantain the VM state map.
 */
class VirtualMachineStateMapHook : public Hook
{
public:
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    VirtualMachineStateMapHook():Hook(name, cmd, args, Hook::UPDATE, remote){};

    ~VirtualMachineStateMapHook(){};

    // -------------------------------------------------------------------------
    // Hook methods
    // -------------------------------------------------------------------------
    void do_hook(void *arg);
};

#endif
