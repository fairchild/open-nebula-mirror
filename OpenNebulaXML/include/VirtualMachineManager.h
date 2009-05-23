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

#ifndef VIRTUAL_MACHINE_MANAGER_H_
#define VIRTUAL_MACHINE_MANAGER_H_

#include "MadManager.h"
#include "ActionManager.h"
#include "VirtualMachineManagerDriver.h"
#include "VirtualMachinePool.h"
#include "HostPool.h"
#include "NebulaTemplate.h"

using namespace std;

extern "C" void * vmm_action_loop(void *arg);

class VirtualMachineManager : public MadManager, public ActionListener
{
public:

    VirtualMachineManager(
        VirtualMachinePool *      _vmpool,
        HostPool *                _hpool,
        time_t                    _timer_period,
        time_t                    _poll_period,        
        vector<const Attribute*>& _mads);

    ~VirtualMachineManager(){};

    enum Actions
    {
        DEPLOY,
        SAVE,
        SHUTDOWN,
        CANCEL,
        MIGRATE,
        RESTORE,
        POLL,
        TIMER,
        FINALIZE
    };

    /**
     *  Triggers specific actions to the Virtual Machine Manager. This function
     *  wraps the ActionManager trigger function.
     *    @param action the VMM action
     *    @param vid VM unique id. This is the argument of the passed to the 
     *    invoked action.
     */
    void trigger(
        Actions action,
        int     vid);

    /**
     *  This functions starts the associated listener thread, and creates a 
     *  new thread for the Virtual Machine Manager. This thread will wait in
     *  an action loop till it receives ACTION_FINALIZE.
     *    @return 0 on success.
     */
    int start();

    /**
     *  Gets the thread identification.
     *    @return pthread_t for the manager thread (that in the action loop).
     */
    pthread_t get_thread_id() const
    {
        return vmm_thread;
    };
    
    /**
     *  Loads Virtual Machine Manager Mads defined in configuration file
     *   @param uid of the user executing the driver. When uid is 0 the nebula 
     *   identity will be used. Otherwise the Mad will be loaded through the
     *   sudo application. 
     */
    void load_mads(int uid);
    
private:
    /**
     *  Thread id for the Virtual Machine Manager
     */
    pthread_t               vmm_thread;

    /**
     *  Pointer to the Virtual Machine Pool, to access VMs
     */
    VirtualMachinePool *    vmpool;

    /**
     *  Pointer to the Host Pool, to access hosts
     */
    HostPool *              hpool;
        
    /**
     *  Timer period for the Virtual Machine Manager.
     */
    time_t                  timer_period;

    /**
     *  Virtual Machine polling interval
     */
    time_t                  poll_period;

    /**
     *  Action engine for the Manager
     */
    ActionManager           am;

    /**
     *  Function to execute the Manager action loop method within a new pthread 
     * (requires C linkage)
     */
    friend void * vmm_action_loop(void *arg);

    /**
     *  Returns a pointer to a Virtual Machine Manager driver.
     *    @param uid of the owner of the driver
     *    @param name of an attribute of the driver (e.g. its type)
     *    @param value of the attribute
     *    @return the VM driver owned by uid with attribute name equal to value
     *    or 0 in not found
     */
    const VirtualMachineManagerDriver * get(
        int             uid,
        const string&   name,
        const string&   value)
    {
        return static_cast<const VirtualMachineManagerDriver *>
               (MadManager::get(uid,name,value));
    };

    /**
     *  Returns a pointer to a Virtual Machine Manager driver. The driver is 
     *  searched by its name.
     *    @param uid of the owner of the driver
     *    @param name the name of the driver
     *    @return the VM driver owned by uid with attribute name equal to value
     *    or 0 in not found
     */
    const VirtualMachineManagerDriver * get(
        int             uid,
        const string&   name)
    {
        string _name("NAME");
        return static_cast<const VirtualMachineManagerDriver *>
               (MadManager::get(uid,_name,name));
    };
    
    /**
     *  The action function executed when an action is triggered.
     *    @param action the name of the action
     *    @param arg arguments for the action function
     */
    void do_action(
        const string &  action,
        void *          arg);

    /**
     *  Function executed when a DEPLOY action is received. It deploys a VM on
     *  a Host.
     *    @param vid the id of the VM to be deployed.
     */
    void deploy_action(
        int vid);

    /**
     *  Function to stop a running VM and generate a checkpoint file. This 
     *  function is executed when a SAVE action is triggered.
     *    @param vid the id of the VM.
     */
    void save_action(
        int vid);

    /**
     *  Shutdowns a VM when a SHUTDOWN action is received.
     *    @param vid the id of the VM.
     */
    void shutdown_action(
        int vid);

    /**
     *  Cancels a VM when a CANCEL action is received.
     *    @param vid the id of the VM.
     */
    void cancel_action(
        int vid);

    /**
     *  Function to migrate (live) a VM (MIGRATE action).
     *    @param vid the id of the VM.
     */
    void migrate_action(
        int vid);

    /**
     *  Restores a VM from a checkpoint file.
     *    @param vid the id of the VM.
     */
    void restore_action(
        int vid);

    /**
     *  Polls a VM.
     *    @param vid the id of the VM.
     */
    void poll_action(
        int vid);
    
    /**
     *  This function is executed periodically to poll the running VMs
     */
    void timer_action();
};

#endif /*VIRTUAL_MACHINE_MANAGER_H*/
