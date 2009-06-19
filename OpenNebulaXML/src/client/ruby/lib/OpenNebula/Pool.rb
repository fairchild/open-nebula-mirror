
module OpenNebula
    # The Pool class represents a generic OpenNebula Pool in XML format
    # and provides the basic functionality to handle the Pool elements
    class Pool
        include Enumerable
        include XMLUtilsPool

    protected

        #+pool+ XML name of the root element
        #+element+ XML name of the Pool elements
        #+client+  a Client object that represents a XML-RPC connection
        def initialize(pool,element,client)
            @pool_name    = pool.upcase
            @element_name = element.upcase

            @client = client 
            @xml    = nil
        end

        # Default Factory Method for the Pools. Each Pool MUST implement the 
        # corresponding factory method
        # +element_xml+ a REXML element describing the pool element  
        def factory(element_xml)
            OpenNebula::PoolElement.new(element_xml,client)
        end

        #######################################################################
        # Common XML-RPC Methods for all the Pool Types
        #######################################################################

        # Calls to the corresponding info method to retreive the pool
        # representation in XML format
        def info(xml_method,*args)
            rc = @client.call(xml_method,*args)

            if !OpenNebula.is_error?(rc)
                @xml=initialize_xml(rc)
                rc   = nil
            end
            
            return rc
        end

    public

        # Iterates over every VirtualNetwork and calls the block with a
        # VirtualNetworkPoolNode
        def each(&block)
            each_element(block) if @xml
        end

        def to_str
            str = ""
            REXML::Formatters::Pretty.new(1).write(@xml,str)

            return str 
        end
    end
    
    # The PoolElement Class represents a generic element of a Pool in
    # XML format
    class PoolElement
        include XMLUtilsElement

    protected
        # +node+ is a REXML element that represents the Pool element 
        # +client+ a Client object that represents a XML-RPC connection
        def initialize(node,client)
            @xml    = node
            @client = client

            if self['ID']
                @pe_id = self['ID'].to_i
            else
                @pe_id  = nil
            end
            @name  = self['NAME'] if self['NAME']
        end

        #######################################################################
        # Common XML-RPC Methods for all the Pool Element Types
        #######################################################################

        def info(xml_method)
            return Error.new('ID not defined') if !@pe_id

            rc = @client.call(xml_method,@pe_id)

            if !OpenNebula.is_error?(rc)
                @xml=initialize_xml(rc)
                rc   = nil
                
                @pe_id = self['ID'].to_i if self['ID']
                @name  = self['NAME'] if self['NAME']
            end

            return rc
        end

        def allocate(xml_method, *args)
            rc = @client.call(xml_method, *args)

            if !OpenNebula.is_error?(rc)
                @pe_id = rc
                rc     = nil
            end

            return rc
        end

        def delete(xml_method)
            return Error.new('ID not defined') if !@pe_id

            rc = @client.call(xml_method,@pe_id)
            rc = nil if !OpenNebula.is_error?(rc)

            return rc
        end

    public
    
        # Creates new element specifying its id
        # id:: identifyier of the element
        # client:: initialized OpenNebula::Client object
        def self.new_with_id(id, client=nil)
            self.new(self.build_xml(id), client)
        end
    
        # Returns element identifier
        def id
            @pe_id
        end
        
        # Returns element name
        def name
            @name
        end

        # Extract an element from the node.
        # +key+ The name of the element
        # Examples:
        #   ['VID'] # gets VM id
        #   ['HISTORY/HOSTNAME'] # get the hostname from the history
        def [](key)
            get_element(key)
        end

        def to_str
            str = ""
            REXML::Formatters::Pretty.new(1).write(@xml,str)
            
            return str 
        end
    end
end
