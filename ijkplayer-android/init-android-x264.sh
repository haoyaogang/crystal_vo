#! /usr/bin/env bash
#
# Copyright (C) 2013-2014 Zhang Rui <bbcallen@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

IJK_X264_UPSTREAM=https://github.com/mirror/x264.git
IJK_X264_FORK=https://github.com/mirror/x264.git
#IJK_OPENSSL_COMMIT=OpenSSL_1_0_1i
IJK_X264_LOCAL_REPO=extra/x264

set -e
TOOLS=tools

echo "== pull x264 base =="
sh $TOOLS/pull-repo-base.sh $IJK_X264_UPSTREAM $IJK_X264_LOCAL_REPO

function pull_fork()
{
    echo "== pull x264 fork $1 =="
    sh $TOOLS/pull-repo-ref.sh $IJK_X264_FORK android/x264-$1 ${IJK_X264_LOCAL_REPO}
    cd android/x264-$1
    #git checkout ${IJK_OPENSSL_COMMIT}
    cd -
}

pull_fork "armv7a"
pull_fork "armv5"
pull_fork "x86"
