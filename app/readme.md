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

## TODO

* Split state in {Describing, Designing, Implementing} and {Waiting, WIP, Blocked, Done}
* Remove '@' indications: type is now driven from the markdown documents
* Support for filtering on type
* Type::File for items that are not part of the document
* Support for changing the type: File to something else
* Name embedded items using "#<ix>" as global ix in nodes
* Update all links around a copy-paste
* Support for "dd" and "da"
* Support for markdown titles by adding a delayed processing
  * std::optional<gubg::Strange> prev_line
  * Lambda that uses "is_title"
  * For-loop over all lines
  * Process final empty line if there is something in the prev_line
* Adjust links when renaming a node
* Support removing a link
* Support for showing the content of a link
* Support changing the order for nodes and links
* Compute aggregated cost
  * Convert different costs for different subtrees
* Indication of completed items
* Compute fraction complete
* Support for showing/opening all files
* Support removing a link
* Support for showing the content of a link
* Support for sprints
  * Defining a new sprint
  * Selecting a sprint
  * Adding items to a sprint
  * Jumping to a sprint to find-out what is next to develop
* Add reload command
* Rework `.proast/metadata.naft` format
  * Use `[item]` for MD on nodes
  * Use `[path]` for MD on path
* Annotate `@`-folders and files as requirements
* Add to model how the different paths for a Node will be called
  * `folder` and `index_fn`
* Test if feature with subfeature can be shown
* Support changing the order for nodes and links
* Rework `g` into `gg`
* Git hash header should not require "git checkout ." each time
