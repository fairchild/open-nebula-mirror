
module OpenNebula
    class VirtualMachinePool
        include Enumerable

        def initialize(user_id=0)
            @user_id=user_id
            @xml=nil
            @client=OpenNebula::Client.new

            response=@client.call('vmpool.info', @user_id)

            if OpenNebula.is_error?(response)
                # TODO: deal with the error
            else
                @xml=response
            end
        end

        # Iterates over every VM and calls the block with a
        # VirtualMachinePoolNode
        def each
            if @xml
                doc=REXML::Document.new(@xml)
                doc.elements.each("/VM_POOL/VM") {|vm|
                    yield OpenNebula::VirtualMachinePoolNode.new(vm)
                }
            end
        end

        # Returns an arry of VirtualMachine id's
        def get_ids
            self.collect {|vm| vm[:vid].to_i }
        end

        # Returns an array of VirtualMachine objects initialized
        # with VM id's. The objects are not populated, you have
        # to call VirtualMachine#refresh to get the information
        # from the server.
        def get_vms
            get_ids.collect {|vid| OpenNebula::VirtualMachine.new(vid) }
        end

        def to_str
            @xml
        end
    end

    # Proxy to easilly get information from xml nodes.
    class VirtualMachinePoolNode
        # +node+ is a REXML node
        def initialize(node)
            @node=node
        end

        # Extract an element from the node:
        #
        #   ['VID'] # gets VM id
        #   ['HISTORY/HOSTNAME'] # get the hostname from the history
        def [](key)
            @node.elements[key.to_s.upcase].text
        end
    end
end



