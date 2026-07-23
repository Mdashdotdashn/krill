include_directories(
../
../third_party/cpp-peglib
)

add_executable(Tests.exe
../third_party/FractionClass/Fraction.cpp

../cycle/Cycle.cpp
../parser/Context.cpp
../parser/KrillParser.cpp
../parser/Parser.cpp
../parser/StringStream.cpp
../parser/XmlBuilder.cpp
../renderer/RenderTreeBuilder.cpp
../harmony/core/NoteMidi.cpp
../harmony/core/Interval.cpp
../harmony/theory/Scale.cpp
../harmony/theory/Roman.cpp

./test.cpp
./testUtils.cpp

./tst_cycle.cpp
./tst_parser.cpp
./tst_stringStream.cpp
./tst_run_cases.cpp
./tst_ast_cases.cpp
./tst_peglib_smoke.cpp
./tst_peglib_primitives.cpp
./tst_peglib_mininotation.cpp
./tst_peglib_operators.cpp
./tst_peglib_statement.cpp
./tst_harmony_noteMidi.cpp
./tst_harmony_interval.cpp
./tst_harmony_scale.cpp
./tst_harmony_roman.cpp
)
