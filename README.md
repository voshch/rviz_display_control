# rviz_display_control

Declarative, runtime control of an RViz2 display tree over a topic.

A producer publishes a latched `DisplaySet` describing the displays it wants; the
`Reconciler` panel inside RViz creates, updates, and removes a managed display
subtree to match it, idempotently. Displays the user added by hand are left alone.

## Packages

| Package | Description |
| --- | --- |
| `rviz_display_control` | RViz2 panel plugin (`rviz_common::Panel`) that reconciles the managed subtree. |
| `rviz_display_control_msgs` | `DisplaySpec` / `DisplaySet` message definitions. |

## How it works

`DisplaySet` is the complete desired state for one RViz instance:

- `displays`: a list of `DisplaySpec`, each carrying a stable `id`, an rviz
  `class_id`, a slash-delimited `group_path`, a full rviz display `config` as YAML
  (fed verbatim to `Display::load`), and an optional `require_topic` gate.
- `root_group`: the managed top-level group name, everything outside it is untouched.
- `node_params`: parameters declared on the host node before reconciling (e.g.
  MoveIt `robot_description`).

The producer republishes the whole set on every change; the panel diffs against the
live tree by `id` and content hash, so only changed displays are reloaded.

## Public API

- `rviz_display_control::Reconciler`: the RViz panel, loaded via `pluginlib` as an
  `rviz_common/Panel`.
- The topic contract: a transient-local (latched) subscription on `display_set`
  (`rviz_display_control_msgs/DisplaySet`), plus the host-node parameters declared from
  its `node_params` field.
- `rviz_display_control_msgs/DisplaySpec` and `rviz_display_control_msgs/DisplaySet`,
  documented field-by-field in their `.msg` files.

## Usage

1. Add the **Display Control** panel to RViz (Panels -> Add New Panel).
2. Publish a latched `rviz_display_control_msgs/DisplaySet` on `display_set`
   (the panel's default topic).

## Build

```sh
colcon build --packages-up-to rviz_display_control
```

## Quality

Both packages claim [REP-2004](https://www.ros.org/reps/rep-2004.html) **Quality Level 3**.
See [rviz_display_control/QUALITY_DECLARATION.md](rviz_display_control/QUALITY_DECLARATION.md)
and [rviz_display_control_msgs/QUALITY_DECLARATION.md](rviz_display_control_msgs/QUALITY_DECLARATION.md).

## License

Apache-2.0.
