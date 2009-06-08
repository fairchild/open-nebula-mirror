
module OpenNebula

    class VirtualNetworkPool

        VN_POOL_METHODS = {
            :info => "vnpool.info"
        }

        def initialize(user_id=0)
            @user_id  = user_id
            @pool_xml = nil

            @client   = Client.new;
            #TODO Check for Errors
        end

        def get()
            @pool_xml = @client.call(VN_POOL_METHODS[:info],user_id)

            if is_error?(@pool_xml)
                error     = @pool_xml.clone
                @pool_xml = nil
            else
                error = nil
            end

            error
        end
    end
end
