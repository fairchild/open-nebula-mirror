
require 'pp'
require 'open3'

class TMPlugin < Hash
    def set(command, script)
        self[command]=script
    end
    
    def execute(command, *args)
        # Command is not defined
        return nil if !self[command]
        
        cmd=[self[command], *args].join(" ")
        exec_local_command(cmd)
    end
    
    private
    
    def exec_local_command(command)
        cmd="#{command} ; echo ExitCode: $? 1>&2"
        stderr=""
        std=Open3.popen3(cmd) {|stdin, stdout, stderr_|
            while !stdout.eof?
                STDERR.puts stdout.readline
                STDERR.flush
            end
            stderr_.read
        }
    end
end

class TMScript
    attr_accessor :lines
    
    def initialize(script_text)
        @lines=Array.new
        parse(script_text)
    end
    
    def parse(script_text)
        script_text.each_line {|line|
            # skip if the line is commented
            next if line.match(/^\s*#/)
            # skip if the line is empty
            next if line.match(/^\s*$/)
            
            command=line.split(" ")
            command[0].upcase!
            @lines<< command
        }
    end
    
    def execute(plugin)
        result=@lines.each {|line|
            res=plugin.execute(*line)
            if !res
                puts "COMMAND not found for: #{line.join(" ")}."
                res=[false, "COMMAND not found for: #{line.join(" ")}."]
            else
                puts "STDERR:"
                puts res
                res=parse_output(res)
            end
            
            # do not continue if command failed
            break res if !res[0]
        }
        
        pp result
        
    end
    
    def parse_output(err)
        exit_code=get_exit_code(err)
        if exit_code==0
            [true, ""]
        else
            [false, get_error_message(err)]
        end
    end
    
    # Gets exit code from STDERR
    def get_exit_code(str)
        tmp=str.scan(/^ExitCode: (\d*)$/)
        return nil if !tmp[0]
        tmp[0][0].to_i
    end
    
    def get_error_message(str)
        tmp=str.scan(/^ERROR MESSAGE --8<------\n(.*?)ERROR MESSAGE ------>8--$/m)
        return "Error message not available" if !tmp[0]
        tmp[0][0].strip
    end
end


script_text="

CLONE localhost:/tmp/source.img ursa:/tmp/one_jfontan/0/hda.img


CLONE localhost:/tmp/source.img ursa:/tmp/one_jfontan/1/hda.img

"

plugin=TMPlugin.new
plugin["CLONE"]="./tm_clone.sh"

scr=TMScript.new(script_text)
pp scr.lines


scr.execute(plugin)

