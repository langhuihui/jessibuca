#!/usr/bin/env bash

set -x
npm run build
mv dist/* ../demo/public