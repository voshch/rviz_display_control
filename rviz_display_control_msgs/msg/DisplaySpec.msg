# One declarative rviz display, rendered by the producer.
string id            # stable reconcile key, producer-assigned
string class_id      # rviz display class lookup name, e.g. rviz_default_plugins/LaserScan
string group_path    # slash-delimited nesting under root_group, empty for root
string config        # full rviz display config as YAML (Class, Name, Enabled, properties), fed verbatim to Display::load
string require_topic # topic that must exist before the display is enabled, empty for no gate
