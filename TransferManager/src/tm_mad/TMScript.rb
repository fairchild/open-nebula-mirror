
require 'pp'
require 'open3'

=begin rdoc

TMPlugin holds the name of the scripts that will be used for each
TransferManager script command. It is basically a hash where keys
are the names of the commands (uppercase) and contain the path of
the script that will be executed for that command.

It also contains some methods to execute the scripts, get the output
of the script (success/failure, error and log messages). The protocol
for scripts to do so is as follows:

* Log messages will be sent to STDOUT
* The script will return 0 if it succeded or any other value
  if there was a failure
* In case of failure the cause of the error will be written to STDERR
  wrapped by start and end marks as follows:

    ERROR MESSAGE --8<------
    error message for the failure
    ERROR MESSAGE ------>8--

=end
class TMPlugin < Hash
    
    # Sets the script path for the specific +command+
    def set(command, script)
        self[command]=script
    end
    
    # Executes the script associated with the +command+ using
    # specified arguments.
    #
    # Returns:
    # * It will return +nil+ if the +command+ is not defined.
    # * String with stderr output of the string (exit code and
    #   error message in case of failure)
    #
    # Note: the exit code will be written like this:
    #   ExitCode: 0
    def execute(command, *args)
        # Command is not defined
        return nil if !self[command]
        
        # Generates the line to call the script with all the
        # arguments provided.
        cmd=[self[command], *args].join(" ")
        exec_local_command(cmd)
    end
    
    private
    
    # Executes the command, get its exit code and logs every line that
    # comes from stdout. Returns whatever came from stderr.
    def exec_local_command(command)
        cmd="#{command} ; echo ExitCode: $? 1>&2"
        stderr=""
        std=Open3.popen3(cmd) {|stdin, stdout, stderr_|
            # TODO: this should be sent to ONE and not to STDERR
            while !stdout.eof?
                STDERR.puts stdout.readline
                STDERR.flush
            end
            stderr_.read
        }
    end
end

# This class will parse and execute TransferManager scripts.
class TMScript
    attr_accessor :lines
    
    def initialize(script_text)
        @lines=Array.new
        parse_script(script_text)
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
    
    private
    
    def parse_script(script_text)
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

