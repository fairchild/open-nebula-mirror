

begin # require 'rubygems'
    require 'rubygems'
rescue Exception
end
require 'xmlrpc/client'
require 'digest/sha1'
require 'pp'

require 'crack'

require 'OpenNebula/VirtualMachine'

module OpenNebula

    def self.is_error?(value)
        value.class==OpenNebula::Error
    end

    class Error
        attr_reader :message

        def initialize(message=nil)
            @message=message
        end
    end

    def self.connection
        #if @connection
        #    @connection
        #else
        #    @connection=Client.new
        #end
        Client.new
    end

    def self.call(*args)
        self.connection.call(*args)
    end

    # Server class. This is the one that makes xml-rpc calls.
    class Client
        def initialize(secret=nil, endpoint=nil)
            if secret
                one_secret = secret
            elsif ENV["ONE_AUTH"]
                one_secret = ENV["ONE_AUTH"]
            else
                return -1
            end

            one_secret=~/(\w+):(\w+)/
            @one_auth  = "#{$1}:#{Digest::SHA1.hexdigest($2)}"

            if endpoint
                one_endpoint=endpoint
            elsif ENV["ONE_XMLRPC"]
                one_endpoint=ENV["ONE_XMLRPC"]
            else
                one_endpoint="http://localhost:2633/RPC2"
            end
            @server=XMLRPC::Client.new2(one_endpoint)
            @server.set_parser(XMLRPC::XMLParser::XMLStreamParser.new)
        end

        def call(action, *args)
            begin
                response=@server.call("one."+action, @one_auth, *args)
                response<<nil if response.length<2
                response
            rescue Exception => e
                [false, e.message]
            end
        end
    end

end


