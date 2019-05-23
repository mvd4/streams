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

#include <deque>

namespace mvd
{
namespace streams
{
  // -----------------------------------------------------------------------------
  // default_queue
  // -----------------------------------------------------------------------------

  template< typename event_t >
  class default_queue
  {
  public:
    default_queue( size_t capacity_  )
      : m_events( capacity_ )
    {}
      
    default_queue( const default_queue& other_ ) { *this = other_; }
    default_queue& operator= ( const default_queue& other_ )
    {
      m_size = other_.m_size;
      m_events = other_.m_events;
      return *this;
    }
    
    default_queue( default_queue&& other_ ) { *this = std::move( other_ ); }
    default_queue& operator= ( default_queue&& other_ )
    {
      m_size = other_.m_size;
      other_.m_size = 0;
      
      m_events = std::move( other_.m_events );
      return *this;
    }
    
    
    bool push( const event_t& e_ )
    {
      if( m_size >= m_events.size() )
        return false;
        
      m_events[m_size++] = e_;
      return true;
    }
    
    bool push( event_t&& e_ )
    {
      if( m_size >= m_events.size() )
        return false;
      
      m_events[m_size++] = std::move( e_ );
      
      return true;
    }
    
    bool pop( event_t& e_ )
    {
      if( m_size == 0 )
        return false;
      
      e_ = m_events[0];
      m_events.pop_front();
      m_events.emplace_back();
      --m_size;
      
      return true;
    }
    
  private:
  
    std::deque< event_t > m_events;
    size_t m_size{0};
  };
  
  
  // -----------------------------------------------------------------------------
  // async_stream
  // -----------------------------------------------------------------------------

  template<
    typename event_t,
    typename access_policy_t,
    template< typename > class collection_t = default_queue
  >
  class async_stream : public stream< event_t, access_policy_t >
  {
    using base_t = stream< event_t, access_policy_t >;

  public:
  
    using on_event_hook_t = std::function< bool( event_t ) >;

    async_stream( collection_t< event_t >&& preparedQueue_ )
      : m_events( std::move( preparedQueue_ ) )
    {}

    async_stream( size_t queueSize_ )
      : m_events( queueSize_ )
    {}
    
    async_stream& operator << ( event_t e_ ) final
    {
      if( !m_onEventHook || m_onEventHook( e_ ) )
      {
        m_events.push( event_t( e_ ) );
      }
      return *this;
    }
    
    void dispatch_events()
    {
      event_t e;
      while( m_events.pop( e ) )
      {
        this->for_each_observer(
          [&e]( observer_base< access_policy_t >& o_ )
          {
            static_cast< typename base_t::observer_t& >( o_ ).on_event( e );
          }
        );
      }
    }
    
    // can use this for pre-filtering events or for automatically triggering processing 
    // on another thread, e.g. by using a condition_variable on the logging thread that
    // - when notified - calls dispatch_events
    void register_on_event_hook( on_event_hook_t hook_ )
    {
      m_onEventHook = std::move( hook_ );
    }
    
  private:
  
    collection_t< event_t > m_events;
    on_event_hook_t m_onEventHook;
  };


  
  // -----------------------------------------------------------------------------
  // async_observer
  // -----------------------------------------------------------------------------

  template<
    typename event_t,
    typename collection_t,
    typename access_policy_t
  >
  class async_observer : public observer< event_t, access_policy_t >
  {
    using base_t = observer< event_t, access_policy_t >;

  public:

    async_observer( collection_t&& preparedQueue_ )
      : m_events( std::move( preparedQueue_ ) )
    {}

    async_observer( size_t queueSize_ )
      : m_events( queueSize_ )
    {}
      
    void on_event( event_t& e_ ) override
    {
      m_events.push( event_t( e_ ) );
    }
    
    void on_done() override {}
    
    void process_events( const std::function< void( const event_t& ) >& f_ )
    {
      event_t e;
      while( m_events.pop( e ) )
        f_( e );
    }
    
  private:
  
    collection_t m_events;
  };
  
  
}
}
