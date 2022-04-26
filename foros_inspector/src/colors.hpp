/*
 * Copyright (c) 2022 42dot All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AKIT_FAILOVER_FOROS_INSPECTOR_COLORS_HPP_
#define AKIT_FAILOVER_FOROS_INSPECTOR_COLORS_HPP_

namespace akit {
namespace failover {
namespace foros_inspector {

enum class Colors : char {
  kGreenOnBlack = 1,
  kRedOnBlack,
  kCyanOnBlack,
  kYellowOnBlack
};

}  // namespace foros_inspector
}  // namespace failover
}  // namespace akit

#endif  // AKIT_FAILOVER_FOROS_INSPECTOR_COLORS_HPP_
