include_directories(
../
)

add_executable(Tests.exe
../third_party/FractionClass/Fraction.cpp

../cycle/Cycle.cpp
../renderer/RenderTreeBuilder.cpp

./test.cpp
./testUtils.cpp

./tst_cycle.cpp
./tst_renderNode.cpp
./tst_run_cases.cpp
)
