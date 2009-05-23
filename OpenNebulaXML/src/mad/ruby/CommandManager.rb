# --------------------------------------------------------------------------
# Copyright 2002-2009 GridWay Team, Distributed Systems Architecture
# Group, Universidad Complutense de Madrid
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# --------------------------------------------------------------------------

require 'pp'
require 'open3'
require 'stringio'

# Generic command executor that holds the code shared by all the command
# executors.
#
# Properties:
#
# * +code+: integer holding the exit code. Read-only
# * +stdout+: string of the standard output. Read-only
# * +stderr+: string of the standard error. Read-only
# * +command+: command to execute. Read-only
class GenericCommand
    attr_reader :code, :stdout, :stderr, :command

    # Creates a command and runs it
    def self.run(command, logger=nil, stdin=nil)
        cmd = self.new(command, logger, stdin)
        cmd.run
        cmd
    end

    # Creates the new command:
    # +command+: string with the command to be executed
    # +logger+: proc that takes a message parameter and logs it
    def initialize(command, logger=nil, stdin=nil)
        @command = command
        @logger  = logger
        @stdin   = stdin
    end

    # Sends a log message to the logger proc
    def log(message)
        @logger.call(message) if @logger
    end

    # Runs the command
    def run
        std = execute

        # Close standard IO descriptors
        if @stdin
            std[0] << @stdin
            std[0].flush
        end
        std[0].close if !std[0].closed?

        @stdout=std[1].read
        std[1].close if !std[1].closed?

        @stderr=std[2].read
        std[2].close if !std[2].closed?

        @code=get_exit_code(@stderr)

        if @code!=0
            log("Command execution fail: #{command}")
            log("STDERR follows.")
            log(@stderr)
        end

        return @code
    end

private

    # Gets exit code from STDERR
    def get_exit_code(str)
        tmp=str.scan(/^ExitCode: (\d*)$/)
        return nil if !tmp[0]
        tmp[0][0].to_i
    end

    # Low level command execution. This method has to be redefined
    # for each kind of command execution. Returns an array with
    # +stdin+, +stdout+ and +stderr+ handlers of the command execution.
    def execute
        puts "About to execute \"#{@command}\""
        [StringIO.new, StringIO.new, StringIO.new]
    end

end

# Executes commands in the machine where it is called. See documentation
# for GenericCommand
class LocalCommand < GenericCommand
private

    def execute
        Open3.popen3("#{command} ; echo ExitCode: $? 1>&2")
    end
end

# Executes commands in a remote machine ussing ssh. See documentation
# for GenericCommand
class SSHCommand < GenericCommand
    attr_accessor :host

    # Creates a command and runs it
    def self.run(command, host, logger=nil, stdin=nil)
        cmd=self.new(command, host, logger, stdin)
        cmd.run
        cmd
    end

    # This one takes another parameter. +host+ is the machine
    # where the command is going to be executed
    def initialize(command, host, logger=nil, stdin=nil)
        @host=host
        super(command, logger, stdin)
    end

private

    def execute
        if @stdin
            Open3.popen3("ssh #{@host} #{@command} ; echo ExitCode: $? 1>&2")
        else
            Open3.popen3("ssh -n #{@host} #{@command} ; echo ExitCode: $? 1>&2")
        end
    end
end


if $0 == __FILE__

    command=GenericCommand.run("uname -a")
    puts command.stderr

    local_command=LocalCommand.run("uname -a")
    puts "STDOUT:"
    puts local_command.stdout
    puts
    puts "STDERR:"
    puts local_command.stderr

    ssh_command=SSHCommand.run("uname -a", "localhost")
    puts "STDOUT:"
    puts ssh_command.stdout
    puts
    puts "STDERR:"
    puts ssh_command.stderr

    fd  = File.new("/etc/passwd")
    str = String.new
    fd.each {|line| str << line}
    fd.close

    ssh_in = SSHCommand.run("cat > /tmp/test","localhost",nil,str)
end
