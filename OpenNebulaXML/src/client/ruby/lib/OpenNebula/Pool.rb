

begin
    require 'nokogiri'
    NOKOGIRI=true
rescue
    NOKOGIRI=false
end

module OpenNebula
    # The Pool class represents a generic OpenNebula Pool in XML format
    # and provides the basic functionality to handle the Pool elements
    class Pool
        include Enumerable

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

        # ---------------------------------------------------------------------
        # Common XML-RPC Methods for all the Pool Types
        # ---------------------------------------------------------------------

        # Calls to the corresponding info method to retreive the pool
        # representation in XML format
        def info(xml_method,*args)
            rc = @client.call(xml_method,*args)

            if !OpenNebula.is_error?(rc)
                if NOKOGIRI
                    @xml = Nokogiri::XML(rc)
                else
                    @xml = REXML::Document.new(rc)
                end
                rc   = nil
            end
            
            return rc
        end

    public    

        # Iterates over every VirtualNetwork and calls the block with a
        # VirtualNetworkPoolNode
        def each
            if @xml
                if NOKOGIRI
                    @xml.xpath(
                        "/#{@pool_name}/#{@element_name}").each {|pelem|
                        yield self.factory(pelem)
                    }
                else
                    @xml.elements.each(
                        "/#{@pool_name}/#{@element_name}") {|pelem|
                        yield self.factory(pelem)
                    }
                end
            end
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

    protected
        # +node+ is a REXML element that represents the Pool element 
        # +client+ a Client object that represents a XML-RPC connection
        def initialize(node,client)
            @xml    = node
            @client = client

            @pe_id  = nil 
        end

        # ---------------------------------------------------------------------
        # Common XML-RPC Methods for all the Pool Element Types
        # ---------------------------------------------------------------------

        def info(xml_method)
            return Error.new('ID not defined') if !@pe_id

            rc = @client.call(xml_method,@pe_id)

            if !OpenNebula.is_error?(rc)
                doc  = REXML::Document.new(rc)

                @xml = doc.root if doc
                rc   = nil
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
    
        def self.new_with_id(id, client=nil)
            self.new(self.build_xml(id), client)
        end
    
        def id
            @pe_id
        end
        
        def name
            @name
        end

        # Extract an element from the node.
        # +key+ The name of the element
        # Examples:
        #   ['VID'] # gets VM id
        #   ['HISTORY/HOSTNAME'] # get the hostname from the history
        def [](key)
            if NOKOGIRI
                if @xml.xpath(key.to_s.upcase)
                    @xml.xpath(key.to_s.upcase).text
                end
            else
                if @xml.elements[key.to_s.upcase]
                    @xml.elements[key.to_s.upcase].text
                end
            end
        end

        def to_str
            str = ""
            REXML::Formatters::Pretty.new(1).write(@xml,str)
            
            return str 
        end
    end
end
