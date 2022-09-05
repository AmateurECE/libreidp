###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Build script for the libreidp-runtime library.
#
# CREATED:          08/17/2022
#
# LAST EDITED:      09/05/2022
#
# Copyright 2022, Ethan D. Twardy
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
###

compiler = meson.get_compiler('c')
if not compiler.has_header('uv.h')
  error('libuv is not installed')
endif

libreidp_runtime_include_directories = include_directories('include')

libreidp_runtime_library = shared_library(
  'reidp-runtime',
  sources: [
    'source/core/http.c',
    'source/plugin.c',
    'source/vector.c',
  ],
  soversion: meson.project_version(),
  include_directories: ['include'],
)

libreidp_runtime = declare_dependency(
  link_with: libreidp_runtime_library,
  include_directories: libreidp_runtime_include_directories
)

###############################################################################
