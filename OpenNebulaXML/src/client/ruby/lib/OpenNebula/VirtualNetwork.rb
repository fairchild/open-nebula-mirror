
module OpenNebula

    class VirtualNetworkPool
        # ---------------------------------------------------------------------
        # Constants and Class attribute accessors
        # ---------------------------------------------------------------------
        VN_POOL_METHODS = {
            :info => "vnpool.info"
        }

        attr_reader :pool_xml

        # ---------------------------------------------------------------------
        # Class constructor
        # ---------------------------------------------------------------------
        def initialize(user_id=0, secret=nil)
            @user_id  = user_id
            @pool_xml = nil

            @client   = OpenNebula::Client.new(secret); #TODO Check for Errors
        end

        # ---------------------------------------------------------------------
        # XML-RPC Methods for the Virtual Network Object
        # ---------------------------------------------------------------------
        def info()
            rc = @client.call(VN_POOL_METHODS[:info],@user_id)

            if OpenNebula.is_error?(rc)
                @pool_xml = nil
            else
                @pool_xml = rc[0];
                rc = nil
            end

            rc
        end
    end
    
    ###########################################################################
    ###########################################################################

    class VirtualNetwork
        # ---------------------------------------------------------------------
        # Constants and Class attribute accessors
        # ---------------------------------------------------------------------
        VN_METHODS = {
            :info     => "vn.info",
            :allocate => "vn.allocate",
            :delete   => "vn.delete"
        }

        attr_reader :vn_xml

        # ---------------------------------------------------------------------
        # Class constructor
        # ---------------------------------------------------------------------
        def initialize(vn_id=nil, secret=nil)
            @vn_id  = vn_id
            @vn_xml = nil

            @client = OpenNebula::Client.new(secret)
        end

        # ---------------------------------------------------------------------
        # XML-RPC Methods for the Virtual Network Object
        # ---------------------------------------------------------------------
        def info()
            return Error.new('Virtual Network ID not defined') if !@vn_id
            rc = @client.call(VN_METHODS[:info],@vn_id)

            if OpenNebula.is_error?(rc)
                @vn_xml = nil
            else
                @vn_xml = rc[0]
                rc = nil
            end

            rc
        end

        def allocate(description)
            rc = @client.call(VN_METHODS[:allocate],description)

            if !OpenNebula.is_error?(rc)
                @vn_id = rc[0]
                rc = nil
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
