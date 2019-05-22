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

#include <mutex>

namespace mvd
{
namespace access_policy
{
  class none
  {
  public:
    struct mutex_t {};
    struct lock_t { ~lock_t(){} };  // non-trivial dtor prevents unused variable warning

    static lock_t scoped_lock( mutex_t& ) { return lock_t{}; }
  };


  class locked
  {
    public:
      using mutex_t = std::mutex;
      using lock_t = std::unique_lock< mutex_t >;
      
    static lock_t scoped_lock( mutex_t& m_ )
    {
      return std::unique_lock< mutex_t >( m_ );
    }
  };  
}
}
