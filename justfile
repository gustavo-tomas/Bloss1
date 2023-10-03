set positional-arguments

@default: (build "profile") (run "profile")

@build cfg:
  ./vendor/premake/premake5_linux gmake2 && make config=$1 -j4

@run cfg:
  bin/$1/bloss1/bloss1

@clean cfg:
  make clean config=$1
