This document is a declaration of software quality for the `rviz_display_control_msgs` package, based on the guidelines in [REP-2004](https://www.ros.org/reps/rep-2004.html).

# rviz_display_control_msgs Quality Declaration

The package `rviz_display_control_msgs` claims to be in the **Quality Level 3** category.

Below are the rationales, notes, and caveats for this claim, organized by each requirement listed in the Package Requirements for Quality Level 3 in REP-2004.

## Version Policy [1]

### Version Scheme [1.i]
`rviz_display_control_msgs` uses [`semver`](https://semver.org/).

### Version Stability [1.ii]
`rviz_display_control_msgs` is at a version below `1.0.0`. A stable version is not required at Quality Level 3.

### Public API Declaration [1.iii]
The public API is the set of message definitions:
- `rviz_display_control_msgs/DisplaySpec`: one declarative rviz display.
- `rviz_display_control_msgs/DisplaySet`: the complete desired display set for one rviz instance.

The generated language bindings follow from these `.msg` files.

### API and ABI Stability [1.iv] - [1.vii]
Message-definition stability is not guaranteed before version `1.0.0`, and is not required at Quality Level 3.

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
Changes to the message definitions are reflected in the field comments and in the repository `README.md`.

## Documentation [3]

### Feature Documentation [3.i]
Features are documented in the repository [`README.md`](../README.md).

### Public API Documentation [3.ii]
Every field of every message is documented with an inline comment in its `.msg` file.

### License [3.iii]
The license is Apache-2.0, declared in `package.xml` and reproduced in [`LICENSE`](LICENSE).

### Copyright Statement [3.iv]
Copyright is held by `voshch`, stated in the appendix of [`LICENSE`](LICENSE).

## Testing [4]
`rviz_display_control_msgs` is an interface-only package; its bindings are generated and exercised by `rosidl` at build time. Quality Level 3 imposes no testing requirement.

## Dependencies [5]

### Direct Runtime ROS Dependencies [5.i]
- `rcl_interfaces` (Quality Level 1)
- `rosidl_default_runtime` (Quality Level 1)

All direct runtime dependencies are Quality Level 1.

## Platform Support [6]
`rviz_display_control_msgs` targets the Tier 1 platforms of its ROS distributions (Ubuntu 24.04 Noble), verified in CI for `jazzy`, `kilted`, and `lyrical`.

## Security [7]

### Vulnerability Disclosure Policy [7.i]
A vulnerability disclosure policy is published in [`SECURITY.md`](../SECURITY.md), directing private reports to the maintainer.
