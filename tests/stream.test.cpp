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

#include <mvd/streams/stream.h>
#include <mvd/streams/access_policy.h>

#include <future>
#include <random>
#include <set>

namespace mvd
{
namespace streams
{
  TEST_CASE( "Subscription (non-locking stream)" )
  {
    struct non_locking_observer : observer < int, access_policy::none >
    {
      void on_event( int& v_ ) final { value = v_; }
      void on_done() final { onDoneReceived = true; };
    
      int value = 0;
      bool onDoneReceived = false;
    };

    using stream_t = stream< int, access_policy::none >;
    
    SECTION( "Subscribing lambda receives events" )
    {
      int pushedValue = 42;
      int receivedValue = 0;

      stream_t stream;
      stream.subscribe( [&receivedValue]( int v_ ) { receivedValue = v_; } );
      stream << pushedValue;
    
      CHECK( receivedValue == pushedValue );
    }
  
    SECTION( "Subscribing Observer receives events" )
    {
      stream_t stream;
      
      non_locking_observer o;
      stream.subscribe( o );
    
      int pushedValue = 42;
      stream << pushedValue;

      CHECK( o.value == pushedValue );
    }

    SECTION( "Unsubscribed observer stops receiving events" )
    {
      stream_t stream;
      
      non_locking_observer o1;
      stream.subscribe( o1 );

      non_locking_observer o2;
      stream.subscribe( o2 );
    
      stream.unsubscribe( o1 );

      int pushedValue = 42;
      stream << pushedValue;
    
      CHECK( o1.value == 0 );
      CHECK( o2.value == pushedValue );
    }


    SECTION( "Can delete subscribed observer" )
    {
      stream_t stream;
      
      non_locking_observer o1;
      stream.subscribe( o1 );

      {
        non_locking_observer o2;
        stream.subscribe( o2 );
      }

      int pushedValue = 42;
      stream << pushedValue;
      CHECK( o1.value == pushedValue );
    }


    SECTION( "Can re-subscribe subscribed observer" )
    {
      stream_t stream1;
      stream_t stream2;

      non_locking_observer o1;
      stream1.subscribe( o1 );

      stream2.subscribe( o1 );

      int pushedValue = 42;
    
      stream1 << pushedValue;
      CHECK( o1.value == 0 );

      stream2 << pushedValue;
      CHECK( o1.value == pushedValue );
    }


    SECTION( "Deleting stream before subscribed observers issues onDone notification" )
    {
      non_locking_observer o;
    
      {
        stream_t stream;
        stream.subscribe( o );
      
        REQUIRE_FALSE( o.onDoneReceived );
      }
    
      CHECK( o.onDoneReceived );
    }


    SECTION( "Can copy construct subscribed observer" )
    {
      stream_t stream;

      non_locking_observer o1;
      stream.subscribe( o1 );

      non_locking_observer o2( o1 );

      int pushedValue = 42;
      stream << pushedValue;
      CHECK( o1.value == pushedValue );
      CHECK( o2.value == pushedValue );
    }


    SECTION( "Can move construct subscribed observer" )
    {
      stream_t stream;

      non_locking_observer o1;
      stream.subscribe( o1 );

      non_locking_observer o2( std::move( o1 ) );
    
      REQUIRE( stream.get_observer_count() == 1 );

      int pushedValue = 42;
      stream << pushedValue;
      
      REQUIRE( o1.value == 0 );
      CHECK( o2.value == pushedValue );
    }
    
    SECTION( "Copying stream does not copy subscribed lambdas" )
    {
      int pushedValue = 42;
      int expectedValue = 0;
      int receivedValue = 0;

      stream_t stream1;
      stream1.subscribe( [&receivedValue]( int v_ ) { receivedValue = v_; } );
      
      stream_t stream2 = stream1;
      stream2 << pushedValue;
    
      CHECK( receivedValue == expectedValue );
    }
    
    SECTION( "Moving stream unregisters subscribed lambdas" )
    {
      int pushedValue = 42;
      int receivedValue = 0;
      int expectedValue = receivedValue;
      
      stream_t stream1;
      stream1.subscribe( [&receivedValue]( int v_ ) { receivedValue = v_; } );
      
      stream_t stream2 = std::move( stream1 );
      stream2 << pushedValue;
    
      CHECK( stream1.get_observer_count() == 0 );
      CHECK( stream2.get_observer_count() == 0 );
      CHECK( receivedValue == expectedValue );
    }
    
    SECTION( "Moving stream unregisters subscribed observers" )
    {
      stream_t stream1;
      non_locking_observer o;
      stream1.subscribe( o );
    
      stream_t stream2 = std::move( stream1 );
      
      CHECK( stream1.get_observer_count() == 0 );
      CHECK( stream2.get_observer_count() == 0 );
      CHECK( o.is_observing() == false );
    }
  }


  TEST_CASE( "Subscription (locking stream)" )
  {
    struct locking_observer : observer < int, access_policy::locked >
    {
      void on_event( int& v_ ) final { value = v_; }
      void on_done() final { onDoneReceived = true; };
    
      int value = 0;
      bool onDoneReceived = false;
    };
    
    using stream_t = stream< int, access_policy::locked >;
  
    SECTION( "Subscribing lambda receives events" )
    {
      int pushedValue = 42;
      int receivedValue = 0;

      stream_t stream;
      stream.subscribe( [&receivedValue]( int v_ ) { receivedValue = v_; } );
      stream << pushedValue;
    
      CHECK( receivedValue == pushedValue );
    }
  
    SECTION( "Subscribing Observer receives events" )
    {
      stream_t stream;
      locking_observer o;
      stream.subscribe( o );
    
      int pushedValue = 42;
      stream << pushedValue;

      CHECK( o.value == pushedValue );
    }

    SECTION( "Can unsubscribe subscribed observer" )
    {
      stream_t stream;

      locking_observer o1;
      stream.subscribe( o1 );

      locking_observer o2;
      stream.subscribe( o2 );
    
      stream.unsubscribe( o1 );

      int pushedValue = 42;
      stream << pushedValue;
    
      CHECK( o1.value == 0 );
      CHECK( o2.value == pushedValue );
    }


    SECTION( "Can delete subscribed observer" )
    {
      stream_t stream;
    
      locking_observer o1;
      stream.subscribe( o1 );

      {
        locking_observer o2;
        stream.subscribe( o2 );
      }

      int pushedValue = 42;
      stream << pushedValue;
      CHECK( o1.value == pushedValue );
    }


    SECTION( "Can re-subscribe subscribed observer" )
    {
      stream_t stream1;
      stream_t stream2;
 
      locking_observer o1;
      stream1.subscribe( o1 );

      stream2.subscribe( o1 );

      int pushedValue = 42;
    
      stream1 << pushedValue;
      CHECK( o1.value == 0 );

      stream2 << pushedValue;
      CHECK( o1.value == pushedValue );
    }


    SECTION( "Deleting stream before subscribed observers issues onDone notification" )
    {
      locking_observer o;
    
      {
        stream_t stream;
        stream.subscribe( o );
      
        REQUIRE_FALSE( o.onDoneReceived );
      }
    
      CHECK( o.onDoneReceived );
    }


    SECTION( "Can copy construct subscribed observer" )
    {
      stream_t stream;

      locking_observer o1;
      stream.subscribe( o1 );

      locking_observer o2( o1 );

      int pushedValue = 42;
      stream << pushedValue;
      CHECK( o1.value == pushedValue );
      CHECK( o2.value == pushedValue );
    }

    SECTION( "Can move construct subscribed observer" )
    {
      stream_t stream;

      locking_observer o1;
      stream.subscribe( o1 );

      locking_observer o2( std::move( o1 ) );
    
      REQUIRE( stream.get_observer_count() == 1 );

      int pushedValue = 42;
      stream << pushedValue;
      
      REQUIRE( o1.value == 0 );
      CHECK( o2.value == pushedValue );
    }
    
    SECTION( "Moving stream unregisters subscribed lambdas" )
    {
      int pushedValue = 42;
      int receivedValue = 0;
      int expectedValue = receivedValue;
      
      stream_t stream1;
      stream1.subscribe( [&receivedValue]( int v_ ) { receivedValue = v_; } );
      
      stream_t stream2 = std::move( stream1 );
      stream2 << pushedValue;
    
      CHECK( stream1.get_observer_count() == 0 );
      CHECK( stream2.get_observer_count() == 0 );
      CHECK( receivedValue == expectedValue );
    }
    
    SECTION( "Moving stream unregisters subscribed observers" )
    {
      stream_t stream1;
      locking_observer o;
      stream1.subscribe( o );
    
      stream_t stream2 = std::move( stream1 );
      
      CHECK( stream1.get_observer_count() == 0 );
      CHECK( stream2.get_observer_count() == 0 );
      CHECK( o.is_observing() == false );
    }
    
    SECTION( "Concurrent subscription / unsubscription of observers" )
    {
      auto observers = std::vector< locking_observer >( 20 );
      stream_t stream;
    
      auto seed = std::random_device()();
      INFO( "Random seed used: " << seed );
      std::mt19937 gen( seed );
      std::uniform_int_distribution<> dist(1, 10);
 
      auto tasks = std::vector< std::future< void > >();
      for( auto& o : observers )
      {
        tasks.push_back( std::async(
          std::launch::async,
          [&o, &stream, &dist, &gen]()
          {
            for( size_t i = 0; i < 100; ++i )
            {
              stream.subscribe( o );
              std::this_thread::sleep_for( std::chrono::milliseconds( dist( gen ) ) );
              stream.unsubscribe( o );
              std::this_thread::sleep_for( std::chrono::milliseconds( dist( gen ) ) );
            }
          }
        ));
      }
      tasks.clear();
    
      CHECK( stream.get_observer_count() == 0 );
    }
  }
  
  
  TEST_CASE( "Concurrent access (locking stream)" )
  {
    struct test_observer : observer < int, access_policy::locked >
    {
      void on_event( int& v_ ) final { receivedValues.insert( v_ ); }
      void on_done() final {}
      
      std::set< int > receivedValues;
    };
    
    using stream_t = stream< int, access_policy::locked >;
    
    SECTION( "Send messages concurrently" )
    {
      auto seed = std::random_device()();
      INFO( "Random seed used: " << seed );
      std::mt19937 gen( seed );
      std::uniform_int_distribution<> dist(1, 1000);
 
      stream_t stream;
      test_observer o;
      stream.subscribe( o );
    
      std::set< int > expectedValues;
      std::mutex mtx;
    
      auto tasks = std::vector< std::future< void > >();
      for( size_t i = 0; i < 10; ++i )
      {
        tasks.push_back( std::async(
          std::launch::async,
          [&stream, &expectedValues, &dist, &gen, &mtx]()
          {
            for( size_t i = 0; i < 100; ++i )
            {
              auto val = dist( gen );
              {
                std::unique_lock< std::mutex > l(mtx);
                expectedValues.insert( val );
              }
              stream << val;
              std::this_thread::sleep_for( std::chrono::milliseconds( dist( gen ) / 100 ) );
            }
          }
        ));
      }
      tasks.clear();
    
      REQUIRE( o.receivedValues.size() == expectedValues.size() );
      CHECK( o.receivedValues == expectedValues );
    }
  }
}
}
