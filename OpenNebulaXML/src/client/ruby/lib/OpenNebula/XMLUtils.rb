
module OpenNebula

    begin
        require 'nokogiri'
        NOKOGIRI=true
    rescue LoadError
        NOKOGIRI=false
    end

    ###########################################################################
    # The XMLUtilsElement module provides an abstraction of the underlying
    # XML parser engine. It provides XML-related methods for the Pool Elements  
    ###########################################################################
    module XMLUtilsElement
        # Initialize a XML document for the element
        # xml:: _String_ the XML document of the object
        # root_element:: _String_ Base xml element
        # [return] _XML_ object for the underlying XML engine
        def self.initialize_xml(xml, root_element)
            if NOKOGIRI
                Nokogiri::XML(xml).xpath("/#{root_element}")
            else
                REXML::Document.new(xml).root
            end
        end

        # Extract an element from the XML description of the PoolElement.
        # key::_String_ The name of the element
        # [return] _String_ the value of the element 
        # Examples:
        #   ['VID'] # gets VM id
        #   ['HISTORY/HOSTNAME'] # get the hostname from the history
        def [](key)
            if NOKOGIRI
                element=@xml.xpath(key.to_s.upcase)
            else
                element=@xml.elements[key.to_s.upcase]
            end
            
            if element 
                element.text
            end
        end
        
        def template_str
            if NOKOGIRI
                xml_template=@xml.xpath('TEMPLATE').to_s
                rexml=REXML::Document.new(xml_template).root
            else
                rexml=@xml.elements['TEMPLATE']
            end
            
            str=rexml.collect {|n|
                if n.class==REXML::Element
                    str_line=""
                    if n.has_elements?
                        str_line<<n.name+"=[\n"
                        str_line<<n.collect {|n2|
                            if n2.class==REXML::Element
                                '  '+n2.name+"="+n2.text
                            end
                        }.compact.join(",\n")
                        str_line<<" ]"
                    else
                        str_line<<n.name+"="+n.text
                    end
                    str_line
                end
            }.compact.join("\n")
            
            str
        end
    end
        
    ###########################################################################
    # The XMLUtilsPool module provides an abstraction of the underlying
    # XML parser engine. It provides XML-related methods for the Pools  
    ###########################################################################
    module XMLUtilsPool

        #Initialize a XML document for the element
        #xml:: _String_ the XML document of the object
        #[return] _XML_ object for the underlying XML engine
        def initialize_xml(xml)
            if NOKOGIRI
                Nokogiri::XML(xml).xpath("/#{@pool_name}")
            else
                xml=REXML::Document.new(xml).root
            end
        end
        
        #Executes the given block for each element of the Pool
        #block:: _Block_ 
        def each_element(block)
            if NOKOGIRI
                @xml.xpath(
                    "#{@element_name}").each {|pelem|
                    block.call self.factory(pelem)
                }
            else
                @xml.elements.each(
                    "#{@element_name}") {|pelem|
                    block.call self.factory(pelem)
                }
            end
        end
    end
end

