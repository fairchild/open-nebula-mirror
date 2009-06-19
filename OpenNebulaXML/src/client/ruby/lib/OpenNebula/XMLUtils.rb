
module OpenNebula

    begin
        require 'nokogiri'
        NOKOGIRI=true
    rescue LoadError
        NOKOGIRI=false
    end

    
    module XMLUtilsElement
        def self.initialize_xml(xml)
            if NOKOGIRI
                Nokogiri::XML(xml)
            else
                REXML::Document.new(xml).root
            end
        end

        def get_element(key)
            if NOKOGIRI
                element=@xml.xpath(key.to_s.upcase)
            else
                element=@xml.elements[key.to_s.upcase]
            end
            
            if element 
                element.text
            end
        end

        def self.initialize_xml(xml)
            puts xml

            if NOKOGIRI
                Nokogiri::XML(xml)
            else
                REXML::Document.new(xml).root
            end
        end
    end
        
    module XMLUtilsPool
        def initialize_xml(xml)
            if NOKOGIRI
                Nokogiri::XML(xml)
            else
                REXML::Document.new(xml).root
            end
        end
        
        def each_element(block)
            if NOKOGIRI
                @xml.xpath(
                    "/#{@pool_name}/#{@element_name}").each {|pelem|
                    block.call self.factory(pelem)
                }
            else
                @xml.elements.each(
                    "/#{@pool_name}/#{@element_name}") {|pelem|
                    block.call self.factory(pelem)
                }
            end
        end
    end
end


