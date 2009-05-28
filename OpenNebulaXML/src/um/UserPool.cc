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

/* ************************************************************************** */
/* User Pool                                                    			  */
/* ************************************************************************** */

#include "UserPool.h"
#include "Nebula.h"

int UserPool::allocate (
    int *  oid,
    string username,
    string password,
    bool   enabled)
{
    User *        user;

    // Build a new User object

    user = new User(-1,
        username,
        password,
        enabled);

    // Insert the Object in the pool

    *oid = PoolSQL::allocate(user);

    return 0;
}
