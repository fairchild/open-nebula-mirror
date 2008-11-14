
require 'optparse'
require 'pp'

class CommandParse
    
    COMMANDS_HELP=<<-EOT
Commands:

EOT

    USAGE_BANNER=<<-EOT
Usage:
    onevm [<options>] <command> [<parameters>]

Options:
EOT

    ONE_VERSION=<<-EOT
OpenNebula release 1.0 (2008/07/24)
Copyright 2002-2008, Distributed Systems Architecture Group, Universidad
Complutense de Madrid (dsa-research.org)

EOT
    
    def initialize
        @options=Hash.new
    
        @cmdparse=OptionParser.new do |opts|
            opts.banner=text_banner
            
            opts.on("-l x,y,z", "--list x,y,z", Array, 
                    "Selects columns to display with list", "command") do |o|
                @options[:list]=o.collect {|c| c.to_sym }
            end
            
            opts.on("--list-columns", "Information about the columns available",
                    "to display, order or filter") do |o|
                puts list_options
                exit
            end
            
            opts.on("-o x,y,z", "--order x,y,z", Array, 
                    "Order by these columns, column starting",
                    "with - means decreasing order") do |o|
                @options[:order]=o
            end
            
            opts.on("-f x,y,z", "--filter x,y,z", Array,
                    "Filter data. An array is specified", "with column=value pairs.") do |o|
                @options[:filter]=Hash.new
                o.each {|i|
                    k,v=i.split('=')
                    @options[:filter][k]=v
                }
            end
            
            opts.on("-d seconds", "--delay seconds", Integer,
                    "Sets the delay in seconds for top", "command") do |o|
                @options[:delay]=o
            end
            
            opts.on_tail("-h", "--help", "Shows this help message") do |o|
                print_help
                exit
            end
            
            opts.on_tail("-v", "--version", 
                        "Shows version and copyright information") do |o|
                puts text_version
                exit
            end
        end
    end

    def parse(args)
        begin
            @cmdparse.parse!(args)
        rescue => e
            puts e.message
            exit -1
        end
    end
    
    def options
        @options
    end
    
    def print_help
        puts @cmdparse
        puts
        puts text_commands
    end
    
    def text_commands
        COMMANDS_HELP
    end
    
    def text_command_name
        "onevm"
    end
    
    def text_banner
        USAGE_BANNER.gsub("onevm", text_command_name)
    end
    
    def text_version
        ONE_VERSION
    end
    
    def list_options
        "<list options>\n\n"
    end
end
