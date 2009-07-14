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



import java.io.*;

import com.vmware.vim.*;
import com.vmware.apputils.*;
import com.vmware.apputils.vim.*;

class OneVmmVmware extends Thread 
{
    private String[]     arguments;
    OperationsOverVM     oVM;
    DeployVM             dVM;

    boolean              debug;

    // Helpers from VI samples
    static  AppUtil cb = null;
    
    public static void main(String[] args) 
    { 
        boolean debug_flag;
        
        // first, make redirection
        PrintStream stdout = System.out;                                       
        PrintStream stderr = System.err;
      
        System.setOut(stderr);
        System.setErr(stdout);
        
        if (System.getProperty("debug").equals("1"))
        {
            debug_flag=true;
        }
        else
        {
            debug_flag=false;
        }

        OneVmmVmware omv = new OneVmmVmware(args, debug_flag);
        omv.loop();
    }

    // Constructor
    OneVmmVmware(String[] args, boolean _debug) 
    {
        debug     = _debug;
        
        arguments = args;
    }

    protected void finalize() throws Throwable
    {	
		cb.disConnect();
    }
    

    // Main loop, threaded
    void loop() 
    {
        String  str     = null;
        String  action  = null;
        String  vid_str = null;
		String  hostName;
		String  fileName;
        boolean fin     = false;
        
        BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

        while (!fin) 
	    {
            action   = null;
            vid_str  = null;
            hostName = null;
            fileName = null;
	        
            // Read a line and parse it
            try
            {
                str = in.readLine();
            }
            catch (IOException e)
            {
                String message = e.getMessage().replace('\n', ' ');

                synchronized (System.err)
                {
                    System.err.println(action + " FAILURE " + vid_str + " " + message);
                }
            }

            String str_split[] = str.split(" ", 5);
            
            action    = str_split[0].toUpperCase();

            // Perform the action
            if (action.equals("INIT"))
            {
                init();
            }
            else if (action.equals("FINALIZE"))
            {
                finalize_mad();
                fin = true;
            } 
            else 
            {
                if (action.equals("DEPLOY"))
                {                           
                    if (str_split.length != 5)
                    {   
                       System.out.println("FAILURE Wrong number of arguments for DEPLOY action. Number args = [" +
                                              str_split.length + "].");
                       synchronized (System.err)
                       {
                           System.err.println(action + " FAILURE " + vid_str); 
                           continue;
                       }      
                    }
                    else
                    {
                        vid_str        = str_split[1];
                        hostName       = str_split[2];  
                        fileName       = str_split[3];
                                          
                        try
                        {   
                            fileName = fileName.replace("/images", "");

                            // let's read the XML file and extract needed info
                            ParseXML pXML = new ParseXML(fileName);

                            // First, register the VM
                            dVM = new DeployVM(arguments, 
                                                        hostName, 
                                                        vid_str, 
                                                        pXML,
                                                        System.getProperty("datastore"),
                                                        System.getProperty("datacenter"));

                            if(!dVM.registerVirtualMachine())
                            {
                                // We will skip this error, it may be pre-registered
                            }

                            // Now, proceed with the reconfiguration
                            
                            if(!dVM.shapeVM())
                            {
                                // Will try and deregister VM
                                try
                                {
                                    oVM = new OperationsOverVM(arguments,hostName);
                                    String vmName = pXML.getName() + "-" + vid_str;
                                    oVM.deregisterVM(vmName);
                                }
                                catch(Exception e){}

                                throw new Exception("Error reconfiguring VM (" + pXML.getName() + ").");
                            }

                            try
                            {
                                oVM = new OperationsOverVM(arguments,hostName);
                            }
                            catch(Exception e)
                            {
                                synchronized (System.err)
                                {
                                    System.err.println(action + " FAILURE " + vid_str + " Failed connection to host " +
                                                       hostName +". Reason: " + e.getMessage());
                                }
                                continue;
                            }
                            
                            if(!oVM.powerOn(pXML.getName() + "-" + vid_str))
                            {
                                // Will try and deregister VM
                                try
                                {
                                    oVM = new OperationsOverVM(arguments,hostName);
                                    String vmName = pXML.getName() + "-" + vid_str;
                                    oVM.deregisterVM(vmName);
                                }
                                catch(Exception e){}
                                throw new Exception("Error powering on VM(" + pXML.getName() + ").");
                            }
                            
                            synchronized (System.err)
                            {
                                 System.err.println("DEPLOY SUCCESS " + vid_str + " " + pXML.getName() + "-" + vid_str);
                            }
                            
                            continue;
                         
                         }
                         catch(Exception e)
                         {
                             System.out.println("Failed deploying VM " + vid_str + " into " + hostName + 
                                                ".Reason:" + e.getMessage());
                             if(debug)
                             {
                                 e.printStackTrace(); 
                             }
                        
                             synchronized (System.err)
                             {
                                 System.err.println("DEPLOY FAILURE " + vid_str + " Failed deploying VM in host " + 
                                                    hostName + ".");
                             }
                         } // catch
           		    } // else if (str_split.length != 4)
                 } // if (action.equals("DEPLOY"))
                 
                 if (action.equals("SHUTDOWN") || action.equals("CANCEL"))
                 {                           
                     if (str_split.length < 3 )
                     {  
                         System.out.println("FAILURE Wrong number of arguments for " + action + 
                                           " action. Number args = [" +
                                           str_split.length + "].");
                                           
                        synchronized (System.err)
                        {
                            System.err.println(action + " FAILURE " + vid_str); 
                            continue;
                        }
                     }
                     else
                     {
                         
                         vid_str        = str_split[1];
                         hostName       = str_split[2];  
                         String vmName  = str_split[3];
                         
                         try
                         {
                             oVM = new OperationsOverVM(arguments,hostName);
                         }
                         catch(Exception e)
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " FAILURE " + vid_str + " Failed connection to host " +
                                                    hostName +". Reason: " + e.getMessage());
                             }
                             continue;
                         }
                         
                         if(!oVM.powerOff(vmName))
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " FAILURE " + vid_str + " Failed shutdown VM in host " + 
                                                    hostName);
                             }
                         }

                         if(!oVM.deregisterVM(vmName))
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " FAILURE " + vid_str + " Failed deregistering of " +vmName
                                                    + " in host " + hostName +".");
                             }
                             continue;
                         }
                         else
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " SUCCESS " + vid_str);                             
                             }
                         }
                      }
                      
                      continue;
                 } // if (action.equals("SHUTDOWN or CANCEL"))
                 
                 if (action.equals("SAVE"))
                 {                           
                     if (str_split.length < 5)
                     {  
                        System.out.println("FAILURE Wrong number of arguments for SAVE action. Number args = [" +
                                           str_split.length + "].");
                                           
                        synchronized (System.err)
                        {
                            System.err.println(action + " FAILURE " + vid_str); 
                            continue;
                        }
                     }
                     else
                     {              
                         vid_str               = str_split[1];
                         hostName              = str_split[2];  
                         String vmName         = str_split[3];
                         String checkpointName = str_split[4];
                         
                         try
                         {
                             oVM = new OperationsOverVM(arguments,hostName);
                         }
                         catch(Exception e)
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " FAILURE " + vid_str + " Failed connection to host " +
                                                    hostName +". Reason: " + e.getMessage());
                             }
                             continue;
                         }
                         
                         if(!oVM.save(vmName))
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " FAILURE " + vid_str + " Failed suspending VM in host " + 
                                                    hostName);
                             }
                             continue;
                         }

                         if(!oVM.deregisterVM(vmName))
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " FAILURE " + vid_str + " Failed deregistering of " +vmName
                                                    + " in host " + hostName +".");
                             }
                             continue;
                         }
                         else
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " SUCCESS " + vid_str);                             
                             }
                         }
                         
                         continue;
                      }
                 } // if (action.equals("SAVE"))
                 
                 if (action.equals("CHECKPOINT"))
                 {       
                     if (str_split.length < 4)
                     {  
                        System.out.println("FAILURE Wrong number of arguments for CHECKPOINT action. Number args = [" +
                                           str_split.length + "].");
                        synchronized (System.err)
                        {
                            System.err.println(action + " FAILURE " + vid_str); 
                            continue;
                        }
                     }
                     else 
                     {              
                         vid_str               = str_split[1];
                         hostName              = str_split[2];  
                         String vmName         = str_split[3];
                         String checkpointName = str_split[4];
                         
                         try
                         {
                             oVM = new OperationsOverVM(arguments,hostName);
                         }
                         catch(Exception e)
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " FAILURE " + vid_str + " Failed connection to host " +
                                                    hostName +". Reason: " + e.getMessage());
                             }
                             continue;
                         }
                         
                         if(!oVM.createCheckpoint(vmName))
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " FAILURE " + vid_str + " Failed suspending VM in host " + 
                                                    hostName);
                             }
                         }
                         else
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " SUCCESS " + vid_str);                             
                             }
                         }
                         
                         continue;
                      }
                 } // if (action.equals("CHECKPOINT"))
                 
                 if (action.equals("RESTORE"))
                 {       
                     if (str_split.length < 4)
                     {  
                        System.out.println("FAILURE Wrong number of arguments for RESTORE " + 
                                           "action. Number args = [" + str_split.length + "].");
                        synchronized (System.err)
                        {
                            System.err.println(action + " FAILURE " + vid_str); 
                            continue;
                        }
                     }
                     else 
                     {              
                         vid_str               = str_split[1];
                         hostName              = str_split[2];  
                         String vmName         = str_split[3];
                         boolean result;
                         
                         try
                         {
                             oVM = new OperationsOverVM(arguments,hostName);

                             dVM = new DeployVM(arguments, 
                                                hostName, 
                                                vmName,
                                                System.getProperty("datastore"),
                                                System.getProperty("datacenter"));
                             
                             if(!dVM.registerVirtualMachine())
                             {
                                 // We will skip this error, it may be pre-registered
                             }
                         }
                         catch(Exception e)
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " FAILURE " + vid_str + " Failed connection to host " +
                                                    hostName +". Reason: " + e.getMessage());
                                 continue;
                             }
                         }
                         
                         if(!oVM.restoreCheckpoint(vmName))
                         {
                             synchronized (System.err)
                             {
                                 System.err.println(action + " FAILURE " + vid_str + " Failed restoring VM in host " + 
                                                    hostName);
                             }
                         }
                         else
                         {
                             try
                             {
                                 if(!oVM.powerOn(vmName))
                                 {
                                      System.err.println(action + " FAILURE " + vid_str + " Failed restoring VM in host " + 
                                                        hostName);
                                 }
                                 else
                                 {
                                     synchronized (System.err)
                                     {
                                         System.err.println(action + " SUCCESS " + vid_str);                             
                                     }
                                 }
                             }
                             catch(Exception e)
                             {
                                 synchronized (System.err)
                                 {
                                     System.err.println(action + " FAILURE " + vid_str + " Failed connection to host " +
                                                        hostName +". Reason: " + e.getMessage());
                                     continue;
                                 }
                              }
                     
                         }

                         continue;
                     }
                 } // if (action.equals("RESTORE"))
                 
                 if (action.equals("MIGRATE"))
                 {      
                     synchronized (System.err)
                     {
                         System.err.println(action + " FAILURE " + vid_str + " Action not implemented."); 
                         continue;
                     }
                 } // if (action.equals("MIGRATE"))      
                 
                 if (action.equals("POLL"))
                 {      
                     if (str_split.length < 4)
                     {
                         System.out.println("FAILURE Wrong number of arguments for POLL " + 
                                            "action. Number args = [" + str_split.length + "].");
                         synchronized (System.err)
                         {
                             System.err.println(action + " FAILURE " + vid_str); 
                             continue;
                         }
                     }
                     else
                     {
                         vid_str               = str_split[1];
                         hostName              = str_split[2];  
                         String vmName         = str_split[3];
                         
                         String                pollInfo;
                         
                         try
                         {  

                             String[] argsWithHost = new String[arguments.length+2];

                             for(int i=0;i<arguments.length;i++)
                             {
                                 argsWithHost[i] = arguments[i];
                             }

                             argsWithHost[arguments.length]      = "--url";
                             //argsWithHost[arguments.length + 1 ] = "https://" + hostName + ":443/sdk";

                             argsWithHost[arguments.length + 1 ] = "https://localhost:8008/sdk";
                            
                             GetProperty gPHost = new GetProperty(argsWithHost, "HostSystem", hostName);
                             GetProperty gpVM   = new GetProperty(argsWithHost, "VirtualMachine", vmName);

                             String hostCPUMhz = gPHost.getObjectProperty("summary.hardware.cpuMhz").toString();
  
                             String vmCPUMhz = 
                                  gpVM.getObjectProperty("summary.quickStats.overallCpuUsage").toString();       
                             
                             String vmMEMMb  = 
                                  gpVM.getObjectProperty("summary.quickStats.guestMemoryUsage").toString();

                                   
                             int hostCPUMhz_i = Integer.parseInt(hostCPUMhz);      
                             int vmCPUMhz_i   = Integer.parseInt(vmCPUMhz);      
                             int vmCPUperc    = (vmCPUMhz_i / hostCPUMhz_i) * 100;
                             
                             pollInfo = "STATE=a USEDMEMORY=" + vmMEMMb + " USEDCPU=" + vmCPUperc;
                             
                         }
                         catch(Exception e)
                         {                             
                             pollInfo = "STATE=-";
                         }

                         synchronized (System.err)
                         {
                             System.err.println(action + " SUCCESS " + vid_str + " " + pollInfo);                             
                         }
                        
                         continue;
                     }                
                 } // if (action.equals("POLL"))     
             } //  else if (action.equals("FINALIZE"))
        } // while(!fin)
    } // loop

    void init() 
    {
        // Nothing to do here
        synchronized(System.err)
        {
            System.err.println("INIT SUCCESS");
        }
    }

    void finalize_mad() 
    {
        // Nothing to do here
        synchronized(System.err)
        {
            System.err.println("FINALIZE SUCCESS");
        }
    }
}

