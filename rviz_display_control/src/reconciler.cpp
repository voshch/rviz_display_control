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

#include "rviz_display_control/reconciler.hpp"

#include <QMetaType>
#include <QString>

#include <unordered_map>
#include <vector>

#include "rviz_common/config.hpp"
#include "rviz_common/display.hpp"
#include "rviz_common/display_context.hpp"
#include "rviz_common/display_group.hpp"
#include "rviz_common/yaml_config_reader.hpp"
#include "rviz_display_control/reconcile_core.hpp"

namespace rviz_display_control
{

namespace
{
const QString kGroupClassId = "rviz_common/Group";
const char kDefaultTopic[] = "display_set";
const char kDefaultRootGroup[] = "Arena";
}  // namespace

Reconciler::Reconciler(QWidget * parent)
: rviz_common::Panel(parent), topic_(kDefaultTopic), root_group_(kDefaultRootGroup)
{
  // Required so the parsed message survives the queued cross-thread connection.
  qRegisterMetaType<rviz_display_control_msgs::msg::DisplaySet>(
    "rviz_display_control_msgs::msg::DisplaySet");
}

Reconciler::~Reconciler() = default;

void Reconciler::onInitialize()
{
  node_ptr_ = getDisplayContext()->getRosNodeAbstraction().lock();
  node_ = node_ptr_->get_raw_node();

  connect(
    this, &Reconciler::displaySetReceived,
    this, &Reconciler::reconcile, Qt::QueuedConnection);

  start();
}

void Reconciler::load(const rviz_common::Config & config)
{
  rviz_common::Panel::load(config);

  QString value;
  if (config.mapGetString("Topic", &value)) {
    topic_ = value.toStdString();
  }
  if (config.mapGetString("RootGroup", &value)) {
    root_group_ = value.toStdString();
  }

  // rviz calls load after onInitialize, so re-bind to the persisted topic and
  // root group once they are known.
  if (node_) {
    start();
  }
}

void Reconciler::start()
{
  // Empty any persisted managed children so a saved .rviz never leaves stale
  // displays before the first message arrives.
  clearManaged();
  subscribe();
}

void Reconciler::save(rviz_common::Config config) const
{
  rviz_common::Panel::save(config);
  config.mapSetValue("Topic", QString::fromStdString(topic_));
  config.mapSetValue("RootGroup", QString::fromStdString(root_group_));
}

void Reconciler::subscribe()
{
  rclcpp::QoS qos(rclcpp::KeepLast(1));
  qos.transient_local();
  sub_ = node_->create_subscription<rviz_display_control_msgs::msg::DisplaySet>(
    topic_, qos,
    [this](const rviz_display_control_msgs::msg::DisplaySet::SharedPtr msg) {
      // Off the GUI thread: hand the parsed message across via a queued signal.
      Q_EMIT displaySetReceived(*msg);
    });
}

void Reconciler::applyNodeParams(const std::vector<rcl_interfaces::msg::Parameter> & params)
{
  for (const auto & pm : params) {
    const rclcpp::Parameter p = rclcpp::Parameter::from_parameter_msg(pm);
    try {
      if (node_->has_parameter(p.get_name())) {
        node_->set_parameter(p);
      } else {
        node_->declare_parameter(p.get_name(), p.get_parameter_value());
      }
    } catch (const std::exception & e) {
      RCLCPP_WARN(
        node_->get_logger(), "failed to apply node param '%s': %s",
        p.get_name().c_str(), e.what());
    }
  }
}

rviz_common::DisplayGroup * Reconciler::ensureRoot()
{
  rviz_common::DisplayGroup * root = getDisplayContext()->getRootDisplayGroup();
  const QString name = QString::fromStdString(root_group_);

  rviz_common::DisplayGroup * managed = findChildGroup(root, name);
  if (managed) {
    return managed;
  }

  rviz_common::Config cfg;
  cfg.mapSetValue("Name", name);
  rviz_common::Display * created = createDisplay(root, kGroupClassId, cfg);
  return qobject_cast<rviz_common::DisplayGroup *>(created);
}

void Reconciler::clearManaged()
{
  rviz_common::DisplayGroup * root = getDisplayContext()->getRootDisplayGroup();
  rviz_common::DisplayGroup * managed =
    findChildGroup(root, QString::fromStdString(root_group_));
  if (managed) {
    managed->removeAllDisplays();
  }
  live_.clear();
}

rviz_common::DisplayGroup * Reconciler::findChildGroup(
  rviz_common::DisplayGroup * parent, const QString & name) const
{
  const int count = parent->numDisplays();
  for (int i = 0; i < count; ++i) {
    rviz_common::DisplayGroup * group = parent->getGroupAt(i);
    if (group && group->getName() == name) {
      return group;
    }
  }
  return nullptr;
}

rviz_common::Display * Reconciler::createDisplay(
  rviz_common::DisplayGroup * parent,
  const QString & class_id,
  const rviz_common::Config & config)
{
  rviz_common::Display * display = parent->createDisplay(class_id);
  parent->addDisplay(display);
  display->initialize(getDisplayContext());
  display->load(config);
  // Enabled by default, only an explicit Enabled false in the config disables it.
  bool enabled = true;
  config.mapGetBool("Enabled", &enabled);
  display->setEnabled(enabled);
  return display;
}

rviz_common::DisplayGroup * Reconciler::ensureGroupPath(
  rviz_common::DisplayGroup * parent, const std::string & group_path)
{
  rviz_common::DisplayGroup * current = parent;
  for (const std::string & segment : splitPath(group_path)) {
    const QString name = QString::fromStdString(segment);
    rviz_common::DisplayGroup * child = findChildGroup(current, name);
    if (!child) {
      rviz_common::Config cfg;
      cfg.mapSetValue("Name", name);
      rviz_common::Display * created = createDisplay(current, kGroupClassId, cfg);
      child = qobject_cast<rviz_common::DisplayGroup *>(created);
    }
    if (!child) {
      return nullptr;
    }
    current = child;
  }
  return current;
}

void Reconciler::pruneEmptyGroups(rviz_common::DisplayGroup * group)
{
  for (int i = group->numDisplays() - 1; i >= 0; --i) {
    rviz_common::DisplayGroup * child = group->getGroupAt(i);
    if (!child) {
      continue;
    }
    pruneEmptyGroups(child);
    if (child->numDisplays() == 0) {
      group->takeDisplay(child);
      delete child;
    }
  }
}

void Reconciler::reconcile(const rviz_display_control_msgs::msg::DisplaySet & set)
{
  if (!set.root_group.empty()) {
    root_group_ = set.root_group;
  }
  applyNodeParams(set.node_params);
  rviz_common::DisplayGroup * managed = ensureRoot();

  std::unordered_map<std::string, std::size_t> current;
  current.reserve(live_.size());
  for (const auto & [id, managed_display] : live_) {
    current.emplace(id, managed_display.content_hash);
  }

  const ReconcilePlan plan_result = plan(current, set);

  for (const auto & id : plan_result.duplicate_ids) {
    RCLCPP_WARN(node_->get_logger(), "duplicate display id '%s', last one wins", id.c_str());
  }

  for (const auto & id : plan_result.to_remove) {
    auto it = live_.find(id);
    if (it == live_.end()) {
      continue;
    }
    rviz_common::DisplayGroup * owner =
      qobject_cast<rviz_common::DisplayGroup *>(it->second.display->getParent());
    if (owner) {
      owner->takeDisplay(it->second.display);
    }
    delete it->second.display;
    live_.erase(it);
  }

  for (const auto & planned : plan_result.to_create) {
    const auto & spec = *planned.spec;
    rviz_common::Config cfg;
    rviz_common::YamlConfigReader reader;
    reader.readString(cfg, QString::fromStdString(spec.config));
    if (reader.error()) {
      RCLCPP_WARN(
        node_->get_logger(), "malformed config for display id '%s', skipping: %s",
        spec.id.c_str(), reader.errorMessage().toStdString().c_str());
      continue;
    }
    rviz_common::DisplayGroup * group = ensureGroupPath(managed, spec.group_path);
    if (!group) {
      continue;
    }
    rviz_common::Display * leaf =
      createDisplay(group, QString::fromStdString(spec.class_id), cfg);
    // FailedDisplay (the bad-class_id sentinel) is a private type, detect it by
    // its description prefix rather than depending on the unexported class.
    if (leaf->getDescription().startsWith("The class required for this display")) {
      RCLCPP_WARN(
        node_->get_logger(), "bad class_id '%s' for display id '%s', skipping",
        spec.class_id.c_str(), spec.id.c_str());
      rviz_common::DisplayGroup * owner =
        qobject_cast<rviz_common::DisplayGroup *>(leaf->getParent());
      if (owner) {
        owner->takeDisplay(leaf);
      }
      delete leaf;
      continue;
    }
    live_[spec.id] = Managed{leaf, planned.content_hash};
  }

  for (const auto & planned : plan_result.to_update) {
    const auto & spec = *planned.spec;
    rviz_common::Config cfg;
    rviz_common::YamlConfigReader reader;
    reader.readString(cfg, QString::fromStdString(spec.config));
    if (reader.error()) {
      RCLCPP_WARN(
        node_->get_logger(), "malformed config for display id '%s', skipping: %s",
        spec.id.c_str(), reader.errorMessage().toStdString().c_str());
      continue;
    }
    bool enabled = true;
    cfg.mapGetBool("Enabled", &enabled);
    auto it = live_.find(spec.id);
    it->second.display->load(cfg);
    it->second.display->setEnabled(enabled);
    it->second.content_hash = planned.content_hash;
  }

  pruneEmptyGroups(managed);
}

}  // namespace rviz_display_control

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(rviz_display_control::Reconciler, rviz_common::Panel)
