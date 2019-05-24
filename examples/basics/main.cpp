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


#include <mvd/streams.h>

#include <iostream>

void subscribe_lambdas()
{
  auto s = mvd::streams::stream< int >();
  s.subscribe( []( int& i ) { std::cout << "Lambda 1 received " << i << "\n"; } );
  s.subscribe( []( int& i ) { std::cout << "Lambda 2 received " << i << "\n"; } );

  s << 1 << 2 << 3 << 4 << 5 << 6;
}

void subscribe_observers()
{
  struct my_observer : mvd::streams::observer< int >
  {
    my_observer( std::string msg_ )
      : m_message( msg_ )
    {}

    void on_event( int& e_ ) override
    {
      std::cout << m_message.c_str() << e_ << "\n";
    }

    void on_done() override
    {
      std::cout << m_message.c_str() << "done \n";
    }

  private:
    std::string m_message;
  };


  auto o1 = my_observer( "Observer 1: " );
  auto o2 = my_observer( "Observer 1: " );
  {
    auto s = mvd::streams::stream< int >();
    s.subscribe( o1 );
    s.subscribe( o2 );

    s << 1 << 2 << 3 << 4 << 5 << 6;
  }

  std::cout << "subscribe_observers done\n";
}

int main( int, char*[] )
{
  subscribe_lambdas();
  subscribe_observers();

  return 0;
}
