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

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "rviz_display_control/reconcile_core.hpp"

using rviz_display_control_msgs::msg::DisplaySet;
using rviz_display_control_msgs::msg::DisplaySpec;

namespace
{

DisplaySpec makeSpec(
  const std::string & id,
  const std::string & class_id = "pkg/MyDisplay",
  const std::string & group_path = "",
  const std::string & config = "",
  const std::string & require_topic = "")
{
  DisplaySpec s;
  s.id = id;
  s.class_id = class_id;
  s.group_path = group_path;
  s.config = config;
  s.require_topic = require_topic;
  return s;
}

DisplaySet makeSet(std::vector<DisplaySpec> specs)
{
  DisplaySet ds;
  ds.displays = std::move(specs);
  return ds;
}

// Apply a ReconcilePlan to a copy of `current`, returning the resulting map.
std::unordered_map<std::string, std::size_t> applyPlan(
  std::unordered_map<std::string, std::size_t> current,
  const rviz_display_control::ReconcilePlan & p)
{
  for (const auto & id : p.to_remove) {
    current.erase(id);
  }
  for (const auto & pd : p.to_create) {
    current[pd.spec->id] = pd.content_hash;
  }
  for (const auto & pd : p.to_update) {
    current[pd.spec->id] = pd.content_hash;
  }
  return current;
}

}  // namespace

TEST(ContentHash, EqualSpecsHashEqual)
{
  DisplaySpec a = makeSpec("x", "pkg/Foo", "grp", "cfg: 1", "/topic");
  DisplaySpec b = makeSpec("x", "pkg/Foo", "grp", "cfg: 1", "/topic");
  EXPECT_EQ(rviz_display_control::contentHash(a), rviz_display_control::contentHash(b));
}

TEST(ContentHash, DifferentClassIdChangesHash)
{
  DisplaySpec a = makeSpec("x", "pkg/Foo", "grp", "cfg: 1", "/topic");
  DisplaySpec b = makeSpec("x", "pkg/Bar", "grp", "cfg: 1", "/topic");
  EXPECT_NE(rviz_display_control::contentHash(a), rviz_display_control::contentHash(b));
}

TEST(ContentHash, DifferentGroupPathChangesHash)
{
  DisplaySpec a = makeSpec("x", "pkg/Foo", "grp/a", "cfg: 1", "/topic");
  DisplaySpec b = makeSpec("x", "pkg/Foo", "grp/b", "cfg: 1", "/topic");
  EXPECT_NE(rviz_display_control::contentHash(a), rviz_display_control::contentHash(b));
}

TEST(ContentHash, DifferentConfigChangesHash)
{
  DisplaySpec a = makeSpec("x", "pkg/Foo", "grp", "cfg: 1", "/topic");
  DisplaySpec b = makeSpec("x", "pkg/Foo", "grp", "cfg: 2", "/topic");
  EXPECT_NE(rviz_display_control::contentHash(a), rviz_display_control::contentHash(b));
}

TEST(ContentHash, DifferentRequireTopicChangesHash)
{
  DisplaySpec a = makeSpec("x", "pkg/Foo", "grp", "cfg: 1", "/topic_a");
  DisplaySpec b = makeSpec("x", "pkg/Foo", "grp", "cfg: 1", "/topic_b");
  EXPECT_NE(rviz_display_control::contentHash(a), rviz_display_control::contentHash(b));
}

TEST(ContentHash, DifferentIdOnlySameHash)
{
  // id is intentionally not part of the hash, changing only id must not change it.
  DisplaySpec a = makeSpec("id_a", "pkg/Foo", "grp", "cfg: 1", "/topic");
  DisplaySpec b = makeSpec("id_b", "pkg/Foo", "grp", "cfg: 1", "/topic");
  EXPECT_EQ(rviz_display_control::contentHash(a), rviz_display_control::contentHash(b));
}

TEST(SplitPath, EmptyStringReturnsEmpty)
{
  EXPECT_TRUE(rviz_display_control::splitPath("").empty());
}

TEST(SplitPath, SingleSegment)
{
  EXPECT_EQ(rviz_display_control::splitPath("a"), (std::vector<std::string>{"a"}));
}

TEST(SplitPath, ThreeSegments)
{
  EXPECT_EQ(
    rviz_display_control::splitPath("a/b/c"),
    (std::vector<std::string>{"a", "b", "c"}));
}

TEST(SplitPath, LeadingTrailingAndDoubleSlashes)
{
  EXPECT_EQ(
    rviz_display_control::splitPath("/a//b/"),
    (std::vector<std::string>{"a", "b"}));
}

TEST(SplitPath, OnlySlashesReturnsEmpty)
{
  EXPECT_TRUE(rviz_display_control::splitPath("///").empty());
}

TEST(Plan, EmptyCurrentTwoDesiredBothCreated)
{
  std::unordered_map<std::string, std::size_t> current;
  DisplaySet desired = makeSet({makeSpec("a"), makeSpec("b")});

  auto p = rviz_display_control::plan(current, desired);

  ASSERT_EQ(p.to_create.size(), 2u);
  EXPECT_EQ(p.to_create[0].spec->id, "a");
  EXPECT_EQ(p.to_create[1].spec->id, "b");
  EXPECT_TRUE(p.to_remove.empty());
  EXPECT_TRUE(p.to_update.empty());
  EXPECT_TRUE(p.duplicate_ids.empty());
}

TEST(Plan, UnchangedSpecProducesNothing)
{
  DisplaySpec s = makeSpec("a", "pkg/Foo", "grp", "cfg: 1", "/t");
  std::size_t h = rviz_display_control::contentHash(s);
  std::unordered_map<std::string, std::size_t> current{{"a", h}};
  DisplaySet desired = makeSet({s});

  auto p = rviz_display_control::plan(current, desired);

  EXPECT_TRUE(p.to_create.empty());
  EXPECT_TRUE(p.to_update.empty());
  EXPECT_TRUE(p.to_remove.empty());
  EXPECT_TRUE(p.duplicate_ids.empty());
}

TEST(Plan, ChangedConfigProducesUpdate)
{
  DisplaySpec old_spec = makeSpec("a", "pkg/Foo", "grp", "cfg: 1", "/t");
  std::size_t old_hash = rviz_display_control::contentHash(old_spec);
  std::unordered_map<std::string, std::size_t> current{{"a", old_hash}};

  DisplaySpec new_spec = makeSpec("a", "pkg/Foo", "grp", "cfg: 2", "/t");
  DisplaySet desired = makeSet({new_spec});

  auto p = rviz_display_control::plan(current, desired);

  ASSERT_EQ(p.to_update.size(), 1u);
  EXPECT_EQ(p.to_update[0].spec->id, "a");
  EXPECT_EQ(p.to_update[0].content_hash, rviz_display_control::contentHash(new_spec));
  EXPECT_TRUE(p.to_create.empty());
  EXPECT_TRUE(p.to_remove.empty());
}

TEST(Plan, AbsentFromDesiredGoesToRemoveSorted)
{
  std::unordered_map<std::string, std::size_t> current{
    {"c", 1}, {"a", 2}, {"b", 3}};
  DisplaySet desired = makeSet({});

  auto p = rviz_display_control::plan(current, desired);

  ASSERT_EQ(p.to_remove.size(), 3u);
  EXPECT_EQ(p.to_remove[0], "a");
  EXPECT_EQ(p.to_remove[1], "b");
  EXPECT_EQ(p.to_remove[2], "c");
  EXPECT_TRUE(p.to_create.empty());
  EXPECT_TRUE(p.to_update.empty());
}

TEST(Plan, DuplicateIdLastWinsHashAndDuplicateRecorded)
{
  DisplaySpec first = makeSpec("dup", "pkg/A", "", "cfg: 1", "");
  DisplaySpec last = makeSpec("dup", "pkg/B", "", "cfg: 2", "");
  std::unordered_map<std::string, std::size_t> current;
  DisplaySet desired = makeSet({first, last});

  auto p = rviz_display_control::plan(current, desired);

  ASSERT_EQ(p.to_create.size(), 1u);
  EXPECT_EQ(p.to_create[0].content_hash, rviz_display_control::contentHash(last));
  ASSERT_EQ(p.duplicate_ids.size(), 1u);
  EXPECT_EQ(p.duplicate_ids[0], "dup");
  EXPECT_TRUE(p.to_remove.empty());
  EXPECT_TRUE(p.to_update.empty());
}

TEST(Plan, MixedScenario)
{
  // current: A, B (with old config), C (unchanged)
  DisplaySpec spec_a = makeSpec("A", "pkg/A", "", "cfg_a", "");
  DisplaySpec spec_b_old = makeSpec("B", "pkg/B", "", "cfg_b_old", "");
  DisplaySpec spec_c = makeSpec("C", "pkg/C", "", "cfg_c", "");

  std::unordered_map<std::string, std::size_t> current{
    {"A", rviz_display_control::contentHash(spec_a)},
    {"B", rviz_display_control::contentHash(spec_b_old)},
    {"C", rviz_display_control::contentHash(spec_c)},
  };

  // desired: B (config changed), C (unchanged), D (new)
  DisplaySpec spec_b_new = makeSpec("B", "pkg/B", "", "cfg_b_new", "");
  DisplaySpec spec_d = makeSpec("D", "pkg/D", "", "cfg_d", "");
  DisplaySet desired = makeSet({spec_b_new, spec_c, spec_d});

  auto p = rviz_display_control::plan(current, desired);

  ASSERT_EQ(p.to_remove.size(), 1u);
  EXPECT_EQ(p.to_remove[0], "A");

  ASSERT_EQ(p.to_update.size(), 1u);
  EXPECT_EQ(p.to_update[0].spec->id, "B");
  EXPECT_EQ(p.to_update[0].content_hash, rviz_display_control::contentHash(spec_b_new));

  ASSERT_EQ(p.to_create.size(), 1u);
  EXPECT_EQ(p.to_create[0].spec->id, "D");

  EXPECT_TRUE(p.duplicate_ids.empty());
}

TEST(Plan, IdempotencyCreateFromEmpty)
{
  std::unordered_map<std::string, std::size_t> current;
  DisplaySet desired = makeSet({makeSpec("a"), makeSpec("b")});

  auto p1 = rviz_display_control::plan(current, desired);
  auto applied = applyPlan(current, p1);
  auto p2 = rviz_display_control::plan(applied, desired);

  EXPECT_TRUE(p2.to_create.empty());
  EXPECT_TRUE(p2.to_update.empty());
  EXPECT_TRUE(p2.to_remove.empty());
}

TEST(Plan, IdempotencyMixedScenario)
{
  DisplaySpec spec_a = makeSpec("A", "pkg/A", "", "cfg_a", "");
  DisplaySpec spec_b_old = makeSpec("B", "pkg/B", "", "cfg_b_old", "");
  DisplaySpec spec_c = makeSpec("C", "pkg/C", "", "cfg_c", "");

  std::unordered_map<std::string, std::size_t> current{
    {"A", rviz_display_control::contentHash(spec_a)},
    {"B", rviz_display_control::contentHash(spec_b_old)},
    {"C", rviz_display_control::contentHash(spec_c)},
  };

  DisplaySpec spec_b_new = makeSpec("B", "pkg/B", "", "cfg_b_new", "");
  DisplaySpec spec_d = makeSpec("D", "pkg/D", "", "cfg_d", "");
  DisplaySet desired = makeSet({spec_b_new, spec_c, spec_d});

  auto p1 = rviz_display_control::plan(current, desired);
  auto applied = applyPlan(current, p1);
  auto p2 = rviz_display_control::plan(applied, desired);

  EXPECT_TRUE(p2.to_create.empty());
  EXPECT_TRUE(p2.to_update.empty());
  EXPECT_TRUE(p2.to_remove.empty());
}

TEST(Plan, EmptyCurrentEmptyDesiredProducesEmpty)
{
  std::unordered_map<std::string, std::size_t> current;
  DisplaySet desired = makeSet({});
  auto p = rviz_display_control::plan(current, desired);
  EXPECT_TRUE(p.to_create.empty());
  EXPECT_TRUE(p.to_update.empty());
  EXPECT_TRUE(p.to_remove.empty());
  EXPECT_TRUE(p.duplicate_ids.empty());
}

TEST(Plan, CreateOrderIsFirstSeenNotAlphabetical)
{
  std::unordered_map<std::string, std::size_t> current;
  DisplaySet desired = makeSet({makeSpec("z"), makeSpec("m"), makeSpec("a")});
  auto p = rviz_display_control::plan(current, desired);
  ASSERT_EQ(p.to_create.size(), 3u);
  EXPECT_EQ(p.to_create[0].spec->id, "z");
  EXPECT_EQ(p.to_create[1].spec->id, "m");
  EXPECT_EQ(p.to_create[2].spec->id, "a");
}

TEST(Plan, CreateHashMatchesContentHash)
{
  std::unordered_map<std::string, std::size_t> current;
  DisplaySpec s = makeSpec("x", "pkg/X", "a/b", "cfg: 42", "/some/topic");
  DisplaySet desired = makeSet({s});
  auto p = rviz_display_control::plan(current, desired);
  ASSERT_EQ(p.to_create.size(), 1u);
  EXPECT_EQ(p.to_create[0].content_hash, rviz_display_control::contentHash(s));
}

TEST(Plan, MultipleDuplicatesAllRecorded)
{
  std::unordered_map<std::string, std::size_t> current;
  DisplaySpec spec_x1 = makeSpec("x", "pkg/A", "", "cfg1", "");
  DisplaySpec spec_x2 = makeSpec("x", "pkg/B", "", "cfg2", "");
  DisplaySpec spec_y1 = makeSpec("y", "pkg/C", "", "cfg3", "");
  DisplaySpec spec_y2 = makeSpec("y", "pkg/D", "", "cfg4", "");
  DisplaySet desired = makeSet({spec_x1, spec_y1, spec_x2, spec_y2});
  auto p = rviz_display_control::plan(current, desired);
  ASSERT_EQ(p.to_create.size(), 2u);
  ASSERT_EQ(p.duplicate_ids.size(), 2u);
  EXPECT_EQ(p.duplicate_ids[0], "x");
  EXPECT_EQ(p.duplicate_ids[1], "y");
}

TEST(Plan, UpdateHashIsNewNotOld)
{
  DisplaySpec old_s = makeSpec("q", "pkg/Q", "", "old_cfg", "");
  DisplaySpec new_s = makeSpec("q", "pkg/Q", "", "new_cfg", "");
  std::size_t old_h = rviz_display_control::contentHash(old_s);
  std::size_t new_h = rviz_display_control::contentHash(new_s);
  ASSERT_NE(old_h, new_h);

  std::unordered_map<std::string, std::size_t> current{{"q", old_h}};
  DisplaySet desired = makeSet({new_s});
  auto p = rviz_display_control::plan(current, desired);

  ASSERT_EQ(p.to_update.size(), 1u);
  EXPECT_EQ(p.to_update[0].content_hash, new_h);
}

TEST(Plan, IdempotencyDuplicateLastWins)
{
  DisplaySpec first = makeSpec("dup", "pkg/A", "", "cfg: 1", "");
  DisplaySpec last = makeSpec("dup", "pkg/B", "", "cfg: 2", "");
  std::unordered_map<std::string, std::size_t> current;
  DisplaySet desired = makeSet({first, last});

  auto p1 = rviz_display_control::plan(current, desired);
  auto applied = applyPlan(current, p1);
  auto p2 = rviz_display_control::plan(applied, desired);

  EXPECT_TRUE(p2.to_create.empty());
  EXPECT_TRUE(p2.to_update.empty());
  EXPECT_TRUE(p2.to_remove.empty());
}
