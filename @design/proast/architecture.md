Architecture for the proast app
===============================

## Model - View - Presenter

The proast app will follow the [model-view-presenter](https://en.wikipedia.org/wiki/Model%E2%80%93view%E2%80%93presenter) design pattern:

### Model

Only holds data and is unaware of the details how this is displayed. As such, the model is independent of any TUI framework.

### View

A passive view, only concerned with displaying items on the screen. No logic should be present here.

### Presenter

Interacts with the model and view; the presenter acts on model updates and formats the data to-be-displayed by the view.
