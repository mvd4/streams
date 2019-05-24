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

#include "basic_stream.h"


namespace mvd
{
namespace streams
{
  template< typename event_t, typename access_policy_t >
  class filter_source : public basic_observer< event_t, access_policy_t >
  {
    using base_t = basic_observer< event_t, access_policy_t >;

  public:

    using stream_t = basic_stream< event_t, access_policy_t >;

    filter_source( stream_t& s_, typename stream_t::filter_fn_t fn_ )
      : m_filter( std::move( fn_ ) )
    {
      s_.subscribe( *this );
    }

    filter_source( const filter_source& other_ ) { *this = other_; }
    filter_source& operator= ( const filter_source& other_ )
    {
      base_t::operator= ( other_ );
      m_pOutStream = nullptr;
      m_filter = other_.m_filter;

      return *this;
    }
    
    filter_source( filter_source&& other_ ) { *this = std::move( other_ ); }
    filter_source& operator= ( filter_source&& other_ )
    {
      base_t::operator= ( std::move( other_ ) );
      m_pOutStream = nullptr;
      m_filter = std::move( other_.m_filter );

      return *this;
    }

    void attach( stream_t& s_ )
    {
      m_pOutStream = &s_;
    }

    void on_event( event_t& e_ ) final
    {
      if ( m_pOutStream && m_filter( e_ ) )
        *m_pOutStream << e_;
    }

    void on_done() final { /*! \todo */ }


  private:

    typename stream_t::filter_fn_t m_filter;
    stream_t* m_pOutStream = nullptr;
  };


  template< typename event_t, typename access_policy_t >
  class merge_source
  {
  public:

    using stream_t = basic_stream< event_t, access_policy_t >;
    using observer_t = basic_observer< event_t, access_policy_t >;

    merge_source( stream_t& s1_, stream_t& s2_ )
      : m_observer1( *this )
      , m_observer2( *this )
    {
      s1_.subscribe( m_observer1 );
      s2_.subscribe( m_observer2 );
    }

    merge_source( const merge_source& other_ ) { *this = other_; }
    merge_source& operator= ( const merge_source& other_ )
    {
      m_pOutStream = nullptr;

      m_observer1 = other_.m_observer1;
      m_observer2 = other_.m_observer2;
      
      m_observer1.set_parent( *this );
      m_observer2.set_parent( *this );
      m_onDoneReceived = other_.m_onDoneReceived;  // the status of the source streams doesn't change

      return *this;
    }
    
    merge_source( merge_source&& other_ ) { *this = std::move( other_ ); }
    merge_source& operator= ( merge_source&& other_ )
    {
      m_pOutStream = nullptr;
      
      m_observer1 = std::move( other_.m_observer1 );
      m_observer2 = std::move( other_.m_observer2 );
      
      m_observer1.set_parent( *this );
      m_observer2.set_parent( *this );
      m_onDoneReceived = other_.m_onDoneReceived;  // the status of the source streams doesn't change

      return *this;
    }

    void attach( stream_t& s_ )
    {
      m_pOutStream = &s_;
    }

    void on_event( event_t& e_ )
    {
      if ( m_pOutStream )
        *m_pOutStream << e_;
    }

    void on_done() 
    { 
      if( m_onDoneReceived )
        m_pOutStream->on_done();
      m_onDoneReceived = true;
    }


  private:

    class merge_observer : public observer_t
    {
    public:

      merge_observer() = default;       
      merge_observer( merge_source& src_ )
        : m_parent( &src_ )
      {}

      void set_parent( merge_source& src_ ) { m_parent = &src_; }

      void on_event( event_t& e_ ) final 
      { 
        if( m_parent )
          m_parent->on_event( e_ ); 
      }
      
      void on_done() final 
      { 
        if( m_parent )
          m_parent->on_done(); 
      }
      
    private:

      merge_source* m_parent = nullptr;
    };


    stream_t* m_pOutStream = nullptr;
    merge_observer m_observer1;
    merge_observer m_observer2;
    bool m_onDoneReceived = false;
  };




  template< typename event_t, typename access_policy_t >
  basic_stream< event_t, access_policy_t > filter( 
    basic_stream< event_t, access_policy_t >& s_, 
    typename basic_stream< event_t, access_policy_t >::filter_fn_t f_ 
  )
  {
    using stream_t = basic_stream< event_t, access_policy_t >;
    using filter_t = filter_source< event_t, access_policy_t >;
    
    return std::move( stream_t( filter_t( s_, f_ ) ) );    
  }
  
  template< typename event_t, typename access_policy_t >
  basic_stream< event_t, access_policy_t > operator>> ( 
    basic_stream< event_t, access_policy_t >& s_, 
    typename basic_stream< event_t, access_policy_t >::filter_fn_t f_ 
  )
  {
    return std::move( filter( s_, f_ ) );
  }



  template< typename event_t, typename access_policy_t >
  basic_stream< event_t, access_policy_t > merge( 
    basic_stream< event_t, access_policy_t >& s1_, 
    basic_stream< event_t, access_policy_t >& s2_ 
  )
  {
    using stream_t = basic_stream< event_t, access_policy_t >;

    return std::move( stream_t( merge_source< event_t, access_policy_t >( s1_, s2_ ) ) );
  }

  template< typename event_t, typename access_policy_t >
  basic_stream< event_t, access_policy_t > operator|| (
    basic_stream< event_t, access_policy_t >& s1_,
    basic_stream< event_t, access_policy_t >& s2_
  )
  {
    return std::move( merge( s1_, s2_ ) );
  }

}
}
