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

#include "stream.h"


namespace mvd
{
  template< typename event_t, typename access_policy_t >
  class subject
    : public observer< event_t, access_policy_t >
    , public stream< event_t, access_policy_t >
  {
    using observer_t = observer< event_t, access_policy_t >;
    using stream_t = stream< event_t, access_policy_t >;
    
  public:

    using on_event_t = std::function< void( subject&, event_t& ) >;

    subject( on_event_t processFn )
      : m_processFn( std::move( processFn ) )
    {}

    subject( subject&& other_ ) { *this = std::move( other_ ); }
    subject& operator= ( subject&& other_ )
    {
      observer_t::operator= ( std::move( other_ ) );
      stream_t::operator= ( std::move( other_ ) );
      m_processFn = std::move( other_.m_processFn );
      return *this;
    }

    void on_event( event_t& e_ ) override
    {
      m_processFn( *this, e_ );
    }
    
    void on_done() override { stream_t::issue_on_done(); }

  private:

    on_event_t m_processFn;
  };

}

