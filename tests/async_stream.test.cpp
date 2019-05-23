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

#include <catch2/catch.hpp>

#include <mvd/streams/async_stream.h>
#include <mvd/streams/access_policy.h>

#include <boost/predef.h>
#if BOOST_COMP_MSVC && (BOOST_COMP_MSVC < BOOST_VERSION_NUMBER( 19, 1, 0 ) )
#define BOOST_LOCKFREE_FORCE_BOOST_ATOMIC
#endif
#include <boost/lockfree/queue.hpp>

#include <future>
#include <random>
#include <set>
#include <deque>

namespace mvd
{
namespace streams
{
  template< typename T >
  using lockfree_queue_t = boost::lockfree::queue< T, boost::lockfree::fixed_sized< true > >;

  TEST_CASE( "async_stream (lockfree queue)" )
  {
    struct test_observer
      : observer< int, access_policy::none >
    {
      void on_event( int& v_ ) final { receivedValues.push_back( v_ ); }
      void on_done() final {};
      
      std::vector< int > receivedValues;
      bool onDoneReceived = false;
    };
    
    async_stream< int, access_policy::none, lockfree_queue_t > stream( 100u );
    
    SECTION( "All pushed events are dispatched only on explicit request" )
    {
      std::vector< int > receivedValues;
      test_observer o;
      stream.subscribe( o );
      
      std::vector< int > expectedValues = { 42, 314, 7, 0 };
      for( auto i : expectedValues )
        stream << i;
      
      REQUIRE( o.receivedValues.empty() );
      
      stream.dispatch_events();
      
      CHECK( o.receivedValues == expectedValues );
    }
    
    SECTION( "All pushed events are removed from stream after being dispatched once" )
    {
      std::vector< int > receivedValues;
      test_observer o;
      stream.subscribe( o );
      
      std::vector< int > expectedValues = { 42, 314, 7, 0 };
      for( auto i : expectedValues )
        stream << i;
      
      stream.dispatch_events();
      o.receivedValues.clear();
      REQUIRE( o.receivedValues.empty() );
      
      stream.dispatch_events();
      
      CHECK( o.receivedValues.empty() );
    }
    
    SECTION( "onEvent hook is invoked" )
    {
      int receivedValue = 0;
      stream.register_on_event_hook( [&receivedValue](int i){ receivedValue = i; return false; } );
      
      int expectedValue = 42;
      stream << expectedValue;
      
      CHECK( receivedValue == expectedValue );
    }
    
    SECTION( "Events are not blocked if onEvent hook returns true" )
    {
      int receivedValue = 0;
      test_observer o;
      stream.subscribe( o );
      stream.register_on_event_hook( [&receivedValue](int i){ receivedValue = i; return true; } );
      
      int expectedValue = 42;
      stream << expectedValue;
      stream.dispatch_events();
      
      CHECK( receivedValue == expectedValue );
      CHECK( o.receivedValues.back() == expectedValue );
    }
    
    SECTION( "events are blocked if onEvent hook returns false" )
    {
      int receivedValue = 0;
      test_observer o;
      stream.subscribe( o );
      stream.register_on_event_hook( [&receivedValue](int i){ receivedValue = i; return false; } );
      
      int expectedValue = 42;
      stream << expectedValue;
      stream.dispatch_events();
      
      CHECK( receivedValue == expectedValue );
      CHECK( o.receivedValues.empty() );
    }    
  
  }


  TEST_CASE( "async_stream (std::deque)" )
  {
    struct test_observer
      : observer< int, access_policy::none >
    {
      void on_event( int& v_ ) final { receivedValues.push_back( v_ ); }
      void on_done() final {};
      
      std::vector< int > receivedValues;
      bool onDoneReceived = false;
    };

    async_stream< int, access_policy::none > stream( default_queue< int >( 100u ) );
    
    SECTION( "Copying async_stream copies the queue" )
    {
      std::vector< int > receivedValues;
      
      std::vector< int > expectedValues = { 42, 314, 7, 0 };
      for( auto i : expectedValues )
        stream << i;
      
      test_observer o1;
      stream.subscribe( o1 );
      
      test_observer o2;
      auto stream2 = stream;
      stream2.subscribe( o2 );
      
      stream.dispatch_events();
      stream2.dispatch_events();
      
      CHECK( o1.receivedValues == expectedValues  );
      CHECK( o2.receivedValues == expectedValues );
    }
    
    SECTION( "Moving async_stream moves the queue" )
    {
      std::vector< int > receivedValues;
      
      std::vector< int > expectedValues = { 42, 314, 7, 0 };
      for( auto i : expectedValues )
        stream << i;
      
      test_observer o1;
      stream.subscribe( o1 );
      
      auto stream2 = std::move( stream );
      test_observer o2;
      stream2.subscribe( o2 );
      
      stream.dispatch_events();
      stream2.dispatch_events();
      
      CHECK( o1.receivedValues.empty() );
      CHECK( o2.receivedValues == expectedValues );
    }
  }
    
    
  TEST_CASE( "async_observer (lockfree queue)" )
  {
    struct test_observer
      : async_observer < int, boost::lockfree::queue< int >, access_policy::none >
    {
      test_observer()
        : async_observer( 100u )
      {}
    };
    
    stream< int, access_policy::none > stream;

    SECTION( "All received events are processed correctly" )
    {
      test_observer o;
      stream.subscribe( o );  
      
      std::vector< int > expectedValues = { 42, 314, 7, 0 };
      for( auto i : expectedValues )
        stream << i;
        
      std::vector< int > receivedValues;
      o.process_events( [&receivedValues]( const int& i_ ) { receivedValues.push_back(i_); } );
      
      CHECK( receivedValues == expectedValues );
    }
    
    SECTION( "All received events are removed after being processed" )
    {
      test_observer o;
      stream.subscribe( o );
      
      std::vector< int > expectedValues = { 42, 314, 7, 0 };
      for( auto i : expectedValues )
        stream << i;
        
      std::vector< int > receivedValues;
      o.process_events( [&receivedValues]( const int& i_ ) { receivedValues.push_back(i_); } );
      
      receivedValues.clear();
      
      o.process_events( [&receivedValues]( const int& i_ ) { receivedValues.push_back(i_); } );
      CHECK( receivedValues.empty() );
    }
  }
}
}