
module OpenNebula

    class VirtualMachine
        attr_reader :vm_data

        #######################
        # ENUMS AND CONSTANTS #
        #######################
        
        VM_STATE=%w{INIT PENDING HOLD ACTIVE STOPPED SUSPENDED DONE FAILED}
        
        LCM_STATE=%w{LCM_INIT PROLOG BOOT RUNNING MIGRATE SAVE_STOP SAVE_SUSPEND
            SAVE_MIGRATE PROLOG_MIGRATE EPILOG_STOP EPILOG SHUTDOWN CANCEL}

        SHORT_VM_STATES={
            "INIT"      => "init",
            "PENDING"   => "pend",
            "HOLD"      => "hold",
            "ACTIVE"    => "actv",
            "STOPPED"   => "stop",
            "SUSPENDED" => "susp",
            "DONE"      => "done",
            "FAILED"    => "fail"
        }

        SHORT_LCM_STATES={
            "PROLOG"        => "prol",
            "BOOT"          => "boot",
            "RUNNING"       => "runn",
            "MIGRATE"       => "migr",
            "SAVE_STOP"     => "save",
            "SAVE_SUSPEND"  => "save",
            "SAVE_MIGRATE"  => "save",
            "PROLOG_MIGRATE"=> "migr",
            "EPILOG_STOP"   => "epil",
            "EPILOG"        => "epil",
            "SHUTDOWN"      => "shut",
            "CANCEL"        => "shut"
        }
        

        ################
        # CONSTRUCTORS #
        ################

        # Creates a new VM described by template
        def self.create(template)
            res=OpenNebula.call('vm.allocate', template)
            if res[0]
                self.new_id(res[1])
            else
                false
            end
        end

        # Creates a new VirtualMachine object described by xml
        def self.new_xml(xml)
            object=self.new
            object.parse_xml(xml)
        end

        # Gets a VirtualMachine from the server
        def self.new_id(vmid)
            object=self.new
            object.get_info(vmid)
            object
        end
        
        # Initializes an empty VirtualMachine
        def initialize
            @vm_data=Hash.new
            @vm_id=nil
        end

        # Populates a VirtualMachine object with information from
        # server. If vmid is not provided internal vmid will be used,
        # this is useful to refresh data.
        def get_info(vmid=nil)
            vm_to_get=@vm_id
            vm_to_get=vmid if vmid

            res=OpenNebula.call('vm.info', vm_to_get)

            if res[0]
                parse_xml(res[1])
            else
                @vm_data=res[1]
            end

            self
        end

        # Parses the provided xml and modifies information in the
        # object accordingly.
        def parse_xml(xml)
            @vm_data=Crack::XML.parse(xml)['VM']
            @vm_id=@vm_data['VID'].to_i if @vm_data['VID']
            self
        end

        # Returns the id of the VirtualMachine
        def id
            @vm_id
        end
        
        # Returns the VM state of the VirtualMachine (numeric value)
        def state
            @vm_data['STATE'].to_i
        end

        # Returns the VM state of the VirtualMachine (string value)
        def state_str
            VM_STATE[state]
        end

        # Returns the LCM state of the VirtualMachine (numeric value)
        def lcm_state
            @vm_data['LCM_STATE'].to_i
        end

        # Returns the LCM state of the VirtualMachine (string value)
        def lcm_state_str
            LCM_STATE[lcm_state]
        end

        # Returns the short status string for the VirtualMachine
        def status
            short_state_str=SHORT_VM_STATES[state_str]

            if short_state_str=="actv"
                short_state_str=SHORT_LCM_STATES[lcm_state_str]
            end

            short_state_str
        end

        def [](key)
            @vm_data[key]
        end


        ###########
        # ACTIONS #
        ###########

        def deploy(host_id)
            res=OpenNebula.call('vm.deploy', @vm_id, host_id)
        end

        def shutdown
            res=OpenNebula.call('vm.action', 'shutdown', @vm_id)
        end

        def cancel
            res=OpenNebula.call('vm.action', 'cancel', @vm_id)
        end

        def hold
            res=OpenNebula.call('vm.action', 'hold', @vm_id)
        end

        def release
            res=OpenNebula.call('vm.action', 'release', @vm_id)
        end

        def stop
            res=OpenNebula.call('vm.action', 'stop', @vm_id)
        end

        def suspend
            res=OpenNebula.call('vm.action', 'suspend', @vm_id)
        end

        def resume
            res=OpenNebula.call('vm.action', 'resume', @vm_id)
        end

        def delete
            res=OpenNebula.call('vm.action', 'delete', @vm_id)
        end

        def migrate(host_id)
            res=OpenNebula.call('vm.migrate', @vm_id, host_id, false)
        end

        def live_migrate(host_id)
            res=OpenNebula.call('vm.migrate', @vm_id, host_id, true)
        end
    end
end





