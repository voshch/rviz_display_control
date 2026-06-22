# Complete desired display set for one rviz instance, reconciled idempotently.
rviz_display_control_msgs/DisplaySpec[] displays
string root_group # managed top-level group name, default Arena (producer always sets this)
rcl_interfaces/Parameter[] node_params # params declared on the host node before reconciling, e.g. MoveIt robot_description
