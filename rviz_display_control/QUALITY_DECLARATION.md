This document is a declaration of software quality for the `rviz_display_control` package, based on the guidelines in [REP-2004](https://www.ros.org/reps/rep-2004.html).

# rviz_display_control Quality Declaration

The package `rviz_display_control` claims to be in the **Quality Level 3** category.

Below are the rationales, notes, and caveats for this claim, organized by each requirement listed in the Package Requirements for Quality Level 3 in REP-2004.

## Version Policy [1]

### Version Scheme [1.i]
`rviz_display_control` uses [`semver`](https://semver.org/).

### Version Stability [1.ii]
`rviz_display_control` is at a version below `1.0.0`. A stable version is not required at Quality Level 3.

### Public API Declaration [1.iii]
The public API consists of:
- the `rviz_display_control::Reconciler` RViz panel, exported through `pluginlib` as an `rviz_common/Panel` by `plugin.xml`;
- the topic contract: a transient-local (latched) subscription on `display_set` of type `rviz_display_control_msgs/DisplaySet`, and the host-node parameters declared from its `node_params` field.

The anonymous-namespace helpers in `src/reconciler.cpp` are implementation detail and not part of the public API.

### API and ABI Stability [1.iv] - [1.vii]
API and ABI stability is not guaranteed before version `1.0.0`, and is not required at Quality Level 3.

## Change Control Process [2]

### Change Requests [2.i]
All changes occur through a pull request on [GitHub](https://github.com/voshch/rviz_display_control).

### Contributor Origin [2.ii]
A formal confirmation of contributor origin (e.g. DCO) is not enforced. This is not required at Quality Level 3.

### Peer Review Policy [2.iii]
A formal peer-review policy is not enforced (single maintainer). This is not required at Quality Level 3.

### Continuous Integration [2.iv]
All pull requests are built and tested by GitHub Actions (`.github/workflows/ci.yaml`) on the `jazzy`, `kilted`, and `lyrical` distributions.

### Documentation Policy [2.v]
Changes affecting the public API are reflected in the `README.md` and in the message field comments.

## Documentation [3]

### Feature Documentation [3.i]
Features are documented in the repository [`README.md`](../README.md).

### Public API Documentation [3.ii]
The panel behavior and topic contract are documented in `README.md`; the message contract is documented inline in the `.msg` field comments of `rviz_display_control_msgs`.

### License [3.iii]
The license is Apache-2.0, declared in `package.xml` and reproduced in [`LICENSE`](LICENSE).

### Copyright Statement [3.iv]
Copyright is held by `voshch`, stated in the appendix of [`LICENSE`](LICENSE).

## Testing [4]
`rviz_display_control` has no automated tests; Quality Level 3 imposes no testing requirement. The package builds with `-Wall -Wextra -Wpedantic`, which serves as its static-analysis baseline.

## Dependencies [5]

### Direct Runtime ROS Dependencies [5.i]
- `rviz_display_control_msgs` (Quality Level 3, this repository)
- `rclcpp` (Quality Level 1)
- `rcl_interfaces` (Quality Level 1)
- `pluginlib` (see its own Quality Declaration)
- `rviz_common` (no Quality Declaration published)
- `rviz_default_plugins` (no Quality Declaration published)

Quality Level 3 permits dependencies below Level 3. `rviz_common` and `rviz_default_plugins` do not publish Quality Declarations; this is disclosed rather than treated as a blocker.

### Direct Runtime non-ROS Dependencies [5.iii]
- `qtbase5-dev` (Qt 5), a mature, widely used system dependency.

## Platform Support [6]
`rviz_display_control` targets the Tier 1 platforms of its ROS distributions (Ubuntu 24.04 Noble), verified in CI for `jazzy`, `kilted`, and `lyrical`.

## Security [7]

### Vulnerability Disclosure Policy [7.i]
A vulnerability disclosure policy is published in [`SECURITY.md`](../SECURITY.md), directing private reports to the maintainer.
