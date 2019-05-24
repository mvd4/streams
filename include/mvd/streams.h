/*************************************************************************************************************

 mvd streams


 Copyright 2019 mvd

 Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in
 compliance with the License. You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software distributed under the License is
 distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and limitations under the License.

*************************************************************************************************************/

#pragma once

#include "streams/basic_stream.h"
#include "streams/basic_async_stream.h"
#include "streams/access_policy.h"
#include "streams/operators.h"

namespace mvd
{
namespace streams
{
  template< typename event_t >
  using stream = basic_stream< event_t, access_policy::none >;

  template< typename event_t >
  using observer = basic_observer< event_t, access_policy::none >;

  template< typename event_t >
  using locked_stream = basic_stream< event_t, access_policy::locked >;

  template< typename event_t >
  using locked_observer = basic_observer< event_t, access_policy::locked >;


  template< typename event_t >
  using async_stream = basic_async_stream< event_t, access_policy::none >;

  template< typename event_t >
  using async_observer = basic_async_observer< event_t, access_policy::none >;

  template< typename event_t >
  using async_locked_stream = basic_async_stream< event_t, access_policy::locked >;

  template< typename event_t >
  using async_locked_observer = basic_async_observer< event_t, access_policy::locked >;
}
}