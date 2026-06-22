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

#include "rviz_display_control/reconcile_core.hpp"

#include <algorithm>
#include <functional>
#include <sstream>
#include <unordered_set>

namespace rviz_display_control
{

std::size_t contentHash(const rviz_display_control_msgs::msg::DisplaySpec & spec)
{
  std::hash<std::string> h;
  std::size_t seed = h(spec.class_id);
  auto mix = [&seed, &h](const std::string & s) {
      seed ^= h(s) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    };
  mix(spec.group_path);
  mix(spec.config);
  mix(spec.require_topic);
  return seed;
}

std::vector<std::string> splitPath(const std::string & path)
{
  std::vector<std::string> out;
  std::string seg;
  std::istringstream stream(path);
  while (std::getline(stream, seg, '/')) {
    if (!seg.empty()) {
      out.push_back(seg);
    }
  }
  return out;
}

ReconcilePlan plan(
  const std::unordered_map<std::string, std::size_t> & current,
  const rviz_display_control_msgs::msg::DisplaySet & desired)
{
  ReconcilePlan result;

  std::unordered_map<std::string, const rviz_display_control_msgs::msg::DisplaySpec *> latest;
  std::unordered_set<std::string> duplicated;
  std::vector<std::string> order;
  for (const auto & spec : desired.displays) {
    auto it = latest.find(spec.id);
    if (it == latest.end()) {
      latest.emplace(spec.id, &spec);
      order.push_back(spec.id);
    } else {
      it->second = &spec;
      duplicated.insert(spec.id);
    }
  }

  for (const auto & entry : current) {
    if (latest.find(entry.first) == latest.end()) {
      result.to_remove.push_back(entry.first);
    }
  }
  std::sort(result.to_remove.begin(), result.to_remove.end());

  for (const auto & id : order) {
    const auto * spec = latest.at(id);
    const std::size_t hash = contentHash(*spec);
    auto cur = current.find(id);
    if (cur == current.end()) {
      result.to_create.push_back({spec, hash});
    } else if (cur->second != hash) {
      result.to_update.push_back({spec, hash});
    }
  }

  for (const auto & id : order) {
    if (duplicated.count(id) > 0) {
      result.duplicate_ids.push_back(id);
    }
  }

  return result;
}

}  // namespace rviz_display_control
