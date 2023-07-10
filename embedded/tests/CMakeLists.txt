include_directories(
../
../third_party
)

add_executable(Tests.exe
../third_party/FractionClass/Fraction.cpp

../cycle/Cycle.cpp

./test.cpp
./testUtils.cpp

./tst_cycle.cpp
./tst_renderNode.cpp
)
