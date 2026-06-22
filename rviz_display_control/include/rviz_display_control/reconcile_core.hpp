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

#ifndef RVIZ_DISPLAY_CONTROL__RECONCILE_CORE_HPP_
#define RVIZ_DISPLAY_CONTROL__RECONCILE_CORE_HPP_

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "rviz_display_control_msgs/msg/display_set.hpp"
#include "rviz_display_control_msgs/msg/display_spec.hpp"

namespace rviz_display_control
{

// Hash over the fields that decide whether a live display must be reloaded.
std::size_t contentHash(const rviz_display_control_msgs::msg::DisplaySpec & spec);

// Split a slash-delimited group_path into its non-empty segments.
std::vector<std::string> splitPath(const std::string & path);

// One display the plan wants created or updated, with its precomputed hash.
struct PlannedDisplay
{
  const rviz_display_control_msgs::msg::DisplaySpec * spec;
  std::size_t content_hash;
};

// The set of changes that turn `current` into `desired`, keyed by display id.
struct ReconcilePlan
{
  std::vector<std::string> to_remove;
  std::vector<PlannedDisplay> to_create;
  std::vector<PlannedDisplay> to_update;
  std::vector<std::string> duplicate_ids;
};

// Compute the reconcile plan. `current` maps each tracked display id to its last
// applied content hash. Duplicate ids in `desired` resolve last-wins. Pointers in
// the returned plan alias into `desired` and are valid for its lifetime.
ReconcilePlan plan(
  const std::unordered_map<std::string, std::size_t> & current,
  const rviz_display_control_msgs::msg::DisplaySet & desired);

}  // namespace rviz_display_control

#endif  // RVIZ_DISPLAY_CONTROL__RECONCILE_CORE_HPP_
