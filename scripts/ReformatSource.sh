#!/bin/bash

clang-format -style=file -i \
 ../Intrinsic/src/main.cpp \
 ../IntrinsicEd/src/main.cpp \
 ../IntrinsicCore/src/stdafx.h \
 ../IntrinsicCore/src/IntrinsicCore*.h \
 ../IntrinsicCore/src/IntrinsicCore*.cpp \
 ../IntrinsicCore/src/stdafx_renderer.h \
 ../IntrinsicCore/src/IntrinsicRenderer*.h \
 ../IntrinsicCore/src/IntrinsicRenderer*.cpp \
 ../IntrinsicEd/src/stdafx_editor.h \
 ../IntrinsicEd/src/IntrinsicEd*.h \
 ../IntrinsicEd/src/IntrinsicEd*.cpp \
 ../IntrinsicAssetManagement/src/stdafx_assets.h \
 ../IntrinsicAssetManagement/src/IntrinsicAssetManagement*.h \
 ../IntrinsicAssetManagement/src/IntrinsicAssetManagement*.cpp \
 ../app/assets/shaders/*.glsl \

sleep 1
