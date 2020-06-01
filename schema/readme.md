<!--
[proast]
{
  [link](path:/proast/file_format/markdown)
  [link](path:/proast/@design/annotation)
}
-->
Schema to represent data
========================

## Design

### Tree data structure

All `proast` data will be stored in a tree data structure.

## Model

### Node

Base class for the tree, if needed, members can be part of a polymorphic hierarchy.

### Root

The root of the `proast` data structure

### Feature

The Completion of a Feature is the fraction of Requirements that are achieved.

#### Data

* ShortName
* LongName
* Visibility

#### Has

* Requirements
* Design documentation

### Requirement

#### Data

* Id for reference
* MoSCoW priority: Must, Should, Could or Wont
* Status: Open, Done, Unclear

### Sprint

A Sprint is an ordered and prioritized collection of Features, the tasks that should be completed as part of the Sprint. In addition, a Sprint has a TimeSpan attached to it.

The Completion of a Sprint is the Completion of each of its Features, weighted by the estimate of each Feature.