include_directories(
../
../third_party
)

add_executable(Tests.exe
./test.cpp
./tst_cycle.cpp
./tst_renderNode.cpp
)
