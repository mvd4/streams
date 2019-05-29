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
#include <mvd/streams/basic_async_stream.h>

#include <future>
#include <random>
#include <set>

namespace mvd
{
namespace streams
{
  TEST_CASE( "filter basic_stream" )
  {
    struct filter_observer : basic_observer< int, access_policy::none >
    {
      void on_event( int& v_ ) final { values.push_back( v_ ); }
      void on_done() final { onDoneReceived = true; }
    
      std::vector< int > values;
      bool onDoneReceived = false;
    };
  
    using stream_t = basic_stream< int, access_policy::none >;
    
    
    SECTION( "Observer to filtered basic_stream receives filtered events only" )
    {
      stream_t s;
      auto filtered = s && []( const int& i ) { return i%2 == 0; };
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
  
    SECTION( "Copying a filtered basic_stream duplicates the filter" )
    {
      stream_t s;
      
      auto filtered1 = s && []( int i ) { return i%2 == 0; };
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
    
    SECTION( "Moving a filtered basic_stream receives the same filtered events" )
    {
      stream_t s;
      auto filtered1 = s && []( int i ) { return i%2 == 0; };
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
  
    SECTION( "Can remove source basic_stream before filter" )
    {
      stream_t filtered;
      
      {
        stream_t s;
        filtered = s && []( int i ) { return i%2 == 0; };
      
        filter_observer o;
        filtered.subscribe( o );
      }
    }
  }


  TEST_CASE( "filter basic_async_stream" )
  {
    struct filter_observer : basic_observer< int, access_policy::none >
    {
      void on_event( int& v_ ) final { values.push_back( v_ ); }
      void on_done() final { onDoneReceived = true; }
    
      std::vector< int > values;
      bool onDoneReceived = false;
    };
  
    using stream_t = basic_async_stream< int, access_policy::none >;
    
    
    SECTION( "Observer to filtered async_stream receives filtered events only" )
    {
      stream_t s( 100 );
      auto filtered = s && []( const int& i ) { return i%2 == 0; };
      filter_observer o;
      filtered.subscribe( o );
    
      std::vector< int > expectedValues = { 2, 4, 16, 100 };
      for( auto v : expectedValues )
      {
        s << v;
        s << v + 1;
      }
      
      s.dispatch_events();

      CHECK( o.values == expectedValues );
    }

    SECTION( "Copying a filtered asnyc_stream duplicates the filter" )
    {
      stream_t s( 100 );
      
      auto filtered1 = s && []( int i ) { return i%2 == 0; };
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
    
      s.dispatch_events();

      CHECK( o1.values == expectedValues );
      CHECK( o2.values == expectedValues );
    }
    
    SECTION( "Moving a filtered async_stream receives the same filtered events" )
    {
      stream_t s( 100 );
      auto filtered1 = s && []( int i ) { return i%2 == 0; };
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
    
      s.dispatch_events();

      CHECK( o.values == expectedValues );
    }
  
    SECTION( "Can remove source async_stream before filter" )
    {
      basic_stream< int, access_policy::none > filtered;
      
      {
        stream_t s( 100 );
        filtered = s && []( int i ) { return i%2 == 0; };
      
        filter_observer o;
        filtered.subscribe( o );
      }
    }    
  }


  TEST_CASE( "merge basic_stream" )
  {
    struct merge_observer : basic_observer< int, access_policy::none >
    {
      void on_event( int& v_ ) final { values.push_back( v_ ); }
      void on_done() final { onDoneReceived = true; }
    
      std::vector< int > values;
      bool onDoneReceived = false;
    };
  
    using stream_t = basic_stream< int, access_policy::none >;
    
    
    SECTION( "Observer to merged basic_stream receives events from both sources" )
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
    
    
    SECTION( "Copyied merged basic_stream merges the same source streams" )
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
      
      std::vector< int > expectedValues = { 2, 4, 15, 27, 100 };
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
    

    SECTION( "Moved merged basic_stream merges the same source streams" )
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
      
      std::vector< int > expectedValues = { 2, 4, 15, 27, 100 };
      for( auto v : expectedValues )
      {
          if( v % 2 )
            s1 << v;
          else
            s2 << v;
      }
    
      CHECK( o1.values.empty() ); 
      CHECK( o2.values == expectedValues );
    }
 
    SECTION( "Can remove source streams before merged basic_stream" )
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


  TEST_CASE( "merge basic_async_stream" )
  {
    struct merge_observer : basic_observer< int, access_policy::none >
    {
      void on_event( int& v_ ) final { values.push_back( v_ ); }
      void on_done() final { onDoneReceived = true; }
    
      std::vector< int > values;
      bool onDoneReceived = false;
    };
  
    using stream_t = basic_async_stream< int, access_policy::none >;
    
    
    SECTION( "Observer to merged async_stream receives events from both sources" )
    {
      stream_t s1( 100 );
      stream_t s2( 100 );

      auto merged = s1 || s2;
      merge_observer o;
      merged.subscribe( o );
      
      std::vector< int > expectedValues = { 2, 4, 15, 27, 100 };
      for( auto v : expectedValues )
      {
          if( v < 20 )
            s1 << v;
          else
            s2 << v;
      }
      s1.dispatch_events();
      s2.dispatch_events();
    
      CHECK( o.values == expectedValues );
    }

    SECTION( "Copyied merged async_stream merges the same source streams" )
    {
      stream_t s1( 100 );
      stream_t s2( 100 );

      auto merged1 = s1 || s2;
      merge_observer o1;
      merged1.subscribe( o1 );

      auto merged2 = merged1;
      merge_observer o2;
      merged2.subscribe( o2 );
    
      REQUIRE( s1.get_observer_count() == 2 );
      REQUIRE( s2.get_observer_count() == 2 );
      
      std::vector< int > expectedValues = { 2, 4, 15, 27, 100 };
      for( auto v : expectedValues )
      {
          if( v < 20 )
            s1 << v;
          else
            s2 << v;
      }
      s1.dispatch_events();
      s2.dispatch_events();

      CHECK( o1.values == expectedValues );
      CHECK( o2.values == expectedValues );
    }
    

    SECTION( "Moved merged async_stream merges the same source streams" )
    {
      stream_t s1( 100 );
      stream_t s2( 100 );

      auto merged1 = s1 || s2;
      merge_observer o1;
      merged1.subscribe( o1 );

      auto merged2 = std::move( merged1 );
      merge_observer o2;
      merged2.subscribe( o2 );
    
      REQUIRE( s1.get_observer_count() == 1 );
      REQUIRE( s2.get_observer_count() == 1 );
      
      std::vector< int > expectedValues = { 2, 4, 15, 27, 100 };
      for( auto v : expectedValues )
      {
          if( v < 20 )
            s1 << v;
          else
            s2 << v;
      }
      s1.dispatch_events();
      s2.dispatch_events();

      CHECK( o1.values.empty() ); 
      CHECK( o2.values == expectedValues );
    }
 
    SECTION( "Can remove source streams before merged async_stream" )
    {
      basic_stream< int, access_policy::none > merged;
      
      {
        stream_t s1( 100 );
        stream_t s2( 100 );
        merged = s1 || s2;
      
        merge_observer o;
        merged.subscribe( o );
      }
    }
  }


  TEST_CASE( "map basic_stream" )
  {
    struct map_observer : basic_observer< std::string, access_policy::none >
    {
      void on_event( std::string& v_ ) final { receivedValues.push_back( v_ ); }
      void on_done() final { onDoneReceived = true; }
    
      std::vector< std::string > receivedValues;
      bool onDoneReceived = false;
    };
  
    using stream_t = basic_stream< int, access_policy::none >;
    
    
    SECTION( "Observer to mapped basic_stream receives mapped events" )
    {
      stream_t s;
      
      auto mapped = s >> static_cast< std::function< std::string( const int& ) > >(
        []( const int& i_ ) { return std::to_string( i_ ); }
      );

      map_observer o;
      mapped.subscribe( o );

      auto inputValues = std::vector< int >{ 1, 2, 4, 7, 11, 42 };
      auto expectedValues = std::vector< std::string >{ "1", "2", "4", "7", "11", "42" };

      for( auto v : inputValues )
      {
        s << v;
      }

      CHECK( o.receivedValues == expectedValues );
    }

    SECTION( "Copyied mapped basic_stream merges the same source streams" )
    {
      stream_t s;
      auto mapped1 = s >> static_cast< std::function< std::string( const int& ) > >(
        []( const int& i_ ) { return std::to_string( i_ ); }
      );
      
      map_observer o1;
      mapped1.subscribe( o1 );

      auto mapped2 = mapped1;
      map_observer o2;
      mapped2.subscribe( o2 );
    
      REQUIRE( s.get_observer_count() == 2 );
            
      auto inputValues = std::vector< int >{ 1, 2, 4, 7, 11, 42 };
      auto expectedValues = std::vector< std::string >{ "1", "2", "4", "7", "11", "42" };

      for( auto v : inputValues )
      {
        s << v;
      }
    
      CHECK( o1.receivedValues == expectedValues );
      CHECK( o2.receivedValues == expectedValues );
    }
    
    
    SECTION( "Moved mapped basic_stream merges the same source streams" )
    {
      stream_t s;
      auto mapped1 = s >> static_cast< std::function< std::string( const int& ) > >(
        []( const int& i_ ) { return std::to_string( i_ ); }
      );
      
      map_observer o1;
      mapped1.subscribe( o1 );

      auto mapped2= std::move( mapped1 );
      map_observer o2;
      mapped2.subscribe( o2 );
    
      REQUIRE( s.get_observer_count() == 1 );
      
      auto inputValues = std::vector< int >{ 1, 2, 4, 7, 11, 42 };
      auto expectedValues = std::vector< std::string >{ "1", "2", "4", "7", "11", "42" };

      for( auto v : inputValues )
      {
        s << v;
      }
    
      CHECK( o1.receivedValues.empty() );
      CHECK( o2.receivedValues == expectedValues );
    }
 
    SECTION( "Can remove source stream before mapped basic_stream" )
    {
      basic_stream< std::string, access_policy::none > mapped;
      
      {
        stream_t s;
        mapped = s >> static_cast< std::function< std::string( const int& ) > >(
          []( const int& i_ ) { return std::to_string( i_ ); }
        );
        
        map_observer o;
        mapped.subscribe( o );
      }
    }
  }


  TEST_CASE( "map basic_async_stream" )
  {
    struct map_observer : basic_observer< std::string, access_policy::none >
    {
      void on_event( std::string& v_ ) final { receivedValues.push_back( v_ ); }
      void on_done() final { onDoneReceived = true; }
    
      std::vector< std::string > receivedValues;
      bool onDoneReceived = false;
    };
  
    using stream_t = basic_async_stream< int, access_policy::none >;
    
    
    SECTION( "Observer to mapped async_stream receives mapped events" )
    {
      stream_t s( 100 );      
      auto mapped = s >> static_cast< std::function< std::string( const int& ) > >(
        []( const int& i_ ) { return std::to_string( i_ ); }
      );

      map_observer o;
      mapped.subscribe( o );

      auto inputValues = std::vector< int >{ 1, 2, 4, 7, 11, 42 };
      auto expectedValues = std::vector< std::string >{ "1", "2", "4", "7", "11", "42" };

      for( auto v : inputValues )
      {
        s << v;
      }
      s.dispatch_events();

      CHECK( o.receivedValues == expectedValues );
    }

    SECTION( "Copyied mapped async_stream merges the same source streams" )
    {
      stream_t s( 100 );
      auto mapped1 = s >> static_cast< std::function< std::string( const int& ) > >(
        []( const int& i_ ) { return std::to_string( i_ ); }
      );
      
      map_observer o1;
      mapped1.subscribe( o1 );

      auto mapped2 = mapped1;
      map_observer o2;
      mapped2.subscribe( o2 );
    
      REQUIRE( s.get_observer_count() == 2 );
            
      auto inputValues = std::vector< int >{ 1, 2, 4, 7, 11, 42 };
      auto expectedValues = std::vector< std::string >{ "1", "2", "4", "7", "11", "42" };

      for( auto v : inputValues )
      {
        s << v;
      }
      s.dispatch_events();

      CHECK( o1.receivedValues == expectedValues );
      CHECK( o2.receivedValues == expectedValues );
    }
    
    
    SECTION( "Moved mapped async_stream merges the same source streams" )
    {
      stream_t s( 100 );
      auto mapped1 = s >> static_cast< std::function< std::string( const int& ) > >(
        []( const int& i_ ) { return std::to_string( i_ ); }
      );
      
      map_observer o1;
      mapped1.subscribe( o1 );

      auto mapped2= std::move( mapped1 );
      map_observer o2;
      mapped2.subscribe( o2 );
    
      REQUIRE( s.get_observer_count() == 1 );
      
      auto inputValues = std::vector< int >{ 1, 2, 4, 7, 11, 42 };
      auto expectedValues = std::vector< std::string >{ "1", "2", "4", "7", "11", "42" };

      for( auto v : inputValues )
      {
        s << v;
      }
      s.dispatch_events();
    
      CHECK( o1.receivedValues.empty() );
      CHECK( o2.receivedValues == expectedValues );
    }
 
    SECTION( "Can remove source stream before mapped async_stream" )
    {
      basic_stream< std::string, access_policy::none > mapped;
      
      {
        stream_t s( 100 );
        mapped = s >> static_cast< std::function< std::string( const int& ) > >(
          []( const int& i_ ) { return std::to_string( i_ ); }
        );
        
        map_observer o;
        mapped.subscribe( o );
      }
    }
  }
}
}
