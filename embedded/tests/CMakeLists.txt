include_directories(
../
../third_party/cpp-peglib
)

add_executable(Tests.exe
../third_party/FractionClass/Fraction.cpp

../cycle/Cycle.cpp
../parser/Context.cpp
../parser/MiniNotation.cpp
../parser/Parser.cpp
../parser/StringStream.cpp
../parser/XmlBuilder.cpp
../renderer/RenderTreeBuilder.cpp

./test.cpp
./testUtils.cpp

./tst_cycle.cpp
./tst_parser.cpp
./tst_renderNode.cpp
./tst_stringStream.cpp
./tst_run_cases.cpp
./tst_peglib_smoke.cpp
./tst_peglib_primitives.cpp
./tst_peglib_mininotation.cpp
./tst_peglib_operators.cpp
)
