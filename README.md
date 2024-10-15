# gst-example

[![Build and Run](https://github.com/shalex88/gst-example/actions/workflows/build.yaml/badge.svg)](https://github.com/shalex88/gst-example/actions/workflows/build.yaml)

# Install

```bash
# Install gstreamer
sudo apt -y install pkg-config libgstreamer1.0-dev

# Install gstreamer via vcpkg (not working, gst_element_factory_make() returns NULL)
sudo apt -y install pkg-config bison flex nasm 
``` 

# TODO

* Enable/disable specific elements
* Support multiple sinks
* Embed metadata and extract it