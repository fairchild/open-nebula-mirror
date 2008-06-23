#!/usr/bin/env ruby

XENTOP_PATH="/usr/sbin/xentop"
XM_PATH="/usr/sbin/xm"

ONE_LOCATION=ENV["ONE_LOCATION"]

if !ONE_LOCATION
	puts "ONE_LOCATION not set"
	exit -1
end

$: << ONE_LOCATION+"/lib/ruby"

require 'pp'

require 'one_mad'
require 'open3'
require 'one_ssh'

class DM < ONEMad
	include SSHActionController
	
	def initialize
		super(5, 4)
		
		# Set log file
		#log_file=File.open("dm.log", "w")
		#set_logger(log_file)
		
		init_actions
	end
	
	def action_init(args)
		send_message("INIT", "SUCCESS")
	end
		
	def action_deploy(args)
		std_action("DEPLOY", "create #{args[3]}", args)
	end
	
	def action_shutdown(args)
		std_action("SHUTDOWN", "shutdown #{args[3]}", args)
	end
	
	def action_cancel(args)
		std_action("CANCEL", "destroy #{args[3]}", args)
	end
	
	def action_checkpoint(args)
		std_action("CHECKPOINT", "save -c #{args[3]} #{args[4]}", args)
	end

	def action_save(args)
		std_action("SAVE", "save #{args[3]} #{args[4]}", args)
	end

	def action_restore(args)
		std_action("RESTORE", "restore #{args[3]}", args)
	end
	
	def action_migrate(args)
		std_action("MIGRATE", "migrate -l #{args[3]} #{args[4]}", args)
	end
	
	def action_poll(args)
		#std=Open3.popen3(
		#	"ssh -n #{args[2]} sudo #{XENTOP_PATH} -bi2 ;"+
		#	" echo ExitCode: $? 1>&2")
		
		action_number=args[1]
		action_host=args[2]
		
		cmd=SSHCommand.new("sudo #{XENTOP_PATH} -bi2")
		cmd.callback=lambda do |a,num|
		
			stdout=a.stdout
			stderr=a.stderr
		
			exit_code=get_exit_code(stderr)
		
			if exit_code!=0
				send_message("POLL", "FAILURE", args[1])
				return nil
			end

			#log("stdout:")
			#log(stdout)
			#log("stderr:")
			#log(stderr)
		
			values=parse_xentop(args[3], stdout)
		
			if !values
				send_message("POLL", "FAILURE", args[1], "Domain not found")
				return nil
			end
		
			info=values.map do |k,v|
				k+"="+v
			end.join(" ")
		
			send_message("POLL", "SUCCESS", args[1], info)
		end # End of callback
		
		action=SSHAction.new(action_number, action_host, cmd)
		send_ssh_action(action_number, action_host, action)
	end
	
	###########################
	# Common action functions #
	###########################
	
	def std_action(name, command, args)
		action_number=args[1]
		action_host=args[2]
		
		cmd=SSHCommand.new("sudo #{XM_PATH} "+command)
		cmd.callback=lambda do |a, num|
			write_response(name, a.stdout, a.stderr, args)
		end
		
		action=SSHAction.new(action_number, action_host, cmd)
		send_ssh_action(action_number, action_host, cmd)
	end
		
	def exec_xm_command(host, command)
		Open3.popen3(
			"ssh -n #{host} sudo /usr/sbin/xm #{command} ;"+
			" echo ExitCode: $? 1>&2")
	end
	
	def write_response(action, stdout, stderr, args)
		exit_code=get_exit_code(stderr)
		
		if exit_code==0
			domain_name=get_domain_name(stdout)
			send_message(action, "SUCCESS", args[1], domain_name)
		else
			error_message=get_error_message(stderr)
			send_message(action, "FAILURE", args[1], error_message)
		end

		#log("stdout:")
		#log(stdout)
		#log("stderr:")
		#log(stderr)
	end
	

	#########################################
	# Get information form xm create output #
	#########################################

	# From STDERR if exit code == 1
	def get_exit_code(str)
		tmp=str.scan(/^ExitCode: (\d*)$/)
		return nil if !tmp[0]
		tmp[0][0].to_i
	end
	
	# From STDERR if exit code == 1
	def get_error_message(str)
		tmp=str.scan(/^Error: (.*)$/)
		return "Unknown error" if !tmp[0]
		tmp[0][0]
	end

	# From STDOUT if exit code == 0
	def get_domain_name(str)
		tmp=str.scan(/^Started domain (.*)$/)
		return nil if !tmp[0]
		tmp[0][0]
	end
	
	
	###############################
	# Get information from xentop #
	###############################
	
	# COLUMNS
	#
	# 00 -> NAME
	# 01 -> STATE
	# 02 -> CPU(sec)
	# 03 -> CPU(%)
	# 04 -> MEM(k) 
	# 05 -> MEM(%)	
	# 06 -> MAXMEM(k) 
	# 07 -> MAXMEM(%) 
	# 08 -> VCPUS 
	# 09 -> NETS 
	# 10 -> NETTX(k) 
	# 11 -> NETRX(k) 
	# 12 -> VBDS   
	# 13 -> VBD_OO	 
	# 14 -> VBD_RD	 
	# 15 -> VBD_WR 
	# 16 -> SSID
	
	ColumnNames=[
		"name", "STATE", "cpu_sec", "USEDCPU", "USEDMEMORY", "mem_percent", 
		"maxmem_k", "maxmem_percent", "vcpus", "nets", "NETTX", "NETRX",
		"vdbs", "vdb_oo", "vdb_rd", "vdb_wr", "ssid"
	]
	
	ColumnsToPrint=[ "USEDMEMORY", "USEDCPU", "NETRX", "NETTX", "STATE"]
	
	def parse_xentop(name, stdout)
		line=stdout.split(/$/).select{|l| l.match(/^ *(migrating-)?#{name} /) }[-1]
		
		return nil if !line
		
		line.gsub!("no limit", "no_limit")
		data=line.split
		values=Hash.new
		
		# Get status code
		index=ColumnNames.index("STATE")
		state=data[index]
		state.gsub!("-", "")
		
		case state
		when "r", "b", "s"
			state="a" # alive
		when "p"
			state="p" # paused
		else
			state="e" # error
		end
		
		data[index]=state
		
		ColumnNames.each_with_index do |n, i|
			values[n]=data[i] if ColumnsToPrint.include? n
		end
	
		values
	end
	
end

dm=DM.new
dm.loop
