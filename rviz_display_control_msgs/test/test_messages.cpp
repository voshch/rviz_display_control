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

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "rviz_display_control_msgs/msg/display_set.hpp"
#include "rviz_display_control_msgs/msg/display_spec.hpp"
#include "rcl_interfaces/msg/parameter.hpp"

// Schema guard: assigning each field to an explicitly-typed local makes a
// rename, removal, or retype fail to compile.

TEST(SchemaGuard, DisplaySpec)
{
  rviz_display_control_msgs::msg::DisplaySpec s;
  std::string id = s.id;
  std::string class_id = s.class_id;
  std::string group_path = s.group_path;
  std::string config = s.config;
  std::string require_topic = s.require_topic;
  (void)id;
  (void)class_id;
  (void)group_path;
  (void)config;
  (void)require_topic;
  SUCCEED();
}

TEST(SchemaGuard, DisplaySet)
{
  rviz_display_control_msgs::msg::DisplaySet ds;
  std::vector<rviz_display_control_msgs::msg::DisplaySpec> displays = ds.displays;
  std::string root_group = ds.root_group;
  std::vector<rcl_interfaces::msg::Parameter> node_params = ds.node_params;
  (void)displays;
  (void)root_group;
  (void)node_params;
  SUCCEED();
}
