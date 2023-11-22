set positional-arguments

@default: (lint) (format) (build "profile") (run "profile")

@debug:   (lint) (format) (build "debug") (run "debug")
@profile: (lint) (format) (build "profile") (run "profile")
@release: (lint) (format) (build "release") (run "release")

@lint:
  cppcheck --enable=warning,performance,portability,style,information --disable=missingInclude --std=c++20 bloss1/src/**

@format:
  find bloss1/src/ -iname *.hpp -o -iname *.cpp | xargs clang-format -i -style=file

@build cfg:
  ./vendor/premake/premake5_linux gmake2 && make config=$1 -j4

@run cfg:
  bin/$1/bloss1/bloss1

@clean cfg:
  make clean config=$1
