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
  // ---------------------------------------------------------------------------
  // filter_source
  // ---------------------------------------------------------------------------

  template< typename event_t >
  using filter_fn_t = std::function< bool( const event_t& ) >;

  template< typename stream_t >
  class filter_source 
    : public basic_observer< 
        typename stream_t::event_type, 
        typename stream_t::access_policy 
      >
  {
    using base_t = basic_observer< 
      typename stream_t::event_type, 
      typename stream_t::access_policy 
    >;

  public:

    using event_t = typename stream_t::event_type;
    
    
    filter_source( stream_t& s_, filter_fn_t< event_t > fn_ )
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

    filter_fn_t< event_t > m_filter;
    stream_t* m_pOutStream = nullptr;
  };


  // ---------------------------------------------------------------------------
  // merge_source
  // ---------------------------------------------------------------------------

  template< typename stream_t >
  class merge_source
  {
  public:

    using event_t = typename stream_t::event_type;
    using access_policy_t = typename stream_t::access_policy;
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


  // -----------------------------------------------------------------------------
  // map_source
  // -----------------------------------------------------------------------------

  template< typename src_event_t, typename dst_event_t >
  using map_fn_t = std::function< dst_event_t( const src_event_t& ) >;

  template< typename src_stream_t, typename dst_event_t >
  class map_source 
    : public basic_observer< 
        typename src_stream_t::event_type, 
        typename src_stream_t::access_policy 
      >
  {
    using base_t = basic_observer< 
      typename src_stream_t::event_type, 
      typename src_stream_t::access_policy 
    >;

  public:

    using src_event_t = typename src_stream_t::event_type;
    using access_policy_t = typename src_stream_t::access_policy;
    using dst_stream_t = basic_stream< dst_event_t, access_policy_t >;
    using map_fn_t = std::function< dst_event_t( const src_event_t& ) >;

    map_source( src_stream_t& s_, map_fn_t fn_ )
      : m_map( std::move( fn_ ) )
    {
      s_.subscribe( *this );
    }

    map_source( const map_source& other_ ) { *this = other_; }
    map_source& operator= ( const map_source& other_ )
    {
      base_t::operator= ( other_ );
      m_pOutStream = nullptr;
      m_map = other_.m_map;

      return *this;
    }
    
    map_source( map_source&& other_ ) { *this = std::move( other_ ); }
    map_source& operator= ( map_source&& other_ )
    {
      base_t::operator= ( std::move( other_ ) );
      m_pOutStream = nullptr;
      m_map = std::move( other_.m_map );

      return *this;
    }

    void attach( dst_stream_t& s_ )
    {
      m_pOutStream = &s_;
    }

    void on_event( src_event_t& e_ ) final
    {
      if ( m_pOutStream  )
        *m_pOutStream << m_map( e_ );
    }

    void on_done() final { /*! \todo */ }


  private:

    map_fn_t m_map;
    dst_stream_t* m_pOutStream = nullptr;
  };


  // -----------------------------------------------------------------------------
  // operators
  // -----------------------------------------------------------------------------

  template< typename stream_t >
  stream_t filter( 
    stream_t& s_, 
    filter_fn_t< typename stream_t::event_type > f_ 
  )
  {
    return std::move( stream_t( filter_source< stream_t >( s_, f_ ) ) );    
  }
  
  template< typename stream_t >
  stream_t operator& ( 
    stream_t& s_, 
    filter_fn_t< typename stream_t::event_type > f_ 
  )
  {
    return std::move( filter( s_, f_ ) );
  }



  template< typename stream_t >
  stream_t merge( 
    stream_t& s1_, 
    stream_t& s2_ 
  )
  {
    return std::move( stream_t( merge_source< stream_t >( s1_, s2_ ) ) );
  }

  template< typename stream_t >
  stream_t operator| ( 
    stream_t& s1_, 
    stream_t& s2_ 
  )
  {
    return std::move( merge( s1_, s2_ ) );
  }


  template< typename src_stream_t, typename dst_event_t >
  basic_stream< dst_event_t, typename src_stream_t::access_policy > map( 
    src_stream_t& s_, 
    map_fn_t< typename src_stream_t::event_type, dst_event_t > f_ 
  )
  {
    using dst_stream_t = basic_stream< dst_event_t, typename src_stream_t::access_policy >;

    return std::move( dst_stream_t( map_source< src_stream_t, dst_event_t >( s_, f_ ) ) );    
  }

  template< typename src_event_t, typename dst_event_t, typename access_policy_t >
  basic_stream< dst_event_t, access_policy_t > operator> ( 
    basic_stream< src_event_t, access_policy_t >& s_, 
    typename basic_stream< src_event_t, access_policy_t >::map_fn_t f_ 
  )
  {
   return std::move( map( s_, f_ ) );
  }
}
}
