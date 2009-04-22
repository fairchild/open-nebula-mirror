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


import com.vmware.vim.*;
import com.vmware.apputils.*;
import com.vmware.apputils.vim.*;

import java.util.*;
import java.io.*;

import java.lang.*;

import java.rmi.RemoteException;


/*
 * Used to register a VM preloaded 
 */

public class DeployVM 
{
    // Helpers from VI samples
    private static  ServiceContent content;    
    private static  AppUtil cb = null;
    private static  VimPortType service;  
    
    private String  datacenterName  ="";
    private String  datastoreName   ="";
    private String  vmName          ="";
    
    ParseXML        pXML;
    
    // VM configuration objects
    VirtualMachineConfigSpec              vmConfigSpec;
    VirtualMachineConfigInfo              vmConfigInfo; 
    ManagedObjectReference                virtualMachine;
    
    com.vmware.vim.ManagedObjectReference hostMor; 
    
    public boolean registerVirtualMachine() throws Exception 
    {     
       boolean registered = false;

       ManagedObjectReference host = null;

       ManagedObjectReference dcmor 
          = cb.getServiceUtil().getDecendentMoRef(null, "Datacenter", getDataCenterName());
          
       com.vmware.vim.ManagedObjectReference vmFolderMor 
          = (com.vmware.vim.ManagedObjectReference)
             cb.getServiceUtil().getDynamicProperty(dcmor,"vmFolder");
    
       // Default Host
       com.vmware.vim.ManagedObjectReference hostFolderMor 
          = (com.vmware.vim.ManagedObjectReference)
             cb.getServiceUtil().getDynamicProperty(dcmor,"hostFolder");
             
       ArrayList hostList = (ArrayList)cb.getServiceUtil().getDecendentMoRefs(
            hostFolderMor,"HostSystem");  
            
       if(hostList.size() < 1) 
       {
          System.out.println("No host found in datacenter to"
                            +" register the Virtual Machine");
          return registered;
       }
       else 
       {
          boolean hostFound = false; 
          for(int i=0; i<hostList.size(); i++) 
          {

             com.vmware.vim.ManagedObjectReference [] datastores 
                = (com.vmware.vim.ManagedObjectReference [])
                   cb.getServiceUtil().getDynamicProperty(hostMor,"datastore");
       
             for(int j=0; j<datastores.length; j++) 
             {
                com.vmware.vim.DatastoreSummary datastoreSummary 
                   = (com.vmware.vim.DatastoreSummary)
                         cb.getServiceUtil().getDynamicProperty(datastores[j],"summary");

                if(datastoreSummary.getName().equalsIgnoreCase(getDataStoreName())) 
                {
                   com.vmware.vim.DatastoreInfo datastoreInfo 
                      = (com.vmware.vim.DatastoreInfo)
                         cb.getServiceUtil().getDynamicProperty(datastores[j],"info");

                   host      = hostMor;
                   hostFound = true;
                   i = hostList.size()+1;
                   j = datastores.length+1;
                }
             }
          }
          if(hostFound) 
          {
    
             String vmxPath = "[" + getDataStoreName() + "]"+getVmName()+"/"+getVmName()+".vmx";
             // Resource Pool
             ManagedObjectReference resourcePool 
                = cb.getServiceUtil().getFirstDecendentMoRef(null, "ResourcePool");
             // Registering The Virtual machine
             ManagedObjectReference taskmor 
                = cb.getConnection().getService().registerVM_Task(
                   vmFolderMor,vmxPath,getVmName(),false,resourcePool,host);

             String result = cb.getServiceUtil().waitForTask(taskmor);
             if (result.equalsIgnoreCase("Sucess")) 
             {
                registered = true;
             }
             else 
             {
                System.out.println("Some Exception While Registering The VM");
                registered = false;
             }
             return registered;
          }
          else 
          {
             System.out.println("No host in datacenter got the"
                               +" specified datastore and free space");
             return registered;
          }
       }
    }
    
    /**
     * Gets the name of the VM
     * @returns name of the VM
     */   
    private String getVmName()
    {
        return vmName;
    }


    /**
     * Gets the name of the datacenter
     * @returns name of the datacenter
     */   
    private String getDataCenterName()
    {
        return datacenterName;
    }
    
    /**
     * Gets the name of the datastore
     * @returns name of the datastore
     */    
    private String getDataStoreName()
    {
        return datastoreName;
    }
    
    private boolean shapeVM() throws Exception 
    {
         
         virtualMachine 
            = cb.getServiceUtil().getDecendentMoRef(null, "VirtualMachine", vmName);
            
         vmConfigInfo 
            = (VirtualMachineConfigInfo)cb.getServiceUtil().getDynamicProperty(
                 virtualMachine,"config");
         
         vmConfigSpec = new VirtualMachineConfigSpec();
         

         ResourceAllocationInfo raInfo     = new ResourceAllocationInfo();
         SharesInfo             sharesInfo = new SharesInfo();
         
         // Memory
         
         sharesInfo.setLevel(SharesLevel.custom);          
         sharesInfo.setShares(Integer.parseInt(pXML.getMemory()));
         
         raInfo.setShares(sharesInfo);
         
         vmConfigSpec.setMemoryAllocation(raInfo);
         
         // CPU
                      
         sharesInfo = new SharesInfo();
         raInfo     = new ResourceAllocationInfo();
         
         sharesInfo.setLevel(SharesLevel.custom);          
         sharesInfo.setShares(Integer.parseInt(pXML.getCPU()));
         
         raInfo.setShares(sharesInfo);
            
         vmConfigSpec.setCpuAllocation(raInfo);
         
         // DISKs
         
         addDisks();
         
         // Network
         
         configureNetwork();
         
       
         // TODO CD for contextualization  
          
       
         /* if(deviceType.equalsIgnoreCase("cd")) {
            System.out.println("Reconfiguring The Virtual Machine For CD Update "  
                              + cb.get_option("vmname"));                          
            VirtualDeviceConfigSpec cdSpec = getCDDeviceConfigSpec();
            if(cdSpec != null) {
               VirtualDeviceConfigSpec [] cdSpecArray = {cdSpec};                     
               vmConfigSpec.setDeviceChange(cdSpecArray);
            }*/ 

         ManagedObjectReference tmor 
            = cb.getConnection().getService().reconfigVM_Task(
                virtualMachine, vmConfigSpec);
        
        String result = cb.getServiceUtil().waitForTask(tmor);
            
        if(!result.equalsIgnoreCase("sucess")) 
        {
            return false;
         }
         
         return true;
 
      }
      
      
     void addDisks()
     {
         String[] disks = pXML.getDisk();
         
         VirtualDeviceConfigSpec [] vdiskSpecArray = new VirtualDeviceConfigSpec[disks.length];
         
         for(int i=0;i<disks.length;i++)
         {
         
             VirtualDeviceConfigSpec diskSpec = new VirtualDeviceConfigSpec();      
                                   
             
             VirtualDisk disk =  new VirtualDisk();
             VirtualDiskFlatVer2BackingInfo diskfileBacking 
                = new VirtualDiskFlatVer2BackingInfo();    
        
             
             int ckey       = 0;
             int unitNumber = 0;
             
             VirtualDevice [] test = vmConfigInfo.getHardware().getDevice();
             for(int k=0;k<test.length;k++)
             {
                if(test[k].getDeviceInfo().getLabel().equalsIgnoreCase(
                   "SCSI Controller 0"))
                {
                   ckey = test[k].getKey();                                
                }
             }     
             
             unitNumber = test.length + 1;                
             String fileName = "["+datastoreName+"] "+ getVmName()
                             + "/"+ disks[i];
             
             diskfileBacking.setFileName(fileName);
             // TODO make this configurable
             diskfileBacking.setDiskMode("persistent");          
             
             disk.setControllerKey(ckey);
             disk.setUnitNumber(unitNumber);
             
             // TODO does this work
             disk.setBacking(diskfileBacking);
             //int size = 1024 * (Integer.parseInt(cb.get_option("disksize")));
             disk.setCapacityInKB(8388608);
             disk.setKey(-1);
             
             diskSpec.setOperation(VirtualDeviceConfigSpecOperation.add);           
             diskSpec.setFileOperation(VirtualDeviceConfigSpecFileOperation.create);           
             diskSpec.setDevice(disk);
             
             vdiskSpecArray[i]=diskSpec;         
         }
         
         vmConfigSpec.setDeviceChange(vdiskSpecArray);           
     }
     
     void configureNetwork()
     {
         // Firt, let's find out the number of NICs to be removed
         
         VirtualDevice [] test = vmConfigInfo.getHardware().getDevice();
         
         String[] nics = pXML.getNet();
         
         // Lenth of array is #nicsToBeRemoved-#nicsToBeAdded
                     
         VirtualDeviceConfigSpec [] nicSpecArray = new VirtualDeviceConfigSpec[test.length+
                                                                               nics.length];
         
         // Let's remove existing NICs
         
         for(int i=0;i<test.length;i++)
         {
             VirtualDeviceConfigSpec nicSpec = new VirtualDeviceConfigSpec(); 
             VirtualEthernetCard nic;
             
             nicSpec.setOperation(VirtualDeviceConfigSpecOperation.remove);            
             nic             = (VirtualEthernetCard)test[i];
             
             nicSpec.setDevice(nic);
             
             nicSpecArray[i] = nicSpec;
         }
         
         
        // Let's add specified NICs
         
         for(int i=0;i<nics.length;i++)
         {
             VirtualDeviceConfigSpec nicSpec = new VirtualDeviceConfigSpec();
             // TODO make this dynamic?
             String networkName = "one-net"; 
             
             nicSpec.setOperation(VirtualDeviceConfigSpecOperation.add);
             VirtualEthernetCard nic =  new VirtualPCNet32();
             VirtualEthernetCardNetworkBackingInfo nicBacking 
                = new VirtualEthernetCardNetworkBackingInfo();
             nicBacking.setDeviceName(networkName);
             nic.setAddressType(nics[i]); 
             nic.setBacking(nicBacking);
             nic.setKey(4);
             nicSpec.setDevice(nic);
             nicSpecArray[i+test.length] = nicSpec;
         }
     }
     
      
    
    /*
    private String getDataStoreName(int size) throws Exception{
       String dsName = null;
       ManagedObjectReference [] datastores 
          = (ManagedObjectReference [])cb.getServiceUtil().getDynamicProperty(
                _virtualMachine,"datastore");
       for(int i=0; i<datastores.length; i++) {
          DatastoreSummary ds 
             = (DatastoreSummary)cb.getServiceUtil().getDynamicProperty(datastores[i],
                                                                       "summary");
          if(ds.getFreeSpace() > size) {
             dsName = ds.getName();
             i = datastores.length + 1;           
          }
       }
       return dsName;
    }
    */
    
    DeployVM(String[] args, String hostName, String _vmName, ParseXML _pXML) throws Exception
    {
        cb = AppUtil.initialize("RegisterVM", null, args);
        cb.connect();
        
        // TODO get this dynamically
        datastoreName  = "datastore1";
        datacenterName = "ha-datacenter";
        
        vmName = _vmName;
        pXML   = _pXML;

        // Get reference to host
        
        hostMor = cb.getServiceUtil().getDecendentMoRef(null,"HostSystem",
                                                           hostName);
                                                        

        com.vmware.apputils.vim.ServiceConnection sc = cb.getConnection();
        content = sc.getServiceContent();
        service = sc.getService();
    }
    
    protected void finalize() throws Throwable
    {	
		cb.disConnect();
    }
}