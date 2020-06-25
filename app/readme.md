<!--
[proast]
-->
Text-UI application
===================

## Requirements

### Must

* allow easy navigation and inspection of a project tree
  * fast
  * VI keybindings
  * preview support
* allow easy search of a project tree
  * links, filtering
* allow easy extension and rework of a project tree
  * different modes corresponding to the different phases of project execution

### Should

* allow creation of project documentation as a `.pdf` file
* allow easy reordering of sprints by changing the rang index of a sprint and regenerate the rang indices with a stride of 10

## Features

* Add help command
  * Support for showing formatted text in a dialog
* [external](key:support_sorting_based_on_state)
* [external](key:when_adding_an_item,_add_it_right_behind_the_current_location)
* [external](key:do_not_fail_on_unfound_not_after_opening_a_document)
* [external](key:apply_all_changes_via_oo)
* Support for using keys containing '/'
* Add Type::Info to state information
* Support for more than one Config.index_name()
* Support for moving an item in
* Support for filtering what is shown
  * Make sure that navigation still works correctly
* Support for changing from embedded to non-embedded: maybe with renaming it?
* Check if it is a good idea to have both File and Directory: this gives problems with changing Item Type
* Split state in {Describing, Designing, Implementing} and {Waiting, WIP, Blocked, Done}
* Remove empty folders from time to time
* Support for filtering on type
* Introduce the concept of physical and embedded items
* Update all links around a copy-paste
  * Only paste in physical items
* Support for "dd" and "da"
* Adjust links when renaming a node
* Support removing a link
* Support for showing the content of a link
* Compute aggregated cost
  * Convert different costs for different subtrees
* Indication of completed items
* Compute fraction complete
* Support for sprints
  * Defining a new sprint
  * Selecting a sprint
  * Adding items to a sprint
  * Jumping to a sprint to find-out what is next to develop
* Add reload command
* Rework `.proast/metadata.naft` format
  * Use `[item]` for MD on nodes
  * Use `[path]` for MD on path
* Test if feature with subfeature can be shown
* Rework `g` into `gg`
* Git hash header should not require "git checkout ." each time
* [external](key:help)
* [external](key:bugs)
