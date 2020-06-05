# Store data in a file system for a project

## Requirements

### Must

* define in detail how project data will be stored in a file system to leverage
  * Version control using source-code version systems
  * Text-based search
  * Visualization and navigation with common [text editors](https://neovim.io/) and [file managers](https://github.com/ranger/ranger)
  * Refactoring

## Design

Where possible, information will represented in a file system with a hierarchy as similar as possible as the structure of the project data. Human-readable descriptions will use [markdown](/home/geertf/gubg/gubg.io/src/gubg/parse/naft/spec.md) and well-structured data will use [.naft](http://github.com/gfannes/gubg.io/src/gubg/parse/naft/spec.md), either directly, or embedded in a markdown file.

### Annotation

Embedding such annotation data will use a [fenced code block](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet#code) at the start or bottom of the markdown file without language specification, and use following schema:

```
[proast]{
    [status](open|done|blocked)
}
```

## Model

### Root

To identify the Root Node of the `proast` tree

### Feature

#### ShortName

* ShortName is the folder name or base name when no folder is present

#### LongName

* Optional, when not present, equal to ShortName
* LongName is the title found in 'ShortName/readme.md' or 'ShortName.md'

### Sprint

Will be stored in a [.naft](http://github.com/gfannes/gubg.io/src/gubg/parse/naft/spec.md)-formatted file with following schema:

```
[sprint]{
    [goal]{<description of the goal of this sprint>}
    [features]{
        [<path to feature>]
        [<path to feature>]
        [<path to feature>]
        [<path to feature>]
    }
}
```

```
[data]{
    [abc](aa::bb)
    [jaja]{abc}
}
```
