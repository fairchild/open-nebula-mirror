#!/bin/bash

# -------------------------------------------------------------------------- 
# Copyright 2002-2006 GridWay Team, Distributed Systems Architecture         
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

echo "MAD started" > mad.log
 

while read COMMAND ARG1 ARG2 ARG3
do
    echo "$COMMAND $ARG1 $ARG2 $ARG3" >> mad.log
    case $COMMAND in
        "INIT" | "init")
            echo "INIT SUCCESS"
            ;;
        "FINALIZE" | "finalize")
            echo "FINALIZE SUCCESS"
            exit 0
            ;;            
        "TEST" | "test")
			echo "TEST SUCCESS $ARG1 $ARG2 $ARG3"
            ;;
        "FIRST" | "first")
            echo "FIRST SUCCESS $ARG1"
            ;;
        *)
            echo "$COMMAND - FAILURE Unknown command"
            ;;
    esac
done
