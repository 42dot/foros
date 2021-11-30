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

#ifndef AKIT_FAILOVER_FOROS_CLUSTER_NODE_HPP_
#define AKIT_FAILOVER_FOROS_CLUSTER_NODE_HPP_

#include <rclcpp/callback_group.hpp>
#include <rclcpp/create_client.hpp>
#include <rclcpp/create_publisher.hpp>
#include <rclcpp/create_service.hpp>
#include <rclcpp/create_subscription.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_clock_interface.hpp>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>
#include <rclcpp/node_interfaces/node_parameters_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>
#include <rclcpp/node_interfaces/node_time_source_interface.hpp>
#include <rclcpp/node_interfaces/node_timers.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/node_interfaces/node_topics_interface.hpp>
#include <rclcpp/node_interfaces/node_waitables_interface.hpp>
#include <rclcpp/node_options.hpp>
#include <rclcpp/time.hpp>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "akit/failover/foros/cluster_node_lifecycle_interface.hpp"
#include "akit/failover/foros/cluster_node_options.hpp"
#include "akit/failover/foros/cluster_node_publisher.hpp"
#include "akit/failover/foros/cluster_node_service.hpp"
#include "akit/failover/foros/command.hpp"
#include "akit/failover/foros/common.hpp"

namespace akit {
namespace failover {
namespace foros {

class ClusterNodeImpl;

/// A Clustered node.
class ClusterNode : public std::enable_shared_from_this<ClusterNode>,
                    public ClusterNodeLifecycleInterface {
 public:
  RCLCPP_SMART_PTR_DEFINITIONS(ClusterNode)

  /// Create a new clustered node with the specified cluster name and node id.
  /**
   * \param[in] cluster_name Cluster name of the node.
   * \param[in] node_id ID of the node.
   * \param[in] cluster_node_ids IDs of nodes in the cluster.
   * \param[in] options Additional options to control creation of the node.
   */
  CLUSTER_NODE_PUBLIC
  explicit ClusterNode(
      const std::string &cluster_name, const uint32_t node_id,
      const std::vector<uint32_t> &cluster_node_ids,
      const ClusterNodeOptions &options = ClusterNodeOptions());

  /// Create a new clustered node with the specified cluster name and node id.
  /**
   * \param[in] cluster_name Cluster name of the node.
   * \param[in] node_id ID of the node.
   * \param[in] cluster_node_ids IDs of nodes in the cluster.
   * \param[in] node_namespace Namespace of the nodo.
   * \param[in] options Additional options to control creation of the node.
   */
  CLUSTER_NODE_PUBLIC
  explicit ClusterNode(
      const std::string &cluster_name, const uint32_t node_id,
      const std::vector<uint32_t> &cluster_node_ids,
      const std::string &node_namespace,
      const ClusterNodeOptions &options = ClusterNodeOptions());

  CLUSTER_NODE_PUBLIC
  virtual ~ClusterNode();

  /// Get the name of the node.
  /**
   * \return The name of the node.
   */
  CLUSTER_NODE_PUBLIC
  const char *get_name() const;

  /// Get the namespace of the node.
  /**
   * \return The namespace of the node.
   */
  CLUSTER_NODE_PUBLIC
  const char *get_namespace() const;

  /// Get the fully-qualified name of the node.
  /**
   * The fully-qualified name includes the local namespace and name of the node.
   *
   * \return Fully-qualified name of the node.
   */
  CLUSTER_NODE_PUBLIC
  const char *get_fully_qualified_name() const;

  /// Get the logger of the node.
  /**
   * \return The logger of the node.
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::Logger get_logger() const;

  /// Create a callback group.
  /**
   * \param[in] group_type Callback group type to create by this method.
   * \param[in] automatically_add_to_executor_with_node A boolean that
   *   determines whether a callback group is automatically added to an executor
   *   with the node with which it is associated.
   * \return A callback group.
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::CallbackGroup::SharedPtr create_callback_group(
      rclcpp::CallbackGroupType group_type,
      bool automatically_add_to_executor_with_node = true);

  /// Return the list of callback groups in the node.
  /**
   * \return List of callback groups in the node.
   */
  CLUSTER_NODE_PUBLIC
  const std::vector<rclcpp::CallbackGroup::WeakPtr> &get_callback_groups()
      const;

  /// Create a Publisher.
  /**
   * The rclcpp::QoS has several convenient constructors, including a
   * conversion constructor for size_t, which mimics older API's that
   * allows just a string and size_t to create a publisher.
   *
   * For example, all of these cases will work:
   *
   * ```cpp
   * pub = node->create_publisher<MsgT>("chatter", 10);  // implicitly KeepLast
   * pub = node->create_publisher<MsgT>("chatter", QoS(10));  // implicitly
   *                                                          // KeepLast
   * pub = node->create_publisher<MsgT>("chatter", QoS(KeepLast(10)));
   * pub = node->create_publisher<MsgT>("chatter", QoS(KeepAll()));
   * pub = node->create_publisher<MsgT>("chatter",
   * QoS(1).best_effort().durability_volatile());
   * {
   *   rclcpp::QoS custom_qos(KeepLast(10), rmw_qos_profile_sensor_data);
   *   pub = node->create_publisher<MsgT>("chatter", custom_qos);
   * }
   * ```
   *
   * The publisher options may optionally be passed as the third argument for
   * any of the above cases.
   *
   * \param[in] topic_name The topic for this publisher to publish on.
   * \param[in] qos The Quality of Service settings for the publisher.
   * \param[in] options Additional options for the created Publisher.
   * \return Shared pointer to the created cluster node publisher.
   */
  template <typename MessageT, typename AllocatorT = std::allocator<void>>
  std::shared_ptr<ClusterNodePublisher<MessageT, AllocatorT>> create_publisher(
      const std::string &topic_name, const rclcpp::QoS &qos,
      const rclcpp::PublisherOptionsWithAllocator<AllocatorT> &options =
          (rclcpp::PublisherOptionsWithAllocator<AllocatorT>()));

  /// Create a Subscription.
  /**
   * \param[in] topic_name The topic to subscribe on.
   * \param[in] callback The user-defined callback function.
   * \param[in] qos The quality of service for this subscription.
   * \param[in] options The subscription options for this subscription.
   * \param[in] msg_mem_strat The message memory strategy to use for allocating
   *   messages.
   * \return Shared pointer to the created subscription.
   */
  template <typename MessageT, typename CallbackT,
            typename AllocatorT = std::allocator<void>,
            typename CallbackMessageT = typename rclcpp::subscription_traits::
                has_message_type<CallbackT>::type,
            typename SubscriptionT = rclcpp::Subscription<MessageT, AllocatorT>,
            typename MessageMemoryStrategyT = rclcpp::message_memory_strategy::
                MessageMemoryStrategy<CallbackMessageT, AllocatorT>>
  std::shared_ptr<SubscriptionT> create_subscription(
      const std::string &topic_name, const rclcpp::QoS &qos,
      CallbackT &&callback,
      const rclcpp::SubscriptionOptionsWithAllocator<AllocatorT> &options =
          rclcpp::SubscriptionOptionsWithAllocator<AllocatorT>(),
      typename MessageMemoryStrategyT::SharedPtr msg_mem_strat =
          (MessageMemoryStrategyT::create_default()));

  /// Create a timer.
  /**
   * \param[in] period Time interval between triggers of the callback.
   * \param[in] callback User-defined callback function.
   * \param[in] group Callback group to execute this timer's callback in.
   */
  template <typename DurationRepT = int64_t, typename DurationT = std::milli,
            typename CallbackT>
  typename rclcpp::WallTimer<CallbackT>::SharedPtr create_wall_timer(
      std::chrono::duration<DurationRepT, DurationT> period, CallbackT callback,
      rclcpp::CallbackGroup::SharedPtr group = nullptr);

  /// Create a Client.
  /**
   * \param[in] service_name The topic to service on.
   * \param[in] qos_profile Quality of service profile for client.
   * \param[in] group Callback group to call the service.
   * \return Shared pointer to the created client.
   */
  template <typename ServiceT>
  typename rclcpp::Client<ServiceT>::SharedPtr create_client(
      const std::string &service_name,
      const rmw_qos_profile_t &qos_profile = rmw_qos_profile_services_default,
      rclcpp::CallbackGroup::SharedPtr group = nullptr);

  /// Create a Service.
  /**
   * \param[in] service_name The topic to service on.
   * \param[in] callback User-defined callback function.
   * \param[in] qos_profile Quality of service profile for client.
   * \param[in] group Callback group to call the service.
   * \return Shared pointer to the created service.
   */
  template <typename ServiceT, typename CallbackT>
  typename ClusterNodeService<ServiceT>::SharedPtr create_service(
      const std::string &service_name, CallbackT &&callback,
      const rmw_qos_profile_t &qos_profile = rmw_qos_profile_services_default,
      rclcpp::CallbackGroup::SharedPtr group = nullptr);

  /// Declare and initialize a parameter, return the effective value.
  /**
   * This method is used to declare that a parameter exists on this node.
   * If, at run-time, the user has provided an initial value then it will be
   * set in this method, otherwise the given default_value will be set.
   * In either case, the resulting value is returned, whether or not it is
   * based on the default value or the user provided initial value.
   *
   * If no parameter_descriptor is given, then the default values from the
   * message definition will be used, e.g. read_only will be false.
   *
   * The name and type in the given rcl_interfaces::msg::ParameterDescriptor
   * are ignored, and should be specified using the name argument to this
   * function and the default value's type instead.
   *
   * If `ignore_override` is `true`, the parameter override will be ignored.
   *
   * This method, if successful, will result in any callback registered with
   * add_on_set_parameters_callback to be called.
   * If that callback prevents the initial value for the parameter from being
   * set then rclcpp::exceptions::InvalidParameterValueException is thrown.
   *
   * The returned reference will remain valid until the parameter is
   * undeclared.
   *
   * \param[in] name The name of the parameter.
   * \param[in] default_value An initial value to be used if at run-time user
   *   did not override it.
   * \param[in] parameter_descriptor An optional, custom description for
   *   the parameter.
   * \param[in] ignore_override When `true`, the parameter override is ignored.
   *    Default to `false`.
   * \return A const reference to the value of the parameter.
   * \throws rclcpp::exceptions::ParameterAlreadyDeclaredException if parameter
   *   has already been declared.
   * \throws rclcpp::exceptions::InvalidParametersException if a parameter
   *   name is invalid.
   * \throws rclcpp::exceptions::InvalidParameterValueException if initial
   *   value fails to be set.
   * \throws rclcpp::exceptions::InvalidParameterTypeException
   *   if the type of the default value or override is wrong.
   */
  CLUSTER_NODE_PUBLIC const rclcpp::ParameterValue &declare_parameter(
      const std::string &name, const rclcpp::ParameterValue &default_value,
      const rcl_interfaces::msg::ParameterDescriptor &parameter_descriptor =
          rcl_interfaces::msg::ParameterDescriptor(),
      bool ignore_override = false);

  /// Declare and initialize a parameter, return the effective value.
  /**
   * Same as the previous one, but a default value is not provided and the user
   * must provide a parameter override of the correct type.
   *
   * \param[in] name The name of the parameter.
   * \param[in] type Desired type of the parameter, which will enforced at
   *   runtime.
   * \param[in] parameter_descriptor An optional, custom description for the
   *   parameter.
   * \param[in] ignore_override When `true`, the parameter override is ignored.
   *   Default to `false`.
   * \return A const reference to the value of the parameter.
   * \throws Same as the previous overload taking a default value.
   * \throws rclcpp::exceptions::InvalidParameterTypeException if an override is
   *   not provided or the provided override is of the wrong type.
   */
  CLUSTER_NODE_PUBLIC
  const rclcpp::ParameterValue &declare_parameter(
      const std::string &name, rclcpp::ParameterType type,
      const rcl_interfaces::msg::ParameterDescriptor &parameter_descriptor =
          rcl_interfaces::msg::ParameterDescriptor(),
      bool ignore_override = false);

  /// Declare and initialize a parameter with a type.
  /**
   * See the non-templated declare_parameter() on this class for details.
   *
   * If the type of the default value, and therefore also the type of return
   * value, differs from the initial value provided in the node options, then
   * a rclcpp::exceptions::InvalidParameterTypeException may be thrown.
   * To avoid this, use the declare_parameter() method which returns an
   * rclcpp::ParameterValue instead.
   *
   * Note, this method cannot return a const reference, because extending the
   * lifetime of a temporary only works recursively with member initializers,
   * and cannot be extended to members of a class returned.
   * The return value of this class is a copy of the member of a ParameterValue
   * which is returned by the other version of declare_parameter().
   * See also:
   *
   * https://en.cppreference.com/w/cpp/language/lifetime
   * https://herbsutter.com/2008/01/01/gotw-88-a-candidate-for-the-most-important-const/
   * https://www.youtube.com/watch?v=uQyT-5iWUow (cppnow 2018 presentation)
   */
  template <typename ParameterT>
  auto declare_parameter(
      const std::string &name, const ParameterT &default_value,
      const rcl_interfaces::msg::ParameterDescriptor &parameter_descriptor =
          rcl_interfaces::msg::ParameterDescriptor(),
      bool ignore_override = false);

  /// Declare and initialize a parameter with a type.
  /**
   * See the non-templated declare_parameter() on this class for details.
   */
  template <typename ParameterT>
  auto declare_parameter(
      const std::string &name,
      const rcl_interfaces::msg::ParameterDescriptor &parameter_descriptor =
          rcl_interfaces::msg::ParameterDescriptor(),
      bool ignore_override = false);

  /// Declare and initialize several parameters with the same namespace and
  /// type.
  /**
   * For each key in the map, a parameter with a name of "namespace.key"
   * will be set to the value in the map.
   * The resulting value for each declared parameter will be returned.
   *
   * The name expansion is naive, so if you set the namespace to be "foo.",
   * then the resulting parameter names will be like "foo..key".
   * However, if the namespace is an empty string, then no leading '.' will be
   * placed before each key, which would have been the case when naively
   * expanding "namespace.key".
   * This allows you to declare several parameters at once without a namespace.
   *
   * The map contains default values for parameters.
   * There is another overload which takes the std::pair with the default value
   * and descriptor.
   *
   * If `ignore_overrides` is `true`, all the overrides of the parameters
   * declared by the function call will be ignored.
   *
   * This method, if successful, will result in any callback registered with
   * add_on_set_parameters_callback to be called, once for each parameter.
   * If that callback prevents the initial value for any parameter from being
   * set then rclcpp::exceptions::InvalidParameterValueException is thrown.
   *
   * \param[in] parameter_namespace The namespace in which to declare the
   *   parameters.
   * \param[in] parameters The parameters to set in the given namespace.
   * \param[in] ignore_overrides When `true`, the parameters overrides are
   * ignored. Default to `false`.
   * \throws rclcpp::exceptions::ParameterAlreadyDeclaredException if parameter
   *   has already been declared.
   * \throws rclcpp::exceptions::InvalidParametersException if a parameter name
   *   is invalid.
   * \throws rclcpp::exceptions::InvalidParameterValueException if initial value
   *   fails to be set.
   */
  template <typename ParameterT>
  std::vector<ParameterT> declare_parameters(
      const std::string &parameter_namespace,
      const std::map<std::string, ParameterT> &parameters,
      bool ignore_overrides = false);

  /// Declare and initialize several parameters with the same namespace and
  /// type.
  /**
   * This version will take a map where the value is a pair, with the default
   * parameter value as the first item and a parameter descriptor as the second.
   *
   * See the simpler declare_parameters() on this class for more details.
   */
  template <typename ParameterT>
  std::vector<ParameterT> declare_parameters(
      const std::string &parameter_namespace,
      const std::map<
          std::string,
          std::pair<ParameterT, rcl_interfaces::msg::ParameterDescriptor>>
          &parameters,
      bool ignore_overrides = false);

  /// Undeclare a previously declared parameter.
  /**
   * This method will not cause a callback registered with
   * add_on_set_parameters_callback to be called.
   *
   * \param[in] name The name of the parameter to be undeclared.
   * \throws rclcpp::exceptions::ParameterNotDeclaredException if the parameter
   *   has not been declared.
   * \throws rclcpp::exceptions::ParameterImmutableException if the parameter
   *   was create as read_only (immutable).
   */
  CLUSTER_NODE_PUBLIC
  void undeclare_parameter(const std::string &name);

  /// Return true if a given parameter is declared.
  /**
   * \param[in] name The name of the parameter to check for being declared.
   * \return true if the parameter name has been declared, otherwise false.
   */
  CLUSTER_NODE_PUBLIC
  bool has_parameter(const std::string &name) const;

  /// Set a single parameter.
  /**
   * Set the given parameter and then return result of the set action.
   *
   * If the parameter has not been declared this function may throw the
   * rclcpp::exceptions::ParameterNotDeclaredException exception, but only if
   * the node was not created with the
   * rclcpp::NodeOptions::allow_undeclared_parameters set to true.
   * If undeclared parameters are allowed, then the parameter is implicitly
   * declared with the default parameter meta data before being set.
   * Parameter overrides are ignored by set_parameter.
   *
   * This method will result in any callback registered with
   * add_on_set_parameters_callback to be called.
   * If the callback prevents the parameter from being set, then it will be
   * reflected in the SetParametersResult that is returned, but no exception
   * will be thrown.
   *
   * If the value type of the parameter is rclcpp::PARAMETER_NOT_SET, and the
   * existing parameter type is something else, then the parameter will be
   * implicitly undeclared.
   * This will result in a parameter event indicating that the parameter was
   * deleted.
   *
   * \param[in] parameter The parameter to be set.
   * \return The result of the set action.
   * \throws rclcpp::exceptions::ParameterNotDeclaredException if the parameter
   *   has not been declared and undeclared parameters are not allowed.
   */
  CLUSTER_NODE_PUBLIC
  rcl_interfaces::msg::SetParametersResult set_parameter(
      const rclcpp::Parameter &parameter);

  /// Set one or more parameters, one at a time.
  /**
   * Set the given parameters, one at a time, and then return result of each
   * set action.
   *
   * Parameters are set in the order they are given within the input vector.
   *
   * Like set_parameter, if any of the parameters to be set have not first been
   * declared, and undeclared parameters are not allowed (the default), then
   * this method will throw rclcpp::exceptions::ParameterNotDeclaredException.
   *
   * If setting a parameter fails due to not being declared, then the
   * parameters which have already been set will stay set, and no attempt will
   * be made to set the parameters which come after.
   *
   * If a parameter fails to be set due to any other reason, like being
   * rejected by the user's callback (basically any reason other than not
   * having been declared beforehand), then that is reflected in the
   * corresponding SetParametersResult in the vector returned by this function.
   *
   * This method will result in any callback registered with
   * add_on_set_parameters_callback to be called, once for each parameter.
   * If the callback prevents the parameter from being set, then, as mentioned
   * before, it will be reflected in the corresponding SetParametersResult
   * that is returned, but no exception will be thrown.
   *
   * Like set_parameter() this method will implicitly undeclare parameters
   * with the type rclcpp::PARAMETER_NOT_SET.
   *
   * \param[in] parameters The vector of parameters to be set.
   * \return The results for each set action as a vector.
   * \throws rclcpp::exceptions::ParameterNotDeclaredException if any parameter
   *   has not been declared and undeclared parameters are not allowed.
   */
  CLUSTER_NODE_PUBLIC
  std::vector<rcl_interfaces::msg::SetParametersResult> set_parameters(
      const std::vector<rclcpp::Parameter> &parameters);

  /// Set one or more parameters, all at once.
  /**
   * Set the given parameters, all at one time, and then aggregate result.
   *
   * Behaves like set_parameter, except that it sets multiple parameters,
   * failing all if just one of the parameters are unsuccessfully set.
   * Either all of the parameters are set or none of them are set.
   *
   * Like set_parameter and set_parameters, this method may throw an
   * rclcpp::exceptions::ParameterNotDeclaredException exception if any of the
   * parameters to be set have not first been declared.
   * If the exception is thrown then none of the parameters will have been set.
   *
   * This method will result in any callback registered with
   * add_on_set_parameters_callback to be called, just one time.
   * If the callback prevents the parameters from being set, then it will be
   * reflected in the SetParametersResult which is returned, but no exception
   * will be thrown.
   *
   * If you pass multiple rclcpp::Parameter instances with the same name, then
   * only the last one in the vector (forward iteration) will be set.
   *
   * Like set_parameter() this method will implicitly undeclare parameters
   * with the type rclcpp::PARAMETER_NOT_SET.
   *
   * \param[in] parameters The vector of parameters to be set.
   * \return The aggregate result of setting all the parameters atomically.
   * \throws rclcpp::exceptions::ParameterNotDeclaredException if any parameter
   *   has not been declared and undeclared parameters are not allowed.
   */
  CLUSTER_NODE_PUBLIC
  rcl_interfaces::msg::SetParametersResult set_parameters_atomically(
      const std::vector<rclcpp::Parameter> &parameters);

  /// Return the parameter by the given name.
  /**
   * If the parameter has not been declared, then this method may throw the
   * rclcpp::exceptions::ParameterNotDeclaredException exception.
   *
   * If undeclared parameters are allowed, see the node option
   * rclcpp::NodeOptions::allow_undeclared_parameters, then this method will
   * not throw an exception, and instead return a default initialized
   * rclcpp::Parameter, which has a type of
   * rclcpp::ParameterType::PARAMETER_NOT_SET.
   *
   * \param[in] name The name of the parameter to get.
   * \return The requested parameter inside of a rclcpp parameter object.
   * \throws rclcpp::exceptions::ParameterNotDeclaredException if the parameter
   *   has not been declared and undeclared parameters are not allowed.
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::Parameter get_parameter(const std::string &name) const;

  /// Get the value of a parameter by the given name, true if it was
  /// set.
  /**
   * This method will never throw the
   * rclcpp::exceptions::ParameterNotDeclaredException exception, but will
   * instead return false if the parameter has not be previously declared.
   *
   * If the parameter was not declared, then the output argument for this
   * method which is called "parameter" will not be assigned a value.
   * If the parameter was declared, and therefore has a value, then it is
   * assigned into the "parameter" argument of this method.
   *
   * \param[in] name The name of the parameter to get.
   * \param[out] parameter The output storage for the parameter being retrieved.
   * \return true if the parameter was previously declared, otherwise false.
   */
  CLUSTER_NODE_PUBLIC
  bool get_parameter(const std::string &name,
                     rclcpp::Parameter &parameter) const;

  /// Get the value of a parameter by the given name, true if it was
  /// set.
  /**
   * Identical to the non-templated version of this method, except that when
   * assigning the output argument called "parameter", this method will attempt
   * to coerce the parameter value into the type requested by the given
   * template argument, which may fail and throw an exception.
   *
   * If the parameter has not been declared, it will not attempt to coerce the
   * value into the requested type, as it is known that the type is not set.
   *
   * \throws rclcpp::ParameterTypeException if the requested type does not
   *   match the value of the parameter which is stored.
   */
  template <typename ParameterT>
  bool get_parameter(const std::string &name, ParameterT &parameter) const;

  /// Get the parameter value, or the "alternative_value" if not set, and assign
  /// it to "parameter".
  /**
   * If the parameter was not set, then the "parameter" argument is assigned
   * the "alternative_value".
   *
   * Like the version of get_parameter() which returns a bool, this method will
   * not throw the rclcpp::exceptions::ParameterNotDeclaredException exception.
   *
   * In all cases, the parameter is never set or declared within the node.
   *
   * \param[in] name The name of the parameter to get.
   * \param[out] parameter The output where the value of the parameter should be
   *   assigned.
   * \param[in] alternative_value Value to be stored in output if the parameter
   *   was not set.
   * \returns true if the parameter was set, false otherwise.
   */
  template <typename ParameterT>
  bool get_parameter_or(const std::string &name, ParameterT &parameter,
                        const ParameterT &alternative_value) const;

  /// Return the parameters by the given parameter names.
  /**
   * Like get_parameters(), this method may throw the
   * rclcpp::exceptions::ParameterNotDeclaredException exception if the
   * requested parameter has not been declared and undeclared parameters are
   * not allowed.
   *
   * Also like get_parameters(), if undeclared parameters are allowed and the
   * parameter has not been declared, then the corresponding rclcpp::Parameter
   * will be default initialized and therefore have the type
   * rclcpp::ParameterType::PARAMETER_NOT_SET.
   *
   * \param[in] names The names of the parameters to be retrieved.
   * \return The parameters that were retrieved.
   * \throws rclcpp::exceptions::ParameterNotDeclaredException if any of the
   *   parameters have not been declared and undeclared parameters are not
   *   allowed.
   */
  CLUSTER_NODE_PUBLIC
  std::vector<rclcpp::Parameter> get_parameters(
      const std::vector<std::string> &names) const;

  /// Get the parameter values for all parameters that have a given prefix.
  /**
   * The "prefix" argument is used to list the parameters which are prefixed
   * with that prefix, see also list_parameters().
   *
   * The resulting list of parameter names are used to get the values of the
   * parameters.
   *
   * The names which are used as keys in the values map have the prefix removed.
   * For example, if you use the prefix "foo" and the parameters "foo.ping" and
   * "foo.pong" exist, then the returned map will have the keys "ping" and
   * "pong".
   *
   * An empty string for the prefix will match all parameters.
   *
   * If no parameters with the prefix are found, then the output parameter
   * "values" will be unchanged and false will be returned.
   * Otherwise, the parameter names and values will be stored in the map and
   * true will be returned to indicate "values" was mutated.
   *
   * This method will never throw the
   * rclcpp::exceptions::ParameterNotDeclaredException exception because the
   * action of listing the parameters is done atomically with getting the
   * values, and therefore they are only listed if already declared and cannot
   * be undeclared before being retrieved.
   *
   * Like the templated get_parameter() variant, this method will attempt to
   * coerce the parameter values into the type requested by the given
   * template argument, which may fail and throw an exception.
   *
   * \param[in] prefix The prefix of the parameters to get.
   * \param[out] values The map used to store the parameter names and values,
   *   respectively, with one entry per parameter matching prefix.
   * \return true if output "values" was changed, false otherwise.
   * \throws rclcpp::ParameterTypeException if the requested type does not
   *   match the value of the parameter which is stored.
   */
  template <typename ParameterT>
  bool get_parameters(const std::string &prefix,
                      std::map<std::string, ParameterT> &values) const;

  /// Return the parameter descriptor for the given parameter name.
  /**
   * Like get_parameters(), this method may throw the
   * rclcpp::exceptions::ParameterNotDeclaredException exception if the
   * requested parameter has not been declared and undeclared parameters are
   * not allowed.
   *
   * If undeclared parameters are allowed, then a default initialized
   * descriptor will be returned.
   *
   * \param[in] name The name of the parameter to describe.
   * \return The descriptor for the given parameter name.
   * \throws rclcpp::exceptions::ParameterNotDeclaredException if the
   *   parameter has not been declared and undeclared parameters are not
   *   allowed.
   * \throws std::runtime_error if the number of described parameters is more
   * than one.
   */
  CLUSTER_NODE_PUBLIC
  rcl_interfaces::msg::ParameterDescriptor describe_parameter(
      const std::string &name) const;

  /// Return a vector of parameter descriptors, one for each of the given names.
  /**
   * Like get_parameters(), this method may throw the
   * rclcpp::exceptions::ParameterNotDeclaredException exception if any of the
   * requested parameters have not been declared and undeclared parameters are
   * not allowed.
   *
   * If undeclared parameters are allowed, then a default initialized
   * descriptor will be returned for the undeclared parameter's descriptor.
   *
   * If the names vector is empty, then an empty vector will be returned.
   *
   * \param[in] names The list of parameter names to describe.
   * \return A list of parameter descriptors, one for each parameter given.
   * \throws rclcpp::exceptions::ParameterNotDeclaredException if any of the
   *   parameters have not been declared and undeclared parameters are not
   *   allowed.
   * \throws std::runtime_error if the number of described parameters is more
   * than one.
   */
  CLUSTER_NODE_PUBLIC
  std::vector<rcl_interfaces::msg::ParameterDescriptor> describe_parameters(
      const std::vector<std::string> &names) const;

  /// Return a vector of parameter types, one for each of the given names.
  /**
   * Like get_parameters(), this method may throw the
   * rclcpp::exceptions::ParameterNotDeclaredException exception if any of the
   * requested parameters have not been declared and undeclared parameters are
   * not allowed.
   *
   * If undeclared parameters are allowed, then the default type
   * rclcpp::ParameterType::PARAMETER_NOT_SET will be returned.
   *
   * \param[in] names The list of parameter names to get the types.
   * \return A list of parameter types, one for each parameter given.
   * \throws rclcpp::exceptions::ParameterNotDeclaredException if any of the
   *   parameters have not been declared and undeclared parameters are not
   *   allowed.
   */
  CLUSTER_NODE_PUBLIC
  std::vector<uint8_t> get_parameter_types(
      const std::vector<std::string> &names) const;

  /// Return a list of parameters with any of the given prefixes, up to the
  /// given depth.
  /**
   * \todo: properly document and test this method.
   */
  CLUSTER_NODE_PUBLIC
  rcl_interfaces::msg::ListParametersResult list_parameters(
      const std::vector<std::string> &prefixes, uint64_t depth) const;

  using OnSetParametersCallbackHandle =
      rclcpp::node_interfaces::OnSetParametersCallbackHandle;
  using OnParametersSetCallbackType = rclcpp::node_interfaces::
      NodeParametersInterface::OnParametersSetCallbackType;

  /// Add a callback for when parameters are being set.
  /**
   * The callback signature is designed to allow handling of any of the above
   * `set_parameter*` or `declare_parameter*` methods, and so it takes a const
   * reference to a vector of parameters to be set, and returns an instance of
   * rcl_interfaces::msg::SetParametersResult to indicate whether or not the
   * parameter should be set or not, and if not why.
   *
   * For an example callback:
   *
   * ```cpp
   * rcl_interfaces::msg::SetParametersResult
   * my_callback(const std::vector<rclcpp::Parameter> & parameters)
   * {
   *   rcl_interfaces::msg::SetParametersResult result;
   *   result.successful = true;
   *   for (const auto & parameter : parameters) {
   *     if (!some_condition) {
   *       result.successful = false;
   *       result.reason = "the reason it could not be allowed";
   *     }
   *   }
   *   return result;
   * }
   * ```
   *
   * You can see that the SetParametersResult is a boolean flag for success
   * and an optional reason that can be used in error reporting when it fails.
   *
   * This allows the node developer to control which parameters may be changed.
   *
   * It is considered bad practice to reject changes for "unknown" parameters as
   * this prevents other parts of the node (that may be aware of these
   * parameters) from handling them.
   *
   * Note that the callback is called when declare_parameter() and its variants
   * are called, and so you cannot assume the parameter has been set before
   * this callback, so when checking a new value against the existing one, you
   * must account for the case where the parameter is not yet set.
   *
   * Some constraints like read_only are enforced before the callback is called.
   *
   * The callback may introspect other already set parameters (by calling any
   * of the {get,list,describe}_parameter() methods), but may *not* modify
   * other parameters (by calling any of the {set,declare}_parameter() methods)
   * or modify the registered callback itself (by calling the
   * add_on_set_parameters_callback() method).  If a callback tries to do any
   * of the latter things,
   * rclcpp::exceptions::ParameterModifiedInCallbackException will be thrown.
   *
   * The callback functions must remain valid as long as the
   * returned smart pointer is valid.
   * The returned smart pointer can be promoted to a shared version.
   *
   * Resetting or letting the smart pointer go out of scope unregisters the
   * callback. `remove_on_set_parameters_callback` can also be used.
   *
   * The registered callbacks are called when a parameter is set.
   * When a callback returns a not successful result, the remaining callbacks
   * aren't called. The order of the callback is the reverse from the
   * registration order.
   *
   * \param callback The callback to register.
   * \return A shared pointer. The callback is valid as long as the smart
   *   pointer is alive.
   * \throws std::bad_alloc if the allocation of the
   *   OnSetParametersCallbackHandle fails.
   */
  CLUSTER_NODE_PUBLIC
  RCUTILS_WARN_UNUSED
  OnSetParametersCallbackHandle::SharedPtr add_on_set_parameters_callback(
      OnParametersSetCallbackType callback);

  /// Remove a callback registered with `add_on_set_parameters_callback`.
  /**
   * Delete a handler returned by `add_on_set_parameters_callback`.
   *
   * e.g.:
   *
   *    `remove_on_set_parameters_callback(scoped_callback.get())`
   *
   * As an alternative, the smart pointer can be reset:
   *
   *    `scoped_callback.reset()`
   *
   * Supposing that `scoped_callback` was the only owner.
   *
   * Calling `remove_on_set_parameters_callback` more than once with the same
   * handler, or calling it after the shared pointer has been reset is an error.
   * Resetting or letting the smart pointer go out of scope after calling
   * `remove_on_set_parameters_callback` is not a problem.
   *
   * \param handler The callback handler to remove.
   * \throws std::runtime_error if the handler was not created with
   *   `add_on_set_parameters_callback`, or if it has been removed before.
   */
  CLUSTER_NODE_PUBLIC
  void remove_on_set_parameters_callback(
      const OnSetParametersCallbackHandle *const handler);

  /// Get the fully-qualified names of all available nodes.
  /**
   * The fully-qualified name includes the local namespace and name of the node.
   *
   * \return A vector of fully-qualified names of nodes.
   */
  CLUSTER_NODE_PUBLIC
  std::vector<std::string> get_node_names() const;

  /// Return a map of existing topic names to list of topic types.
  /**
   * \return A map of existing topic names to list of topic types.
   * \throws std::runtime_error anything that rcl_error can throw.
   */
  CLUSTER_NODE_PUBLIC
  std::map<std::string, std::vector<std::string>> get_topic_names_and_types()
      const;

  /// Return a map of existing service names to list of service types.
  /**
   * \return A map of existing service names to list of service types.
   * \throws std::runtime_error anything that rcl_error can throw.
   */
  CLUSTER_NODE_PUBLIC
  std::map<std::string, std::vector<std::string>> get_service_names_and_types()
      const;

  /// Return a map of existing service names to list of service types for a
  /// specific node.
  /**
   * This function only considers services - not clients.
   * The returned names are the actual names used and do not have remap rules
   * applied.
   *
   * \param[in] node_name The name of the node.
   * \param[in] node_namespace The namespace of the node.
   * \return A map of existing service names to list of service types.
   * \throws std::runtime_error anything that rcl_error can throw.
   */
  CLUSTER_NODE_PUBLIC
  std::map<std::string, std::vector<std::string>>
  get_service_names_and_types_by_node(const std::string &node_name,
                                      const std::string &node_namespace) const;

  /// Return the number of publishers created for a given topic.
  /**
   * \param[in] topic_name The actual topic name used; it will not be
   *   automatically remapped.
   * \return The number of publishers that have been created for the given
   * topic. \throws std::runtime_error if publishers could not be counted.
   */
  CLUSTER_NODE_PUBLIC
  size_t count_publishers(const std::string &topic_name) const;

  /// Return the number of subscribers created for a given topic.
  /**
   * \param[in] topic_name The actual topic name used; it will not be
   *   automatically remapped.
   * \return The number of subscribers that have been created for the given
   * topic. \throws std::runtime_error if subscribers could not be counted.
   */
  CLUSTER_NODE_PUBLIC
  size_t count_subscribers(const std::string &topic_name) const;

  /// Return the topic endpoint information about publishers on a given topic.
  /**
   * The returned parameter is a list of topic endpoint information, where each
   * item will contain the node name, node namespace, topic type, endpoint type,
   * topic endpoint's GID, and its QoS profile.
   *
   * When the `no_mangle` parameter is `true`, the provided `topic_name` should
   * be a valid topic name for the middleware (useful when combining ROS with
   * native middleware (e.g. DDS) apps). When the `no_mangle` parameter is
   * `false`, the provided `topic_name` should follow ROS topic name
   * conventions.
   *
   * `topic_name` may be a relative, private, or fully qualified topic name.
   * A relative or private topic will be expanded using this node's namespace
   * and name. The queried `topic_name` is not remapped.
   *
   * \param[in] topic_name The actual topic name used; it will not be
   *   automatically remapped.
   * \param[in] no_mangle If `true`, `topic_name` needs to be a valid middleware
   *   topic name, otherwise it should be a valid ROS topic name. Defaults to
   *   `false`.
   * \return A list of TopicEndpointInfo representing all the publishers on this
   *   topic.
   * \throws InvalidTopicNameError if the given topic_name is invalid.
   * \throws std::runtime_error if internal error happens.
   */
  CLUSTER_NODE_PUBLIC
  std::vector<rclcpp::TopicEndpointInfo> get_publishers_info_by_topic(
      const std::string &topic_name, bool no_mangle = false) const;

  /// Return the topic endpoint information about subscriptions on a given
  /// topic.
  /**
   * The returned parameter is a list of topic endpoint information, where each
   * item will contain the node name, node namespace, topic type, endpoint type,
   * topic endpoint's GID, and its QoS profile.
   *
   * When the `no_mangle` parameter is `true`, the provided `topic_name` should
   * be a valid topic name for the middleware (useful when combining ROS with
   * native middleware (e.g. DDS) apps). When the `no_mangle` parameter is
   * `false`, the provided `topic_name` should follow ROS topic name
   * conventions.
   *
   * `topic_name` may be a relative, private, or fully qualified topic name.
   * A relative or private topic will be expanded using this node's namespace
   * and name. The queried `topic_name` is not remapped.
   *
   * \param[in] topic_name The actual topic name used; it will not be
   *   automatically remapped.
   * \param[in] no_mangle If `true`, `topic_name` needs to be a valid middleware
   *   topic name, otherwise it should be a valid ROS topic name. Defaults to
   *   `false`.
   * \return A list of TopicEndpointInfo representing all the subscriptions on
   *   this topic.
   * \throws InvalidTopicNameError if the given topic_name is invalid.
   * \throws std::runtime_error if internal error happens.
   */
  CLUSTER_NODE_PUBLIC
  std::vector<rclcpp::TopicEndpointInfo> get_subscriptions_info_by_topic(
      const std::string &topic_name, bool no_mangle = false) const;

  /// Return a graph event, which will be set anytime a graph change occurs.
  /* The graph Event object is a loan which must be returned.
   * The Event object is scoped and therefore to return the loan just let it go
   * out of scope.
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::Event::SharedPtr get_graph_event();

  /// Wait for a graph event to occur by waiting on an Event to become set.
  /**
   * The given Event must be acquire through the get_graph_event() method.
   *
   * \param[in] event Pointer to an Event to wait for.
   * \param[in] timeout Nanoseconds to wait for the Event to change the state.
   *
   * \throws InvalidEventError if the given event is nullptr.
   * \throws EventNotRegisteredError if the given event was not acquired with
   *   get_graph_event().
   */
  CLUSTER_NODE_PUBLIC
  void wait_for_graph_change(rclcpp::Event::SharedPtr event,
                             std::chrono::nanoseconds timeout);

  /// Get a clock as a non-const shared pointer which is managed by the node.
  /**
   * \sa rclcpp::node_interfaces::NodeClock::get_clock
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::Clock::SharedPtr get_clock();

  /// Get a clock as a const shared pointer which is managed by the node.
  /**
   * \sa rclcpp::node_interfaces::NodeClock::get_clock
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::Clock::ConstSharedPtr get_clock() const;

  /// Returns current time from the time source specified by clock_type.
  /**
   * \sa rclcpp::Clock::now
   */
  CLUSTER_NODE_PUBLIC
  rclcpp::Time now() const;

  /// Return the Node's internal NodeBaseInterface implementation.
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr
  get_node_base_interface();

  /// Return the Node's internal NodeClockInterface implementation.
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeClockInterface::SharedPtr
  get_node_clock_interface();

  /// Return the Node's internal NodeGraphInterface implementation.
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeGraphInterface::SharedPtr
  get_node_graph_interface();

  /// Return the Node's internal NodeLoggingInterface implementation.
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr
  get_node_logging_interface();

  /// Return the Node's internal NodeTimersInterface implementation.
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeTimersInterface::SharedPtr
  get_node_timers_interface();

  /// Return the Node's internal NodeTopicsInterface implementation.
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr
  get_node_topics_interface();

  /// Return the Node's internal NodeServicesInterface implementation.
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeServicesInterface::SharedPtr
  get_node_services_interface();

  /// Return the Node's internal NodeWaitablesInterface implementation.
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeWaitablesInterface::SharedPtr
  get_node_waitables_interface();

  /// Return the Node's internal NodeParametersInterface implementation.
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
  get_node_parameters_interface();

  /// Return the Node's internal NodeTimeSourceInterface implementation.
  CLUSTER_NODE_PUBLIC
  rclcpp::node_interfaces::NodeTimeSourceInterface::SharedPtr
  get_node_time_source_interface();

  /// Check whether the node is activated or not.
  /**
   * \return true if the node is activated, false if not.
   */
  CLUSTER_NODE_PUBLIC
  bool is_activated() final;

  /// Register the acitvated callback.
  /**
   * \param[in] callback The callback to register.
   */
  CLUSTER_NODE_PUBLIC
  void register_on_activated(std::function<void()> callback);

  /// Register the deacitvated callback.
  /**
   * \param[in] callback The callback to register.
   */
  CLUSTER_NODE_PUBLIC
  void register_on_deactivated(std::function<void()> callback);

  /// Register the standby callback.
  /**
   * \param[in] callback The callback to register.
   */
  CLUSTER_NODE_PUBLIC
  void register_on_standby(std::function<void()> callback);

  /// Commit a command to cluster.
  /**
   * \param[in] command A command to commit.
   * \param[in] callback The callback to receive the commit response.
   * \return Shared future of commit response.
   */
  CLUSTER_NODE_PUBLIC
  CommandCommitResponseSharedFuture commit_command(
      Command::SharedPtr command, CommandCommitResponseCallback callback);

  /// Get the size of available commands
  /**
   * \return The size of commands
   */
  CLUSTER_NODE_PUBLIC
  uint64_t get_commands_size();

  /// Get a command of given ID
  /**
   * \param[in] id ID of the command
   * \return Shared pointer of the command
   */
  CLUSTER_NODE_PUBLIC
  Command::SharedPtr get_command(uint64_t id);

  /// Register the commited callback
  /**
   * \param[in] callback The callback to register
   */
  CLUSTER_NODE_PUBLIC
  void register_on_committed(
      std::function<void(const uint64_t, Command::SharedPtr)> callback);

  /// Register the reverted callback
  /**
   * \param[in] callback The callback to register
   */
  CLUSTER_NODE_PUBLIC
  void register_on_reverted(std::function<void(const uint64_t)> callback);

 private:
  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base_;
  rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph_;
  rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr node_logging_;
  rclcpp::node_interfaces::NodeTimers::SharedPtr node_timers_;
  rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics_;
  rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services_;
  rclcpp::node_interfaces::NodeClockInterface::SharedPtr node_clock_;
  rclcpp::node_interfaces::NodeParametersInterface::SharedPtr node_parameters_;
  rclcpp::node_interfaces::NodeTimeSourceInterface::SharedPtr node_time_source_;
  rclcpp::node_interfaces::NodeWaitablesInterface::SharedPtr node_waitables_;

  std::unique_ptr<ClusterNodeImpl> impl_;
};

}  // namespace foros
}  // namespace failover
}  // namespace akit

#include "cluster_node_template.hpp"

#endif  // AKIT_FAILOVER_FOROS_CLUSTER_NODE_HPP_
