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

#include <vector>
#include <algorithm>
#include <functional>

namespace mvd
{
  // -----------------------------------------------------------------------------
  // observable_base
  // -----------------------------------------------------------------------------

  template< typename access_policy_t >
  class observer_base;

  template< typename access_policy_t >
  class observable_base
  {
    using observer_base_t = observer_base< access_policy_t >;
    
  public:

    observable_base() = default;
    virtual ~observable_base();

    observable_base( const observable_base& other_ ) { *this = other_; }
    observable_base& operator= ( const observable_base& )
    {
      // we implement this in order to not restrict derived classes
      // copying observer ptrs does not make sense, so don't do anything here
      return *this;  
    }
    
    observable_base( observable_base&& other_ ) { *this = std::move( other_ ); }
    observable_base& operator= ( observable_base&& other_ )
    {
      // we implement this in order to not restrict derived classes
      // moving observer ptrs is not really intuitive, so just unregister them
      for( auto o : other_.m_observers )
        o->stop_observing();
      other_.m_observers.clear();
      return *this;
    }

    void register_observer( observer_base_t& observer_base_ );
    void unregister_observer( observer_base_t& observer_base_ );

    size_t get_observer_count() const { return m_observers.size(); }
    
  protected:

    void for_each_observer( const std::function< void( observer_base_t& ) >& fn_ )
    {
      auto l = access_policy_t::scoped_lock( m_mutex );
      for ( auto o : m_observers )
        fn_( *o );
    }


  private:

    std::vector< observer_base_t* > m_observers;
    typename access_policy_t::mutex_t m_mutex;
  };



  // -----------------------------------------------------------------------------
  // observer_base
  // -----------------------------------------------------------------------------

  template< typename access_policy_t >
  class observer_base
  {
    friend class observable_base< access_policy_t >;
  public:

    observer_base() = default;

    virtual ~observer_base() { unregister(); }

    observer_base( const observer_base& other_ ) { *this = other_; }
    observer_base& operator= ( const observer_base& other_ )
    {
      if( this == &other_ )
        return *this;

      if ( other_.m_observable )
        other_.m_observable->register_observer( *this );
      else
        unregister();

      return *this;
    }

    observer_base( observer_base&& other_ ) { *this = std::move( other_ ); }
    observer_base operator= ( observer_base&& other_ )
    {
      if ( other_.m_observable )
      {
        other_.m_observable->register_observer( *this );
        other_.m_observable->unregister_observer( other_ );
      }
      else
      {
        unregister();
      }
      return *this;
    }

    bool is_observing() const { return m_observable != nullptr; }

  private:
    using observable_base_t = observable_base< access_policy_t >;
    
    void unregister()
    {
      if ( m_observable )
        m_observable->unregister_observer( *this );
    }

    void observe( observable_base_t& observable_ )
    {
      unregister();
      m_observable = &observable_;
    }

    void stop_observing()
    {
      m_observable = nullptr;
    }


    observable_base_t* m_observable = nullptr;
  };


  // -----------------------------------------------------------------------------
  // implementation
  // -----------------------------------------------------------------------------

  template< typename access_policy_t >
  observable_base< access_policy_t >::~observable_base()
  {
    auto l = access_policy_t::scoped_lock( m_mutex );
    for ( auto& o : m_observers )
    {
      o->stop_observing();
    }
  }


  template< typename access_policy_t >
  void observable_base< access_policy_t >::register_observer( observer_base< access_policy_t >& observer_base_ )
  {
    // no check here - don't pay for what you don't use => users have to make sure they don't subscribe the
    // same observer_base multiple times (and if they do, they need to remove it multiple times)
    auto l = access_policy_t::scoped_lock( m_mutex );
    observer_base_.observe( *this );
    m_observers.push_back( &observer_base_ );
  }


  template< typename access_policy_t >
  void observable_base< access_policy_t >::unregister_observer( observer_base< access_policy_t >& observer_base_ )
  {
    auto l = access_policy_t::scoped_lock( m_mutex );
    
    auto it = std::find_if(
        m_observers.begin(),
        m_observers.end(),
        [&observer_base_]( observer_base< access_policy_t >* o_ )
        {
          return o_ == &observer_base_;          
        }
    );
    if( it != m_observers.end() )
    {
      (*it)->stop_observing();
      m_observers.erase( it );
    }
  }

}
