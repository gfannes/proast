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


## TUI framework

### [notcurses](https://nick-black.com/dankwiki/index.php/Notcurses)

Install via `yay notcurses`.
Clone frome [github](https://github.com/dankamongmen/notcurses) and building works.

* Pro
  * Seems the most advanced
  * Has [nice documentation](https://nick-black.com/htp-notcurses.pdf)
* Con
  * No windows support

### [ncurses](https://invisible-island.net/ncurses/announce.html)

### [final cut](https://github.com/gansm/finalcut)

Install via `yay final cut` and link with `-lfinal`

* Con
  * Seems to add windows border around everything


## UI Pattern to use

### [Model-View-Presenter](https://en.wikipedia.org/wiki/Model%E2%80%93view%E2%80%93presenter)

* Passive View

## Annotation of Items

* Via adding MetaData in the Markdown files
  * Pro
    * Easy to copy-paste paths from attachments etc

## Logging

* Notcurses in full-screen mode should not be combined with `stdoud`.
