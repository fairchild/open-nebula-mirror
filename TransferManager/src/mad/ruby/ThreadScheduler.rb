# --------------------------------------------------------------------------
# Copyright 2002-2008, Distributed Systems Architecture Group, Universidad
# Complutense de Madrid (dsa-research.org)
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
#---------------------------------------------------------------------------

require 'thread'

class ThreadScheduler
    def initialize(concurrent_number=10)
        @concurrent_number=concurrent_number
        @jobs_queue=Array.new
        @threads=Array.new
        @threads_mutex=Mutex.new
        @threads_cond=ConditionVariable.new
        start_thread_runner
    end
    
    def new_thread(&block)
        @threads_mutex.synchronize {
            @jobs_queue<<block
            
            # Tell thread runner that the thread has finished
            @threads_cond.signal
        }
    end
    
    def shutdown
        @thread_runner.kill!
    end
    
    private
    
    def run_new_jobs
        if @threads.size<@concurrent_number
            number=@concurrent_number-@threads.size
            number.times do
                # Gets next job in the queue
                job=@jobs_queue.shift
                # Exits if there are no more jobs in the queue
                break if !job
                
                # Creates the thread
                thread=Thread.new {
                    job.call
                    @threads_mutex.synchronize {
                        # Tell thread runner that the thread has finished
                        @threads_cond.signal
                    }
                }
                @threads<<thread
            end
        end
    end
    
    # Takes out finished threads from the pool
    def delete_stopped_threads
        stopped_threads=@threads.select {|t| !t.alive? }
        @threads-=stopped_threads
    end
    
    def start_thread_runner
        @thread_runner=Thread.new {
            while true
                @threads_mutex.synchronize {
                    @threads_cond.wait(@threads_mutex)
                    delete_stopped_threads
                    run_new_jobs
                }
            end
        }
    end
end

if __FILE__ == $0
    th=ThreadScheduler.new(10)

    100.times {|n|
        100.times {|m|
            th.new_thread {
                #puts "Starting #{m+n*100}"
                sleep rand*(1)
                puts "Finishing #{m+n*100}"
            }
        }
    }

    th.new_thread {
        sleep 4
        th.shutdown
        exit(0)
    }

    sleep 3600
end

