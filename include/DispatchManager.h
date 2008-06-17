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

#ifndef DISPATCH_MANAGER_H_
#define DISPATCH_MANAGER_H_

#include "ActionManager.h"
#include "HostPool.h"
#include "VirtualMachinePool.h"

using namespace std;

extern "C" void * dm_action_loop(void *arg);

class DispatchManager : public ActionListener
{
public:

    DispatchManager(
        VirtualMachinePool *        _vmpool,
        HostPool *                  _hpool):
            hpool(_hpool),
            vmpool(_vmpool)
    {
        am.addListener(this);
    };

    ~DispatchManager()
    {}
    ;

    enum Actions
    {
        SUSPEND_SUCCESS, /**< Send by LCM when a VM is suspended*/
        SUSPEND_FAILURE, /**< Send by LCM when a VM could not be suspended*/
        STOP_SUCCESS,    /**< Send by LCM when a VM is stopped*/
        STOP_FAILURE,    /**< Send by LCM when a VM could not be stopped*/
        MIGRATE_FAILURE, /**< Send by LCM when a VM could not be migrated*/
        DONE,            /**< Send by LCM when a VM is shut down*/
        FAILED,          /**< Send by LCM when one of the execution steps fails*/
        FINALIZE
    };

    /**
     *  Triggers specific actions to the Dispatch Manager. This function
     *  wraps the ActionManager trigger function.
     *    @param action the DM action
     *    @param vid VM unique id. This is the argument of the passed to the 
     *    invoked action.
     */
    void trigger(
        Actions action,
        int     _vid);

    /**
     *  This functions creates a new thread for the Dispatch Manager. This 
     *  thread will wait in an action loop till it receives ACTION_FINALIZE.
     *    @return 0 on success.
     */
    int start();
    
    /**
     *  Gets the thread identification.
     *    @return pthread_t for the manager thread (that in the action loop).
     */
    pthread_t get_thread_id() const
    {
        return dm_thread;
    };

    //--------------------------------------------------------------------------
    // DM Actions, the RM and the Scheduler will invoke this methods
    //--------------------------------------------------------------------------

    /**
     *  Allocates a new virtual machine
     *    @return 0 on success
     */
    int allocate (
        int     uid,
        const string& stemplate,
        int *   oid);

    /**
     *  Deploys a VM. A new history record MUST be added before calling this
     *  function. Also the VM MUST have its mutex locked. If the function fails
     *  the calling funtion is responsible for recovering from the error.
     *    @param vm pointer to a VirtualMachine with its mutex locked.
     *    @return 0 on success 
     */
    int deploy (
        VirtualMachine * vm);

    /**
     *  Migrates a VM. The following actions must be performed before calling 
     *  this function: 
     *    - Lock the VM mutex.
     *    - Update the History statistics of the current host.
     *    - Add a new History record with the new host. 
     *  If the function fails the calling funtion is responsible for recovering 
     *  from the error.
     *    @param vm pointer to a VirtualMachine with its mutex locked.
     *    @return 0 on success 
     */
    int migrate(
        VirtualMachine * vm);

    /**
     *  Migrates a VM. The following actions must be performed before calling 
     *  this function: 
     *    - Lock the VM mutex.
     *    - Update the History statistics of the current host.
     *    - Add a new History record with the new host. 
     *  If the function fails the calling funtion is responsible for recovering 
     *  from the error.
     *    @param vm pointer to a VirtualMachine with its mutex locked.
     *    @return 0 on success 
     */
    int live_migrate(
        VirtualMachine * vm);
        
    /**
     *  Shuts down a VM.
     *    @param vid VirtualMachine identification
     *    @return 0 on success, -1 if the VM does not exits or -2 if the VM is 
     *    in a wrong a state 
     */            
    int shutdown (
        int vid);

    /**
     *  Holds a VM.
     *    @param vid VirtualMachine identification
     *    @return 0 on success, -1 if the VM does not exits or -2 if the VM is 
     *    in a wrong a state 
     */            
    int hold(
        int vid);

    /**
     *  Releases a VM.
     *    @param vid VirtualMachine identification
     *    @return 0 on success, -1 if the VM does not exits or -2 if the VM is 
     *    in a wrong a state 
     */            
    int release(
        int vid);

    /**
     *  Stops a VM.
     *    @param vid VirtualMachine identification
     *    @return 0 on success, -1 if the VM does not exits or -2 if the VM is 
     *    in a wrong a state 
     */       
    int stop(
        int vid);

    /**
     *  Suspends a VM.
     *    @param vid VirtualMachine identification
     *    @return 0 on success, -1 if the VM does not exits or -2 if the VM is 
     *    in a wrong a state 
     */       
    int suspend(
        int vid);

    /**
     *  Resumes a VM.
     *    @param vid VirtualMachine identification
     *    @return 0 on success, -1 if the VM does not exits or -2 if the VM is 
     *    in a wrong a state 
     */       
    int resume(
        int vid);
        
    /**
     *  Ends a VM life cycle inside ONE.
     *    @param vid VirtualMachine identification
     *    @return 0 on success, -1 if the VM does not exits or -2 if the VM is 
     *    in a wrong a state 
     */       
    int finalize(
        int vid);

private:
    /**
     *  Thread id for the Dispatch Manager
     */
    pthread_t               dm_thread;

    /**
     *  Pointer to the Host Pool, to access hosts
     */
    HostPool *              hpool;

    /**
     *  Pointer to the Host Pool, to access hosts
     */
    VirtualMachinePool *    vmpool;

    /**
     *  Action engine for the Manager
     */
    ActionManager           am;

    /**
     *  Function to execute the Manager action loop method within a new pthread 
     * (requires C linkage)
     */
    friend void * dm_action_loop(void *arg);

    /**
     *  The action function executed when an action is triggered.
     *    @param action the name of the action
     *    @param arg arguments for the action function
     */
    void do_action(
        const string &  action,
        void *          arg);

    //--------------------------------------------------------------------------
    // DM Actions associated with a VM state transition
    //--------------------------------------------------------------------------
    
    void  suspend_success_action(int vid);

    void  suspend_failure_action(int vid);

    void  stop_success_action(int vid);

    void  stop_failure_action(int vid);

    void  migrate_failure_action(int vid);

    void  done_action(int vid);

    void  failed_action(int vid);
    
    void  host_add_vm(VirtualMachine *vm);
    
    void  host_del_vm(VirtualMachine *vm);
    
};

#endif /*DISPATCH_MANAGER_H*/

