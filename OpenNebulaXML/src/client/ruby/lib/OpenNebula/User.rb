require 'OpenNebula/Pool'

module OpenNebula
    class User < PoolElement
        # ---------------------------------------------------------------------
        # Constants and Class Methods
        # ---------------------------------------------------------------------
        USER_METHODS = {
            :info     => "user.info",
            :allocate => "user.allocate",
            :delete   => "user.delete"
        }

        # Creates a User description with just its identifier
        # this method should be used to create plain User objects.
        # +id+ the id of the user
        #
        # Example:
        #   user = User.new(User.build_xml(3),rpc_client)
        #
        def User.build_xml(pe_id=nil)
            if pe_id
                user_xml = "<USER><UID>#{pe_id}</UID></USER>"
            else
                user_xml = "<USER></USER>"
            end

            REXML::Document.new(user_xml).root
        end

        # ---------------------------------------------------------------------
        # Class constructor
        # ---------------------------------------------------------------------
        def initialize(xml, client)
            super(xml,client)

            @client = client
            @pe_id  = xml.elements['UID'].text.to_i if xml.elements['UID']
        end

        # ---------------------------------------------------------------------
        # XML-RPC Methods for the User Object
        # ---------------------------------------------------------------------
        def info()
            super(USER_METHODS[:info])
        end

        def allocate(username, password)
            super(USER_METHODS[:allocate], username, password)
        end

        def delete()
            super(USER_METHODS[:delete])
        end
    end
end
