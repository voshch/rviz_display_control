// Copyright 2026 voshch
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RVIZ_DISPLAY_CONTROL__RECONCILER_HPP_
#define RVIZ_DISPLAY_CONTROL__RECONCILER_HPP_

#include <QString>

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "rclcpp/rclcpp.hpp"

#include <rviz_common/panel.hpp>
#include <rviz_common/ros_integration/ros_node_abstraction_iface.hpp>

#include "rviz_display_control_msgs/msg/display_set.hpp"
#include "rcl_interfaces/msg/parameter.hpp"

namespace rviz_common
{
class Config;
class Display;
class DisplayGroup;
}  // namespace rviz_common

namespace rviz_display_control
{

// Panel base class. The fallback invisible-Display path is not needed: DisplayGroup
// mutation from the GUI thread emits the model insert/remove signals that keep the
// tree view live, so a Panel reconciles the tree without instability.
class Reconciler : public rviz_common::Panel
{
  Q_OBJECT

public:
  explicit Reconciler(QWidget * parent = nullptr);
  ~Reconciler() override;

  void onInitialize() override;
  void load(const rviz_common::Config & config) override;
  void save(rviz_common::Config config) const override;

Q_SIGNALS:
  // Marshals a parsed DisplaySet from the ROS thread to the GUI thread.
  void displaySetReceived(const rviz_display_control_msgs::msg::DisplaySet & set);

private Q_SLOTS:
  // Runs on the GUI thread, reconciles the managed subtree against `set`.
  void reconcile(const rviz_display_control_msgs::msg::DisplaySet & set);

private:
  // One display this plugin created, keyed by its DisplaySet id.
  struct Managed
  {
    rviz_common::Display * display;
    std::size_t content_hash;
  };

  // Clear the managed subtree and (re)subscribe to the configured topic.
  void start();

  // Subscribe to the configured topic with latched QoS.
  void subscribe();

  // Declare or update the node parameters carried by a set before creating displays.
  void applyNodeParams(const std::vector<rcl_interfaces::msg::Parameter> & params);

  // Return the managed root group, creating it under the rviz root if absent.
  rviz_common::DisplayGroup * ensureRoot();

  // Remove every child of the managed root and forget all tracked displays.
  void clearManaged();

  // Resolve or create the nested group chain along `group_path` under `parent`.
  rviz_common::DisplayGroup * ensureGroupPath(
    rviz_common::DisplayGroup * parent, const std::string & group_path);

  // Find a direct child group by name, nullptr if absent.
  rviz_common::DisplayGroup * findChildGroup(
    rviz_common::DisplayGroup * parent, const QString & name) const;

  // Run the create, add, initialize, load lifecycle for a leaf or group.
  rviz_common::Display * createDisplay(
    rviz_common::DisplayGroup * parent,
    const QString & class_id,
    const rviz_common::Config & config);

  // Remove managed groups that became empty after a reconcile.
  void pruneEmptyGroups(rviz_common::DisplayGroup * group);

  std::shared_ptr<rviz_common::ros_integration::RosNodeAbstractionIface> node_ptr_;
  rclcpp::Node::SharedPtr node_;

  std::string topic_;
  std::string root_group_;

  rclcpp::Subscription<rviz_display_control_msgs::msg::DisplaySet>::SharedPtr sub_;

  std::unordered_map<std::string, Managed> live_;
};

}  // namespace rviz_display_control

#endif  // RVIZ_DISPLAY_CONTROL__RECONCILER_HPP_
