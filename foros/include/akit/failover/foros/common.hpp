/*
 * Copyright (c) 2021 42dot All rights reserved.
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

#ifndef AKIT_FAILOVER_FOROS_COMMON_HPP_
#define AKIT_FAILOVER_FOROS_COMMON_HPP_

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
//     https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
#ifdef __GNUC__
#define CLUSTER_NODE_EXPORT __attribute__((dllexport))
#define CLUSTER_NODE_IMPORT __attribute__((dllimport))
#else
#define CLUSTER_NODE_EXPORT __declspec(dllexport)
#define CLUSTER_NODE_IMPORT __declspec(dllimport)
#endif
#ifdef CLUSTER_NODE_BUILDING_DLL
#define CLUSTER_NODE_PUBLIC CLUSTER_NODE_EXPORT
#else
#define CLUSTER_NODE_PUBLIC CLUSTER_NODE_IMPORT
#endif
#define CLUSTER_NODE_PUBLIC_TYPE CLUSTER_NODE_PUBLIC
#define CLUSTER_NODE_LOCAL
#else
#define CLUSTER_NODE_EXPORT __attribute__((visibility("default")))
#define CLUSTER_NODE_IMPORT
#if __GNUC__ >= 4
#define CLUSTER_NODE_PUBLIC __attribute__((visibility("default")))
#define CLUSTER_NODE_LOCAL __attribute__((visibility("hidden")))
#else
#define CLUSTER_NODE_PUBLIC
#define CLUSTER_NODE_LOCAL
#endif
#define CLUSTER_NODE_PUBLIC_TYPE
#endif

#endif  // AKIT_FAILOVER_FOROS_COMMON_HPP_
