# gst-example

[![Build and Run](https://github.com/shalex88/gst-example/actions/workflows/build.yaml/badge.svg)](https://github.com/shalex88/gst-example/actions/workflows/build.yaml)

# Install

```bash
# Install gstreamer
sudo apt -y install pkg-config libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly

# Install gstreamer via vcpkg (not working, gst_element_factory_make() returns NULL)
sudo apt -y install gitlint flex
``` 

# TODO

* Enable/disable specific elements
* Support multiple sinks
* Integrate metadata and extract it