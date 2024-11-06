include_directories(
../
)

add_executable(Tests.exe
../third_party/FractionClass/Fraction.cpp

../cycle/Cycle.cpp
../parser/Context.cpp
../parser/Parser.cpp
../parser/StringStream.cpp
../parser/Rules.cpp
../renderer/RenderTreeBuilder.cpp

./test.cpp
./testUtils.cpp

./tst_cycle.cpp
./tst_parser.cpp
./tst_renderNode.cpp
./tst_stringStream.cpp
./tst_run_cases.cpp
)
