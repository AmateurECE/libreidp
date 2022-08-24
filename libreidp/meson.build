###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Build script for the libreidp executable
#
# CREATED:          08/16/2022
#
# LAST EDITED:      08/24/2022
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
if not compiler.has_function('strdup', prefix: '''#include <string.h>''')
  error('Compiler does not provide strdup(3)')
endif

dl = dependency('dl')

executable(
  'libreidp',
  sources: [
    'source/main.c',
    'source/plugin-resolver.c',
    'source/plugin-loader.c',
    'source/core-impl/http.c',
  ],
  include_directories: ['include', '../libreidp-runtime/include'],
  dependencies: [dl, libreidp_runtime],
)

###############################################################################
