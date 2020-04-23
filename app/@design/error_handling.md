Error handling
==============

All error handling will use [MSS](https://github.com/gfannes/gubg.std/src/mss.hpp). If possible, `bool` is used as return type. Only when more detailed error handling is needed will a dedicated `ReturnCode` enum be defined.
