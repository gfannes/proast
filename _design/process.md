Process
=======

## Input
### Info
#### Roles
#### Resources
### Use cases
#### Actor
### Requirements
#### MoSCoW

## System
### Domain
What should be in the domain model? Only a model of the domain, or also an object model of the solution? If so, this should probably be placed after the design.
#### Tags
#### Model
Maybe a functional and non-functional design would be better than architectural and design
Or simply use the `system/design` or `system/blueprint` folder without more hierarchy
### Architecture (structure)
Architecture is on a higher level than design, and more hierarchical in nature
Partitions the solution in major regions of functionality, interface, protocols, ...
Decisions that are difficult to change later
Architecture is a plan for the structure of something
### Design
Design is a plan on how to create something
Clarifies _how_ the architectural decisions will be used to achieve the requirements
TODO: try to merge architecture and design, or create a clear distinction between them

## Features
### Area
#### Capability

## Planning
### Sprint

## Report

input
  roles
  resources
  info
    <subfolders>/<file>
  requirements
    <subfolders>/<file>
system
  tags
  model
    <namespace>/<entity>
  design
    <subfolders>/<file>
features
  <area>
    <capability>
      <feature>
planning
  prio
  sprints
    
report


Focus on steps that are needed at all levels

root
description
info
requirements
design
model
plan
validation
