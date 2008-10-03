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

#ifndef MAD_MANAGER_H_
#define MAD_MANAGER_H_

#include <pthread.h>
#include <sys/types.h>

#include <string>
#include <vector>
#include <sstream>
#include <vector>

#include "Mad.h"
#include "Attribute.h"

using namespace std;

extern "C" void * mad_manager_listener(void * _mm);

/**
 * Provides general functionality for driver management. The MadManager serves
 * Nebula managers as base clase.
 */
class MadManager
{
public:
    /**
     *  Function to initialize the MAD management system. This function
     *  MUST be called once before using the MadManager class. This function
     *  blocks the SIG_PIPE (broken pipe) signal that may occur when a driver
     *  crashes
     */
    static void mad_manager_system_init();
    
    /**
     *  Loads Virtual Machine Manager Mads defined in configuration file
     *   @param uid of the user executing the driver. When uid is 0 the nebula 
     *   identity will be used. Otherwise the Mad will be loaded through the
     *   sudo application. 
     */
    virtual void load_mads(int uid) = 0;
      
protected:

    MadManager(vector<const Attribute *>& _mads);

    virtual ~MadManager();

    /**
     *  Vector containing Mad configuration for this Manager, as described in 
     *  the nebula location
     */
    vector<const Attribute *> mad_conf;
        
    /**
     *  This function initialize the communication pipes to register new MADs
     *  in the manager, and starts a listener to wait for driver messages.
     */
    virtual int start();
    

    /**
     *  This function closes the communication pipes, stops the listener thread, 
     *  and finalize the associated drivers.
     */
    virtual void stop();    

    /**
     *  Get a mad
     */
    virtual const Mad * get(int uid, const string& name, const string& value);

    /**
     *  Register a new mad in the manager. The Mad is previously started, and
     *  then the listener thread is notified through the pipe_w stream. In case
     *  of failure the calling function MUST free the Mad.
     *    @param mad pointer to the mad to be added to the manager.
     *    @return 0 on success.
     */
    int add(Mad *mad);
    
private:
    /**
     *  Function to lock the Manager
     */
    void lock()
    {
        pthread_mutex_lock(&mutex);
    };

    /**
     *  Function to unlock the Manager
     */
    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    };
    
    /**
     *  Function to execute the listener method within a new pthread (requires
     *  C linkage)
     */
    friend void * mad_manager_listener(void * _mm);

    /**
     *  Synchronization mutex (listener & manager threads)
     */
    pthread_mutex_t         mutex;

    /**
     *  Thread id for the listener process
     */
    pthread_t               listener_thread;

    /**
     *  Communication pipe (read end) to link the Manager and the listener 
     *  thread
     */
    int                     pipe_r;
    
    /**
     *  Communication pipe (write end) to link the Manager and the listener 
     *  thread
     */    
    int                     pipe_w;

    /**
     *  This vector contains the file descriptors of the driver pipes (to read
     *  Mads responses)
     */
    vector<int>             fds;
    
    /**
     *  The sets of Mads managed by the MadManager
     */
    vector<Mad *>           mads;
        
    /**
     *  Read buffer for the listener. This variable is in the class so it
     *  can be free upon listener thread cancellation.
     */
    ostringstream           buffer;
    
    /**
     *  Listener thread implementation.
     */
    void listener();        
};

#endif /*MAD_MANAGER_H_*/
