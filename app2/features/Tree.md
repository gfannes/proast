Tree
====

## Storage

* (/) Use `std::shared_ptr`-based tree implementation to allow reshaping the tree
* (/) Each node stores a part of a path, the full path is the combination of all
  * (/) `Base` node has absolute path, all the rest uses relative paths
* Support for using a different name for a `base` node

## Dependencies

* All dependencies for aggregating tags
* Tag-filtered dependencies for influencing the effort aggregation
