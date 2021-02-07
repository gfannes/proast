Features for proast
===================

## Navigation

* Store/load child navigation in `config` folder
* Store/load `current_node_` in `config` folder

## Jump list

* Keep track of jump locations after `jump to bookmark`
* Register jump location on `m\n`
* Use `-` to jump to last jump location, adding current to list
* Use `ctrl-o` to jump down to jump location list
* Show jump location list in BottomLeft

## ui::List

* Show name in border
* Support for colors

## Metadata

* Show complexity as total amount of dependencies
* `Order`: `Random` or `Ordered`
* `Status`: `Open`, `Design`, `Estimated`, `InProgress`, `Done`

## Help

* Show help on `:help`

## Prio

* prio = age_days*pow(10, total_volume_db/20)*impact/effort
* total_volume_db = volume_db + 6*max(overdue_days+7, 0)

## Create

* `c[fd][in]`: Create file/directory inside/next current folder or file:
  * When creating in a file, replace `file.ext` with `file/index.ext`
  * Creating next to a file/directory creates in the parent

## Deleting

* `dd`: Delete single file or folder. When deleting a folder, all its childs become member of the parent.
* `dD`: Delete file or folder recursively

## Tree

* (/) Use `std::shared_ptr`-based tree implementation to allow reshaping the tree
* (/) Each node stores a part of a path, the full path is the combination of all
  * (/) `Base` node has absolute path, all the rest uses relative paths
* Support for using a different name for a `base` node

## Dependencies

* All dependencies for aggregating tags
* Tag-filtered dependencies for influencing the effort aggregation

## Windows

* `w[abcmd]`: Switches to a different window
  * `a`: parent
  * `b`: self
  * `c`: child
  * `m`: metadata
  * `d`: details

## UI

* Support for colored output with `ui::List`

## Planning

* Select some tags
* Show topological ordering of subtree where all 0-effort is removed in `details` window
  * Sort by dependency and priority

## File types

* Find way to store metadata per file type

### Markdown

* Find way to indicate `Done` for list item
