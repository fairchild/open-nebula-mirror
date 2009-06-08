

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
                @one_endpoint=endpoint
            elsif ENV["ONE_XMLRPC"]
                @one_endpoint=ENV["ONE_XMLRPC"]
            else
                @one_endpoint="http://localhost:2633/RPC2"
            end
        end

        def call(action, *args)

            server=XMLRPC::Client.new2(@one_endpoint)
            server.set_parser(XMLRPC::XMLParser::XMLStreamParser.new)

            begin
                response = server.call("one."+action, @one_auth, *args)
                
                if response.length < 2
                    Error.new("Wrong number of arguments in XML-RPC response")
                elsif response[0] == false
                    Error.new(response[1])
                else
                   response[1..-1] 
                end
            rescue Exception => e
                Error.new(e.message) 
            end
        end
    end

end


