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

* Add support for creating files via `a`
* Rework `g` into `gg`
* Reload tree from time to time
* Reload subtree after edit
* Store metadata in `<!--[proast](a:)(b:)(c:)-->`
* Store `active_ix` in .proast folder
* Rename `@root` into `.proast`
* Load `@`-folders as well and annotate as requirements
* Add to model how the different paths for a Node will be called
* Test if feature with subfeature can be shown
