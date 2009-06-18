require 'OpenNebula/Pool'

module OpenNebula
    class Host < PoolElement
        #######################################################################
        # Constants and Class Methods
        #######################################################################
        HOST_METHODS = {
            :info     => "host.info",
            :allocate => "host.allocate",
            :delete   => "host.delete",
            :enable   => "host.enable"
        }

        # Creates a Host description with just its identifier
        # this method should be used to create plain Host objects.
        # +id+ the id of the host
        #
        # Example:
        #   host = Host.new(Host.build_xml(3),rpc_client)
        #
        def Host.build_xml(pe_id=nil)
            if pe_id
                host_xml = "<HOST><HID>#{pe_id}</HID></HOST>"
            else
                host_xml = "<HOST></HOST>"
            end

            initialize_xml(host_xml)
        end

        #######################################################################
        # Class constructor
        #######################################################################
        def initialize(xml, client)
            super(xml,client)

            @client = client
            @pe_id  = self['HID'].to_i if self['HID']
        end

        #######################################################################
        # XML-RPC Methods for the Host
        #######################################################################
        def info()
            super(HOST_METHODS[:info])
        end

        def allocate(hostname,im,vmm,tm)
            super(HOST_METHODS[:allocate],hostname,im,vmm,tm)
        end

        def delete()
            super(HOST_METHODS[:delete])
        end

        def enable()
            set_enabled(true)
        end

        def disable()
            set_enabled(false)
        end

    private
        def set_enabled(enabled)
            return Error.new('ID not defined') if !@pe_id

            rc = @client.call(HOST_METHODS[:enable], @pe_id, enabled)
            rc = nil if !OpenNebula.is_error?(rc)
        
            return rc
        end
    end
end
