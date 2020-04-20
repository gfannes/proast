Store data in a filesystem for a project
========================================

## Requirements

### Must

* define in detail how project data will be stored in a filesystem to leverage
  * Version control using source-code versioning systems
  * Text-based search
  * Visualization and navigation with common [text editors](https://neovim.io/) and [file managers](https://github.com/ranger/ranger)
  * Refactoring

## Model

### Root

To identify the Root Node of the `proast` tree

### Feature

#### ShortName

* ShortName is the folder name or basename when no folder is present

#### LongName

* Optional, when not present, equal to ShortName
* LongName is the title found in 'ShortName/readme.md' or 'ShortName.md'
