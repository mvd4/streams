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

#include <mvd/streams/observer.h>
#include <mvd/streams/access_policy.h>

#include <boost/predef.h>

#include <future>
#include <random>

namespace mvd
{
namespace streams
{
  TEST_CASE( "observable_base (non-locking)" )
  {
    using observer_t = observer_base < access_policy::none >;
    using observable_t = observable_base < access_policy::none >;
  
    SECTION( "Registering observer increases registered count" )
    {
      observer_t o1, o2;
      observable_t observable;
    
      REQUIRE( observable.get_observer_count() == 0 );
    
      observable.register_observer(o1);
      CHECK( observable.get_observer_count() == 1 );
    
      observable.register_observer(o2);
      CHECK( observable.get_observer_count() == 2 );
    }
  
    SECTION( "Unregistering observer decreases registered count" )
    {
      observer_t o1, o2;
      observable_t observable;
    
      observable.register_observer(o1);
      observable.register_observer(o2);
    
      REQUIRE( observable.get_observer_count() == 2 );
    
      observable.unregister_observer(o1);
      CHECK( observable.get_observer_count() == 1 );
    
      observable.unregister_observer(o2);
      CHECK( observable.get_observer_count() == 0 );
    }
  
    SECTION( "Unregistering unknown observer has no effect" )
    {
      observer_t o1, o2;
      observable_t observable;
    
      observable.register_observer(o1);
    
      REQUIRE( observable.get_observer_count() == 1 );
    
      observable.unregister_observer(o2);
      CHECK( observable.get_observer_count() == 1 );
    }

    SECTION( "Copy constructed observable has no observers" )
    {
      observer_t o1, o2;
      observable_t observable1;
    
      observable1.register_observer(o1);
      observable1.register_observer(o2);
    
      REQUIRE( observable1.get_observer_count() == 2 );
    
      observable_t observable2 = observable1;

      CHECK( observable1.get_observer_count() == 2 );
      CHECK( observable2.get_observer_count() == 0 );
    }

    SECTION( "Move constructing observable unregisters subscribed observers" )
    {
      observer_t o1, o2;
      observable_t observable1;
    
      observable1.register_observer(o1);
      observable1.register_observer(o2);
    
      REQUIRE( observable1.get_observer_count() == 2 );
    
      observable_t observable2 = std::move( observable1 );

      CHECK( observable1.get_observer_count() == 0 );
      CHECK( observable2.get_observer_count() == 0 );
      CHECK( o1.is_observing() == false );
      CHECK( o2.is_observing() == false );
    }

    SECTION( "Can delete observable before subscribed observers" )
    {
      observer_t o1, o2;
      {
        observable_t observable1;
    
        observable1.register_observer(o1);
        observable1.register_observer(o2);
      }
      CHECK( o1.is_observing() == false );   
      CHECK( o2.is_observing() == false );
    }
  }

  TEST_CASE( "observable_base (locking)" )
  {
    using observer_t = observer_base < access_policy::locked >;
    using observable_t = observable_base < access_policy::locked >;
    
    SECTION( "Concurrent subscription / unsubscription" )
    {
      auto observers = std::vector< observer_t >( 20 );
      observable_t observable;
    
      auto seed = std::random_device()();
      INFO( "Random seed used: " << seed );
      std::mt19937 gen( seed );
      std::uniform_int_distribution<> dist(1, 10);
 
      auto tasks = std::vector< std::future< void > >();
      for( auto& o : observers )
      {
        tasks.push_back( std::async(
          std::launch::async,
          [&o, &observable, &dist, &gen]()
          {
            for( size_t i = 0; i < 100; ++i )
            {
              observable.register_observer( o );
              std::this_thread::sleep_for( std::chrono::milliseconds( dist( gen ) ) );
              observable.unregister_observer( o );
              std::this_thread::sleep_for( std::chrono::milliseconds( dist( gen ) ) );
            }
          }
        ));
      }
      tasks.clear();
    
      CHECK( observable.get_observer_count() == 0 );
    }
  }


  TEST_CASE( "observer_base" )
  {
    using observer_t = observer_base < access_policy::none >;
    using observable_t = observable_base < access_policy::none >;
  
    SECTION( "Copy-constructed observer observes the same observable" )
    {
      observer_t o1;
      observable_t observable;
    
      observable.register_observer(o1);
    
      auto o2 = o1;
      
      CHECK( observable.get_observer_count() == 2 );
    }

    SECTION( "Move-constructed observer observes the same observable, moved-from observer is unsubscribed" )
    {
      observer_t o1;
      observable_t observable;
    
      observable.register_observer(o1);
    
      auto o2 = std::move( o1 );
      
      CHECK( o1.is_observing() == false );
      CHECK( observable.get_observer_count() == 1 );
    }
    
    SECTION( "Self copy-assigning observer has no effect" )
    {
      observer_t o1;
      observable_t observable;
    
      observable.register_observer(o1);
#if BOOST_COMP_CLANG && BOOST_COMP_CLANG >= BOOST_VERSION_NUMBER(7, 0, 0) && !defined __APPLE__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
      o1 = o1;
#if BOOST_COMP_CLANG && BOOST_COMP_CLANG >= BOOST_VERSION_NUMBER(7, 0, 0) && !defined __APPLE__
# pragma clang diagnostic pop
#endif

      CHECK( observable.get_observer_count() == 1 );
    }

   
    SECTION( "Copy assigning unsubscribed observer to subscribed observer unsubscribes it" )
    {
      observer_t o1, o2;
      observable_t observable;
    
      observable.register_observer(o1);

      o1 = o2;

      CHECK( observable.get_observer_count() == 0 );
      CHECK( o1.is_observing() == false );
    }

    SECTION( "Move assigning unsubscribed observer to subscribed observer unsubscribes it" )
    {
      observer_t o1, o2;
      observable_t observable;
    
      observable.register_observer(o1);

      o1 = std::move( o2 );

      CHECK( observable.get_observer_count() == 0 );
      CHECK( o1.is_observing() == false );
    }

    SECTION( "Can delete observers before observable" )
    {
      observable_t observable;
      
      {
        observer_t o1, o2;  
    
        observable.register_observer(o1);
        observable.register_observer(o2);
      }

      CHECK( observable.get_observer_count() == 0 );
    }
  }
}
}
