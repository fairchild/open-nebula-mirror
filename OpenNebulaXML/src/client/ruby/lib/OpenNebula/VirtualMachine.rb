require 'OpenNebula/Pool'

module OpenNebula
    class VirtualMachine < PoolElement
        # ---------------------------------------------------------------------
        # Constants and Class Methods
        # ---------------------------------------------------------------------
        VM_METHODS = {
            :info     => "vm.info",
            :allocate => "vm.allocate",
            :action   => "vm.action",
            :migrate  => "vm.migrate",
            :deploy   => "vm.deploy"
        }

        # Creates a VirtualMachine description with just its identifier
        # this method should be used to create plain VirtualMachine objects.
        # +id+ the id of the vm
        #
        # Example:
        #   vnet = VirtualMachine.new(VirtualMachine.build_xml(3),rpc_client)
        #
        def VirtualMachine.build_xml(pe_id=nil)
            if pe_id
                vm_xml = "<VM><VID>#{pe_id}</VID></VM>"
            else
                vm_xml = "<VM></VM>"
            end
            
            if NOKOGIRI
                Nokogiri::XML(vm_xml)
            else
                REXML::Document.new(vm_xml).root
            end
        end

        # ---------------------------------------------------------------------
        # Class constructor
        # ---------------------------------------------------------------------
        def initialize(xml, client)
            super(xml,client)

            @client = client
            #@pe_id  = xml.elements['VID'].text.to_i if xml.elements['VID']
            @pe_id = self['VID'].to_i if self['VID']
        end

        # ---------------------------------------------------------------------
        # XML-RPC Methods for the Virtual Network Object
        # ---------------------------------------------------------------------
        def info()
            super(VM_METHODS[:info])
        end

        def allocate(description)
            super(VM_METHODS[:allocate],description)
        end

        def deploy(host_id)
            return Error.new('ID not defined') if !@pe_id

            rc = @client.call(VM_METHODS[:allocate], @pe_id, host_id)
            rc = nil if !OpenNebula.is_error?(rc)
        
            return rc
        end

        def shutdown
            action('shutdown')
        end

        def cancel
            action('cancel')
        end

        def hold
            action('hold')
        end

        def release
            action('release')
        end

        def stop
            action('stop')
        end

        def suspend
            action('suspend')
        end

        def resume
            action('resume')
        end

        def finalize
            action('finalize')
        end

        def restart
            action('restart')
        end

        def migrate(host_id)
            return Error.new('ID not defined') if !@pe_id

            rc = @client.call(VM_METHODS[:migrate], @pe_id, host_id, false)
            rc = nil if !OpenNebula.is_error?(rc)
        
            return rc
        end

        def live_migrate(host_id)
            return Error.new('ID not defined') if !@pe_id

            rc = @client.call(VM_METHODS[:migrate], @pe_id, host_id, true)
            rc = nil if !OpenNebula.is_error?(rc)
            
            return rc
        end

    private
        def action(name)
            return Error.new('ID not defined') if !@pe_id

            rc = @client.call(VM_METHODS[:action], name, @pe_id)
            rc = nil if !OpenNebula.is_error?(rc)

            return rc
        end
    end
end
