
module OpenNebula

    # The Pool class represents a generic OpenNebula Pool in XML format
    # and provides the basic functionality to handle the Pool elements
    class Pool
        include Enumerable

        def initialize(pool,element)
            @pool_name    = pool.upcase
            @element_name = element.upcase
        end

        # Iterates over every VirtualNetwork and calls the block with a
        # VirtualNetworkPoolNode
        def each
            if @xml
                doc=REXML::Document.new(@xml)
                doc.elements.each("/#{@pool_name}/#{@element_name}") {|pelem|
                    yield self.factory(pelem)
                }
            end
        end
        
        # Default Factory Method for the Pools
        def factory(element_xml)
            OpenNebula::PoolNode.new(element_xml)
        end
    end
    
    # The PoolElement Class represents a generic element of a Pool in
    # XML format
    class PoolElement
        # +node+ is a REXML element that represents the Pool element 
        def initialize(node)
            @xml = node
        end

        # Extract an element from the node:
        #
        #   ['VID'] # gets VM id
        #   ['HISTORY/HOSTNAME'] # get the hostname from the history
        def [](key)
            @xml.elements[key.to_s.upcase].text
        end
    end

    class VirtualNetworkPool < Pool
        # ---------------------------------------------------------------------
        # Constants and Class attribute accessors
        # ---------------------------------------------------------------------

        VN_POOL_METHODS = {
            :info => "vnpool.info"
        }

        attr_reader :xml

        # ---------------------------------------------------------------------
        # Class constructor & Pool Methods
        # ---------------------------------------------------------------------
        
        # +secret+ is the authentication token used with the OpenNebula core
        # +user_id+ is to refer to a Pool with VirtualNetworks from that user
        def initialize(user_id=0, secret=nil)
            super('VNET_POOL','VNET')

            @user_id  = user_id
            @xml      = nil

            @client = OpenNebula::Client.new(secret); 
        end

        # Default Factory Method for the Pools
        def factory(element_xml)
            OpenNebula::VirtualNetwork.new(element_xml)
        end

        # ---------------------------------------------------------------------
        # XML-RPC Methods for the Virtual Network Object
        # ---------------------------------------------------------------------

        def info()
            rc = @client.call(VN_POOL_METHODS[:info],@user_id)

            if !OpenNebula.is_error?(rc)
                @xml = rc;
                rc   = nil
            end
            
            return rc
        end
    end

    class VirtualNetwork < PoolElement
        # ---------------------------------------------------------------------
        # Constants and Class attribute accessors
        # ---------------------------------------------------------------------
        VN_METHODS = {
            :info     => "vn.info",
            :allocate => "vn.allocate",
            :delete   => "vn.delete"
        }

        # ---------------------------------------------------------------------
        # Class constructor
        # ---------------------------------------------------------------------
        def initialize(xml=nil, vn_id=nil, secret=nil)
            super(xml)
            @vn_id = vn_id

            @client = OpenNebula::Client.new(secret)
        end

        # ---------------------------------------------------------------------
        # XML-RPC Methods for the Virtual Network Object
        # ---------------------------------------------------------------------
        def info()
            return Error.new('Virtual Network ID not defined') if !@vn_id
            rc = @client.call(VN_METHODS[:info],@vn_id)

            if !OpenNebula.is_error?(rc)
                @xml = rc
                rc   = nil
            end

            return rc
        end

        def allocate(description)
            rc = @client.call(VN_METHODS[:allocate],description)

            if !OpenNebula.is_error?(rc)
                @vn_id = rc
                rc     = nil
            end

            return rc
        end

        def delete()
            return Error.new('Virtual Network ID not defined') if !@vn_id

            rc = @client.call(VN_METHODS[:delete],@vn_id)
            rc = nil if !OpenNebula.is_error?(rc)

            return rc
        end
    end
end
