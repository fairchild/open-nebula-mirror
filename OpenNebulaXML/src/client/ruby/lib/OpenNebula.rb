

begin # require 'rubygems'
    require 'rubygems'
rescue Exception
end
require 'xmlrpc/client'
require 'pp'

require 'crack'

require 'OpenNebula/VirtualMachine'

module OpenNebula

    def self.connection
        if @connection
            @connection
        else
            @connection=Client.new
        end
    end

    def self.call(*args)
        self.connection.call(*args)
    end

    # Server class. This is the one that makes xml-rpc calls.
    class Client
        def initialize(endpoint=nil)
            if endpoint
                one_endpoint=endpoint
            elsif ENV["ONE_XMLRPC"]
                one_endpoint=ENV["ONE_XMLRPC"]
            else
                one_endpoint="http://localhost:2633/RPC2"
            end
            @server=XMLRPC::Client.new2(one_endpoint)
        end

        def call(action, *args)
            begin
                response=@server.call("one."+action, "sessionID", *args)
                response<<nil if response.length<2
                response
            rescue Exception => e
                [false, e.message]
            end
        end
    end

end


