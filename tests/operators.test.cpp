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

#include <mvd/streams/operators.h>
#include <mvd/streams/access_policy.h>

#include <future>
#include <random>
#include <set>

namespace mvd
{
namespace streams
{
  TEST_CASE( "filter streams" )
  {
    struct filter_observer : observer< int, access_policy::none >
    {
      void on_event( int& v_ ) final { values.push_back( v_ ); }
      void on_done() final { onDoneReceived = true; }
    
      std::vector< int > values;
      bool onDoneReceived = false;
    };
  
    using stream_t = stream< int, access_policy::none >;
    
    
    SECTION( "Observer to filtered stream receives filtered events only" )
    {
      stream_t s;
      auto filtered = s >> []( const int& i ) { return i%2 == 0; };
      filter_observer o;
      filtered.subscribe( o );
    
      std::vector< int > expectedValues = { 2, 4, 16, 100 };
      for( auto v : expectedValues )
      {
        s << v;
        s << v + 1;
      }
    
      CHECK( o.values == expectedValues );
    }
  
    SECTION( "Copying a filtered stream duplicates the filter" )
    {
      stream_t s;
      
      auto filtered1 = s >> []( int i ) { return i%2 == 0; };
      filter_observer o1;
      filtered1.subscribe( o1 );

      auto filtered2 = filtered1;
      filter_observer o2;
      filtered2.subscribe( o2 );
    
      std::vector< int > expectedValues = { 2, 4, 16, 100 };
      for( auto v : expectedValues )
      {
          s << v;
          s << v + 1;
      }
    
      CHECK( o1.values == expectedValues );
      CHECK( o2.values == expectedValues );
    }
    
    SECTION( "Moving a filtered stream receives the same filtered events" )
    {
      stream_t s;
      auto filtered1 = s >> []( int i ) { return i%2 == 0; };
      auto filtered2 = std::move( filtered1 );
    
      REQUIRE( s.get_observer_count() == 1 );
      
      filter_observer o;
      filtered2.subscribe( o );
    
      std::vector< int > expectedValues = { 2, 4, 16, 100 };
      for( auto v : expectedValues )
      {
          s << v;
          s << v + 1;
      }
    
      CHECK( o.values == expectedValues );
    }
  
    SECTION( "Can remove source stream before filter" )
    {
      stream_t filtered;
      
      {
        stream_t s;
        filtered = s >> []( int i ) { return i%2 == 0; };
      
        filter_observer o;
        filtered.subscribe( o );
      }
    }    
  }


  TEST_CASE( "merge streams" )
  {
    struct merge_observer : observer< int, access_policy::none >
    {
      void on_event( int& v_ ) final { values.push_back( v_ ); }
      void on_done() final { onDoneReceived = true; }
    
      std::vector< int > values;
      bool onDoneReceived = false;
    };
  
    using stream_t = stream< int, access_policy::none >;
    
    
    SECTION( "Observer to merged stream receives events from both sources" )
    {
      stream_t s1;
      stream_t s2;

      auto merged = s1 || s2;
      merge_observer o;
      merged.subscribe( o );
      
      std::vector< int > expectedValues = { 2, 4, 16, 100 };
      for( auto v : expectedValues )
      {
          if( v % 2 )
            s1 << v;
          else
            s2 << v;
      }
    
      CHECK( o.values == expectedValues );
    }
    
    
    SECTION( "Copyied merged stream merges the same source streams" )
    {
      stream_t s1;
      stream_t s2;

      auto merged1 = s1 || s2;
      merge_observer o1;
      merged1.subscribe( o1 );

      auto merged2 = merged1;
      merge_observer o2;
      merged2.subscribe( o2 );
    
      REQUIRE( s1.get_observer_count() == 2 );
      REQUIRE( s2.get_observer_count() == 2 );
      
      std::vector< int > expectedValues = { 2, 4, 16, 100 };
      for( auto v : expectedValues )
      {
          if( v % 2 )
            s1 << v;
          else
            s2 << v;
      }
    
      CHECK( o1.values == expectedValues );
      CHECK( o2.values == expectedValues );
    }
    

    SECTION( "Moved merged stream merges the same source streams" )
    {
      stream_t s1;
      stream_t s2;

      auto merged1 = s1 || s2;
      merge_observer o1;
      merged1.subscribe( o1 );

      auto merged2 = std::move( merged1 );
      merge_observer o2;
      merged2.subscribe( o2 );
    
      REQUIRE( s1.get_observer_count() == 1 );
      REQUIRE( s2.get_observer_count() == 1 );
      
      std::vector< int > expectedValues = { 2, 4, 16, 100 };
      for( auto v : expectedValues )
      {
          if( v % 2 )
            s1 << v;
          else
            s2 << v;
      }
    
      CHECK( o1.values.empty() );  // moving stream also moves subscribed observers??
      CHECK( o2.values == expectedValues );
    }
 
    SECTION( "Can remove source streams before merged stream" )
    {
      stream_t merged;
      
      {
        stream_t s1;
        stream_t s2;
        merged = s1 || s2;
      
        merge_observer o;
        merged.subscribe( o );
      }
    }
  }

}
}
