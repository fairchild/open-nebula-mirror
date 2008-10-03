
require 'thread'

=begin rdoc
Copyright 2002-2008, Distributed Systems Architecture Group, Universidad Complutense de Madrid (dsa-research.org)

This class manages a pool of threads with a maximun number of concurrent running jobs.

== Example

Creates 1000 threads that sleep 1 second and executes them with a concurrency of 100.

    th=ThreadScheduler.new(100)

    1000.times {
        th.new_thread {
            sleep 1
        }
    }
=end

class ThreadScheduler
    # Creates a new thread pool
    #
    # +concurrent_number+ is the maximun number of threads that can run
    # at the same time
    def initialize(concurrent_number=10)
        @concurrent_number=concurrent_number
        @jobs_queue=Array.new
        @threads=Array.new
        @threads_mutex=Mutex.new
        @threads_cond=ConditionVariable.new
        start_thread_runner
    end
    
    # Creates a new job that will be placed on the queue. It will be scheduled
    # when there is room at the selected concurrency level. Job is a block.
    def new_thread(&block)
        @threads_mutex.synchronize {
            @jobs_queue<<block
            
            # Awakes thread runner
            @threads_cond.signal
        }
    end
    
    # Kills the thread that manages job queues. Should be called before
    # exiting
    def shutdown
        @thread_runner.kill!
    end
    
    private
    
    # Selects new jobs to be run as threads
    #
    # NOTE: should be called inside a syncronize block
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
    
    # Takes out finished threads from the pool.
    #
    # NOTE: should be called inside a syncronize block
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

