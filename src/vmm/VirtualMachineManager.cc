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

#include "VirtualMachineManager.h"
#include "Nebula.h"
#include "XenDriver.h"
#include "XMLDriver.h"
#include "LibVirtDriver.h"
#include <time.h>

/* ************************************************************************** */
/* Constructor                                                                */
/* ************************************************************************** */

VirtualMachineManager::VirtualMachineManager(
    VirtualMachinePool *            _vmpool,
    HostPool *                      _hpool,
    time_t                          _timer_period,
    time_t                          _poll_period,
    vector<const Attribute*>&       _mads):
        MadManager(_mads),
        vmpool(_vmpool),
        hpool(_hpool),
        timer_period(_timer_period),
        poll_period(_poll_period)
{
    am.addListener(this);
};

/* ************************************************************************** */
/* Manager start function                                                     */
/* ************************************************************************** */

extern "C" void * vmm_action_loop(void *arg)
{
    VirtualMachineManager *  vmm;

    if ( arg == 0 )
    {
        return 0;
    }

    vmm = static_cast<VirtualMachineManager *>(arg);

    Nebula::log("VMM",Log::INFO,"Virtual Machine Manager started.");

    vmm->am.loop(vmm->timer_period,0);

    Nebula::log("VMM",Log::INFO,"Virtual Machine Manager stopped.");

    return 0;
}

/* -------------------------------------------------------------------------- */

int VirtualMachineManager::start()
{
    int               rc;
    pthread_attr_t    pattr;

    rc = MadManager::start();

    if ( rc != 0 )
    {
        return -1;
    }

    Nebula::log("VMM",Log::INFO,"Starting Virtual Machine Manager...");

    pthread_attr_init (&pattr);
    pthread_attr_setdetachstate (&pattr, PTHREAD_CREATE_JOINABLE);

    rc = pthread_create(&vmm_thread,&pattr,vmm_action_loop,(void *) this);

    return rc;
};

/* ************************************************************************** */
/* Manager Action Interface                                                   */
/* ************************************************************************** */

void VirtualMachineManager::trigger(Actions action, int _vid)
{
    int *   vid;
    string  aname;

    vid = new int(_vid);

    switch (action)
    {
    case DEPLOY:
        aname = "DEPLOY";
        break;

    case SAVE:
        aname = "SAVE";
        break;

    case RESTORE:
        aname = "RESTORE";
        break;

    case SHUTDOWN:
        aname = "SHUTDOWN";
        break;

    case CANCEL:
        aname = "CANCEL";
        break;

    case MIGRATE:
        aname = "MIGRATE";
        break;

    case POLL:
        aname = "POLL";
        break;

    case FINALIZE:
        aname = ACTION_FINALIZE;
        break;

    default:
        delete vid;
        return;
    }

    am.trigger(aname,vid);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void VirtualMachineManager::do_action(const string &action, void * arg)
{
    int vid;
    ostringstream os;

    if ( arg == 0)
    {
        if ( action != ACTION_TIMER && action != ACTION_FINALIZE )
            return;
    }
    else
    {
        vid  = *(static_cast<int *>(arg));

        delete static_cast<int *>(arg);
    }

    if (action == "DEPLOY")
    {
        deploy_action(vid);
    }
    else if (action == "SAVE")
    {
        save_action(vid);
    }
    else if (action == "RESTORE")
    {
        restore_action(vid);
    }
    else if (action == "SHUTDOWN")
    {
        shutdown_action(vid);
    }
    else if (action == "CANCEL")
    {
        cancel_action(vid);
    }
    else if (action == "MIGRATE")
    {
        migrate_action(vid);
    }
    else if (action == "POLL")
    {
        poll_action(vid);
    }
    else if (action == ACTION_TIMER)
    {
        timer_action();
    }
    else if (action == ACTION_FINALIZE)
    {
        Nebula::log("VMM",Log::INFO,"Stopping Virtual Machine Manager...");

        MadManager::stop();
    }
    else
    {
        ostringstream oss;
        oss << "Unknown action name: " << action;

        Nebula::log("VMM", Log::ERROR, oss);
    }
}

/* ************************************************************************** */
/* Manager Actions                                                            */
/* ************************************************************************** */

void VirtualMachineManager::deploy_action(int vid)
{
    VirtualMachine *                vm;
    const VirtualMachineManagerDriver *   vmd;
    int                             rc;
    ostringstream                   os;

    // Get the VM from the pool
    vm = vmpool->get(vid,true);

    if (vm == 0)
    {
        return;
    }

    if (!vm->hasHistory())
    {
        goto error_history;
    }

    // Get the driver for this VM
    vmd = get(vm->get_uid(),vm->get_vmm_mad());

    if ( vmd == 0 )
    {
        goto error_driver;
    }

    //Generate VM description file
    os.str("");
    os << "Generating deployment file: " << vm->get_deployment_file();

    vm->log("VMM", Log::INFO, os);

    rc = vmd->deployment_description(vm,vm->get_deployment_file());

    if (rc != 0)
    {
        goto error_file;
    }

    // Invoke driver method
    vmd->deploy(vid,vm->get_hostname(),vm->get_remote_deployment_file());

    vm->unlock();

    return;

error_history:
    os.str("");
    os << "deploy_action, VM has no history";
    goto error_common;

error_driver:
    os.str("");
    os << "deploy_action, error getting driver " << vm->get_vmm_mad();
    goto error_common;

error_file:
    os.str("");
    os << "deploy_action, error generating deployment file: " << vm->get_deployment_file();
    goto error_common;

error_common:
    Nebula              &ne = Nebula::instance();
    LifeCycleManager *  lcm = ne.get_lcm();

    lcm->trigger(LifeCycleManager::DEPLOY_FAILURE, vid);

    vm->log("VMM", Log::ERROR, os);

    vm->unlock();
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void VirtualMachineManager::save_action(
    int vid)
{
    VirtualMachine *                    vm;
    const VirtualMachineManagerDriver * vmd;
    string                              hostname;
    ostringstream                       os;

    // Get the VM from the pool
    vm = vmpool->get(vid,true);

    if (vm == 0)
    {
        return;
    }

    if (!vm->hasHistory())
    {
        goto error_history;
    }

    // Get the driver for this VM
    vmd = get(vm->get_uid(),vm->get_vmm_mad());

    if ( vmd == 0 )
    {
        goto error_driver;
    }

    // Use previous host if it is a migration
    if ( vm->get_lcm_state() == VirtualMachine::SAVE_MIGRATE )
    {
        if (!vm->hasPreviousHistory())
        {
            goto error_previous_history;
        }

        hostname = vm->get_previous_hostname();
    }
    else
    {
        hostname=vm->get_hostname();
    }

    // Invoke driver method
    vmd->save(
        vid,
        hostname,
        vm->get_deploy_id(),
        vm->get_checkpoint_file());

    vm->unlock();
    return;

error_history:
    os.str("");
    os << "save_action, VM has no history";
    goto error_common;

error_driver:
    os.str("");
    os << "save_action, error getting driver " << vm->get_vmm_mad();
    goto error_common;

error_previous_history:
    os.str("");
    os << "save_action, VM has no previous history";

error_common:
    Nebula              &ne = Nebula::instance();
    LifeCycleManager *  lcm = ne.get_lcm();

    lcm->trigger(LifeCycleManager::SAVE_FAILURE, vid);

    vm->log("VMM", Log::ERROR, os);

    vm->unlock();
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void VirtualMachineManager::shutdown_action(
    int vid)
{
    VirtualMachine *                vm;
    const VirtualMachineManagerDriver *   vmd;
    ostringstream                       os;

    // Get the VM from the pool
    vm = vmpool->get(vid,true);

    if (vm == 0)
    {
        return;
    }

    if (!vm->hasHistory())
    {
        goto error_history;
    }

    // Get the driver for this VM
    vmd = get(vm->get_uid(),vm->get_vmm_mad());

    if ( vmd == 0 )
    {
        goto error_driver;
    }

    // Invoke driver method
    vmd->shutdown(vid,vm->get_hostname(),vm->get_deploy_id());

    vm->unlock();
    return;

error_history:
    os.str("");
    os << "shutdown_action, VM has no history";
    goto error_common;

error_driver:
    os.str("");
    os << "shutdown_action, error getting driver " << vm->get_vmm_mad();
    goto error_common;

error_common:
    Nebula              &ne = Nebula::instance();
    LifeCycleManager *  lcm = ne.get_lcm();

    lcm->trigger(LifeCycleManager::SHUTDOWN_FAILURE, vid);

    vm->log("VMM", Log::ERROR, os);

    vm->unlock();
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void VirtualMachineManager::cancel_action(
    int vid)
{
    VirtualMachine *                vm;
    const VirtualMachineManagerDriver *   vmd;
    ostringstream                       os;

    // Get the VM from the pool
    vm = vmpool->get(vid,true);

    if (vm == 0)
    {
        return;
    }

    if (!vm->hasHistory())
    {
        goto error_history;
    }

    // Get the driver for this VM
    vmd = get(vm->get_uid(),vm->get_vmm_mad());

    if ( vmd == 0 )
    {
        goto error_driver;
    }

    // Invoke driver method
    vmd->cancel(vid,vm->get_hostname(),vm->get_deploy_id());

    vm->unlock();
    return;

error_history:
    os.str("");
    os << "cancel_action, VM has no history";
    goto error_common;

error_driver:
    os.str("");
    os << "cancel_action, error getting driver " << vm->get_vmm_mad();
    goto error_common;

error_common:
    Nebula              &ne = Nebula::instance();
    LifeCycleManager *  lcm = ne.get_lcm();

    lcm->trigger(LifeCycleManager::CANCEL_FAILURE, vid);

    vm->log("VMM", Log::ERROR, os);

    vm->unlock();
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void VirtualMachineManager::migrate_action(
    int vid)
{
    VirtualMachine *                    vm;
    const VirtualMachineManagerDriver * vmd;
    ostringstream                       os;

    // Get the VM from the pool
    vm = vmpool->get(vid,true);

    if (vm == 0)
    {
        return;
    }

    if (!vm->hasHistory())
    {
        goto error_history;
    }

    // Get the driver for this VM
    vmd = get(vm->get_uid(),vm->get_vmm_mad());

    if ( vmd == 0 )
    {
        goto error_driver;
    }

    if (!vm->hasPreviousHistory())
    {
        goto error_previous_history;
    }

    // Invoke driver method
    vmd->migrate(vid,
            vm->get_previous_hostname(),
            vm->get_deploy_id(),
            vm->get_hostname());

    vm->unlock();

    return;

error_history:
    os.str("");
    os << "migrate_action, VM has no history";
    goto error_common;

error_driver:
    os.str("");
    os << "migrate_action, error getting driver " << vm->get_vmm_mad();
    goto error_common;

error_previous_history:
    os.str("");
    os << "migrate_action, error VM has no previous history";
    goto error_common;

error_common:
    Nebula              &ne = Nebula::instance();
    LifeCycleManager *  lcm = ne.get_lcm();

    lcm->trigger(LifeCycleManager::DEPLOY_FAILURE, vid);

    vm->log("VMM", Log::ERROR, os);

    vm->unlock();
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void VirtualMachineManager::restore_action(
    int vid)
{
    VirtualMachine *                vm;
    const VirtualMachineManagerDriver *   vmd;
    ostringstream                       os;

    // Get the VM from the pool
    vm = vmpool->get(vid,true);

    if (vm == 0)
    {
        return;
    }

    if (!vm->hasHistory())
    {
        goto error_history;
    }

    // Get the driver for this VM
    vmd = get(vm->get_uid(),vm->get_vmm_mad());

    if ( vmd == 0 )
    {
        goto error_driver;
    }

    // Invoke driver method
    vmd->restore(vid,
                 vm->get_hostname(),
                 vm->get_deploy_id(),
                 vm->get_checkpoint_file());

    vm->unlock();
    return;

error_history:
    os.str("");
    os << "restore_action, VM has no history";
    goto error_common;

error_driver:
    os.str("");
    os << "restore_action, error getting driver " << vm->get_vmm_mad();
    goto error_common;

error_common:
    Nebula              &ne = Nebula::instance();
    LifeCycleManager *  lcm = ne.get_lcm();

    lcm->trigger(LifeCycleManager::DEPLOY_FAILURE, vid);

    vm->log("VMM", Log::ERROR, os);

    vm->unlock();
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void VirtualMachineManager::poll_action(
    int vid)
{
    VirtualMachine *                vm;
    const VirtualMachineManagerDriver *   vmd;
    ostringstream                       os;

    // Get the VM from the pool
    vm = vmpool->get(vid,true);

    if (vm == 0)
    {
        return;
    }

    if (!vm->hasHistory())
    {
        goto error_history;
    }

    // Get the driver for this VM
    vmd = get(vm->get_uid(),vm->get_vmm_mad());

    if ( vmd == 0 )
    {
        goto error_driver;
    }

    // Invoke driver method
    vmd->poll(vid,vm->get_hostname(),vm->get_deploy_id());

    vm->unlock();
    return;

error_history:
    os.str("");
    os << "poll_action, VM has no history";
    goto error_common;

error_driver:
    os.str("");
    os << "poll_action, error getting driver " << vm->get_vmm_mad();
    goto error_common;

error_common:
    Nebula              &ne = Nebula::instance();
    LifeCycleManager *  lcm = ne.get_lcm();

    lcm->trigger(LifeCycleManager::CANCEL_FAILURE, vid);

    vm->log("VMM", Log::ERROR, os);

    vm->unlock();
    return;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void VirtualMachineManager::timer_action()
{
    static int mark = 0;

    VirtualMachine *        vm;
    vector<int>             oids;
    vector<int>::iterator   it;
    int                     rc;
    time_t                  thetime;
    ostringstream           os;

    const VirtualMachineManagerDriver * vmd;

    mark = mark + timer_period;

    if ( mark >= 600 )
    {
        Nebula::log("VMM",Log::INFO,"--Mark--");
        mark = 0;
    }

    rc = vmpool->get_running(oids);

    if ( rc != 0 || oids.empty() )
    {
        return;
    }

    thetime = time(0);

    for ( it = oids.begin(); it != oids.end(); it++ )
    {
        vm = vmpool->get(*it,true);

        if ( vm == 0 || (!vm->hasHistory()))
        {
            os.str("");
            os << "Monitoring VM " << *it << " but it has no history.";
            Nebula::log("VMM", Log::ERROR, os);

            continue;
        }

        if ( (thetime - vm->get_last_poll()) >= poll_period )
        {
            os.str("");

            os << "Monitoring VM " << *it << ".";
            Nebula::log("VMM", Log::INFO, os);

            vm->set_last_poll(thetime);

            vmd = get(vm->get_uid(),vm->get_vmm_mad());

            if ( vmd == 0 )
            {
                vm->unlock();
                continue;
            }

            vmd->poll(*it,vm->get_hostname(),vm->get_deploy_id());

            vmpool->update(vm);
        }

        vm->unlock();
    }
}

/* ************************************************************************** */
/* MAD Loading                                                                */
/* ************************************************************************** */

void VirtualMachineManager::load_mads(int uid)
{
    unsigned int                    i;
    ostringstream                   oss;
    const VectorAttribute *         vattr;
    int                             rc;
    string                          name;
    string                          type;
    VirtualMachineManagerDriver *   vmm_driver = 0;

    oss << "Loading Virtual Machine Manager drivers.";

    Nebula::log("VMM",Log::INFO,oss);

    for(i=0,oss.str("");i<mad_conf.size();i++,oss.str(""),vmm_driver=0)
    {
        vattr = static_cast<const VectorAttribute *>(mad_conf[i]);

        name  = vattr->vector_value("NAME");
        type  = vattr->vector_value("TYPE");

        transform (type.begin(),type.end(),type.begin(),(int(*)(int))toupper);

        oss << "\tLoading driver: " << name << " (" << type << ")";

        Nebula::log("VMM", Log::INFO, oss);

        if ( type == "XEN" )
        {
            vmm_driver = new XenDriver(uid, vattr->value(),(uid != 0),vmpool);
        }
        else if ( type == "KVM" )
        {
            vmm_driver = new LibVirtDriver(uid, vattr->value(),
                                           (uid != 0),vmpool,"kvm");
        }
        else if ( type == "XML" )
        {
            vmm_driver = new XMLDriver(uid, vattr->value(),(uid != 0),vmpool);
        }
        else
        {
            oss.str("");
            oss << "\tUnknown driver type: " << type;

            Nebula::log("VMM",Log::ERROR,oss);

            continue;
        }

        rc = add(vmm_driver);

        if ( rc == 0 )
        {
            oss.str("");
            oss << "\tDriver " << name << " loaded.";

            Nebula::log("VMM",Log::INFO,oss);
        }

    }
}
