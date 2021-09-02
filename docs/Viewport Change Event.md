# Viewport Change Event

## Method 1
OpenGL Viewport Size Changed Callback
(Might not exist?)

## Method 2
Everytime glViewport() gets called dispatch an event
-> Would likely need a global Event Dispatcher (`Application::Get()->DispatchEvent()`?)