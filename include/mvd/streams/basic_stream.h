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

#include "observer.h"

#include <memory>


namespace mvd
{
namespace streams
{
  // -----------------------------------------------------------------------------
  // basic_stream
  // -----------------------------------------------------------------------------

  template< typename event_t, typename access_policy_t >
  class basic_observer;
  
  template< typename event_t, typename access_policy_t >
  class basic_stream : public observable_base< access_policy_t >
  {
    using base_t = observable_base< access_policy_t >;
  public:

    using event_type = event_t;
    using access_policy = access_policy_t;

    using observer_t = basic_observer< event_t, access_policy_t >;
    using on_event_t = std::function< void( event_t& ) >;
    

    basic_stream() = default;
    ~basic_stream() override { on_done(); }
    
    template< typename source_t >
    basic_stream( source_t s_ )
      : m_source( std::make_unique< source_model_t< source_t > >( std::move( s_ ) ) )
    {
      m_source->attach( *this );
    }

    basic_stream( const basic_stream& other_ ) : base_t() { *this = other_; }
    basic_stream& operator= ( const basic_stream& other_ )
    {
      base_t::operator= ( other_ );
      if( other_.m_source )
      {
        m_source = other_.m_source->clone();
        m_source->attach( *this );
      }
      return *this;
    }
    
    basic_stream( basic_stream&& other_ ) : base_t() { *this = std::move( other_ ); }
    basic_stream& operator= ( basic_stream&& other_ )
    {
      //should moving the basic_stream actually move the observers??
      base_t::operator= ( std::move( other_ ) );
      m_lambdaObservers = std::move( other_.m_lambdaObservers );
      m_source = std::move( other_.m_source );
      if( m_source )
        m_source->attach( *this );
      return *this;
    }
    
    void subscribe( observer_t& o_ ) { base_t::register_observer( o_ ); }
    void subscribe( on_event_t callback_ );
    void unsubscribe( observer_t& o_ ) { base_t::unregister_observer( o_ ); }

    virtual basic_stream& operator << ( event_t e_ );
    void on_done();

  private:

    class lambda_observer : public observer_t
    {
    public:

      lambda_observer( on_event_t callback_ )
        : m_onEvent( callback_ )
      {}

      void on_event( event_t& e_ ) final { m_onEvent( e_ ); }
      void on_done() final {};
      
    private:

      on_event_t m_onEvent;
    };


    struct source_concept_t;
    using source_ptr_t = std::unique_ptr< source_concept_t >;


    struct source_concept_t
    {
      virtual ~source_concept_t() = default;

      virtual source_ptr_t clone() = 0;
      virtual void attach( basic_stream& s_ ) = 0;
    };


    template< typename source_impl_t >
    struct source_model_t : source_concept_t
    {
      source_model_t( source_impl_t impl_ )
        : m_impl( std::move( impl_ ) )
      {}

      source_ptr_t clone() final
      {
        return std::make_unique< source_model_t< source_impl_t > >( m_impl );
      }

      void attach( basic_stream& s_ ) final { m_impl.attach( s_ ); }

    private:
      source_impl_t m_impl;
    };
    
    std::vector< lambda_observer > m_lambdaObservers;
    typename access_policy_t::mutex_t m_mutex;
    source_ptr_t m_source;
  };


  // -----------------------------------------------------------------------------
  // basic_observer
  // -----------------------------------------------------------------------------

  template< typename event_t, typename access_policy_t >
  class basic_observer : public observer_base< access_policy_t >
  {
    friend class basic_stream< event_t, access_policy_t >;
  public:

    virtual void on_event( event_t& e_ ) = 0;
    virtual void on_done() = 0;
  };

  
  
  
    
  // -----------------------------------------------------------------------------
  // implementation
  // -----------------------------------------------------------------------------

  template< typename event_t, typename access_policy_t >
  inline void basic_stream< event_t, access_policy_t >::subscribe( on_event_t callback_ )
  {
    auto l = access_policy_t::scoped_lock( m_mutex );
    m_lambdaObservers.emplace_back( callback_ );
    subscribe( m_lambdaObservers.back() );
  }


  template< typename event_t, typename access_policy_t >
  inline basic_stream< event_t, access_policy_t >& basic_stream< event_t, access_policy_t >::operator<<(
    event_t e_
  )
  {
    this->for_each_observer(
      [&e_]( observer_base< access_policy_t >& o_ )
      {
        static_cast< observer_t& >( o_ ).on_event( e_ );
      }
    );
    return *this;
  }
  
  template< typename event_t, typename access_policy_t >
  inline void basic_stream< event_t, access_policy_t >::on_done()
  {
    this->for_each_observer(
      []( observer_base< access_policy_t >& o_ )
      {
        static_cast< observer_t& >( o_ ).on_done();
      }
    );
  }
  
}
}
