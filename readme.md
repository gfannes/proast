# ProAst

Proast is a project assistant.

## Overview

It currently provides:

* A specification of adding metadata to a file system tree in a non-intrusive manner to support project management. Metadata items are:
  * Effort
  * Completion percentage
  * Deadline
  * Tags
  * Inter-dependencies
  * ...
  This metadata is stored in the root of the file system tree and can easily be versionned with a source code versionning system like [git](https://git-scm.com).
* A terminal application that can
  * Update the metadata
  * Rework and refine the file system tree containing the project information
  * Export to various formats
  * Assist with tracking priorities
  * Perform basic planning
* Best practices on how ProAst can be used
  * In a professional environment to provide project management
  * At home to manage personal TODO's and store heterogeneous information
