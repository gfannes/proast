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

* Add support for rename using `r`
* Adding an item
  * Start with `file.md`
  * Convert to `file/index.md` when subitems are added
  * Create `file/index.md` when it does not exist yet and _commander_open()_ is called
* Add reload command
* Rework `.proast/metadata.naft` format
  * Use `[item]` for MD on nodes
  * Use `[path]` for MD on path
* Annotate `@`-folders and files as requirements
* Store metadata in `<!--[proast](a:)(b:)(c:)-->`
* Add to model how the different paths for a Node will be called
  * `folder` and `index_fn`
* Test if feature with subfeature can be shown
* Rework `g` into `gg`
* Git hash header should not require "git checkout ." each time
