Design decisions
================

## Programming language

### Investigate

* How can neovim be loaded/embedded?

### C++

* Pro
  * Most experience
  * Fastest
  * No dependencies
* Con
  * Probably requires separate Python tool to sync with JIRA
  * Difficult to extend or tweak, might require ChaiScript for tweaking

### Python

* Pro
  * Does work with JIRA
  * Might be possible to reuse parts of [ranger](https://github.com/ranger/ranger)
* Con
  * Rather slow to start
  * Depends on availability of Python

## Annotation of Items

* Via adding MetaData in the Markdown files
  * Pro
    * Easy to copy-paste paths from attachments etc

## Logging

* Termbox in full-screen mode should not be combined with `stdout`.
