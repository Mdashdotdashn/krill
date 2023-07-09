#! /bin/bash
export unamestr=`uname`
if [ "$unamestr" == "Linux" ]; then
  export platform='Linux'
elif [ "$MSYSTEM" == "MINGW64" ]; then
  export platform='Windows'
fi

rm -rf build >/dev/null 2>&1
mkdir build >/dev/null 2>&1
cd build
echo "kicking cmake"
if [ "$platform" == "Windows" ]; then
	cmake .. -G "Visual Studio 16 2019" -DCMAKE_INSTALL_PREFIX=. 
else
	cmake .. -DCMAKE_INSTALL_PREFIX=.
fi
echo "ready for build"
