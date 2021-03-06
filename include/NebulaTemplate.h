/* -------------------------------------------------------------------------- */
/* Copyright 2002-2009, Distributed Systems Architecture Group, Universidad   */
/* Complutense de Madrid (dsa-research.org)                                   */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#ifndef NEBULA_TEMPLATE_H_
#define NEBULA_TEMPLATE_H_

#include "Template.h"
#include <map>

class NebulaTemplate : public Template
{    
public:

    NebulaTemplate(string& etc_location, string& var_location);
    
    ~NebulaTemplate(){};

    static const char * conf_name;
    
    int get(const char * name, vector<const Attribute*>& values) const
    {
        string _name(name);
        
        return Template::get(_name,values);   
    };
    
    void get(const char * name, string& values) const
    {
        string _name(name);
        
        Template::get(_name,values);   
    };
        
    void get(const char * name, int& values) const
    {
        string _name(name);
        
        Template::get(_name,values);   
    };
    
    void get(const char * name, time_t& values) const
    {
        const SingleAttribute *		sattr;
        vector<const Attribute *>	attr;
        
        string 						_name(name);
                
        if ( Template::get(_name,attr) == 0 )
        {
        	values = 0;
        	return;
        }
                       
        sattr = dynamic_cast<const SingleAttribute *>(attr[0]);
        
        if ( sattr != 0 )
        {
        	istringstream	is;
        	
        	is.str(sattr->value());
        	is >> values;
        }
        else
        	values = 0;        
    };    

private:
    friend class Nebula;
    
    string                  conf_file;
    
    map<string, Attribute*> conf_default;
    
    int load_configuration();
};


#endif /*NEBULA_TEMPLATE_H_*/
