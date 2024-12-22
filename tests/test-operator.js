require('./base.js');
require('../js/operators/operators.js')

// In this test we create a simple operator that returns the sequence
// stored in the first argument. This ensures the sequence properly
// implements the operator argument interface

function testSequenceImplementsOperatorInterface()
{
  var source = quote + "1 2 3" + quote;
  var expected =   [
      { "0/1" : ["1"] },
      { "1/3" : ["2"] },
      { "2/3" : ["3"] },
    ];

  var sequence = evaluator.evaluatePattern(source);

  // Construct an identity function operator that returns the sequence

  var identityFn = function(arguments)
  {
    return arguments[0];
  }

  // Construct the operator and renders it
  var operator = new Operator(identityFn, [sequence]);
  operator.tick();
  var rendered = operator.render();
  testPatternMatches(rendered, expected);
};

////////////////////////////////////////////////////////////////////////////////

function testOperatorsAreRecursive()
{
  var source = quote + "1 2 3" + quote;
  var expected =   [
      { "0/1" : ["1"] },
      { "1/3" : ["2"] },
      { "2/3" : ["3"] },
    ];

  var sequence = evaluator.evaluatePattern(source);

  // Construct an identity function operator that returns the sequence

  var identityFn = function(arguments)
  {
    return arguments[0];
  }

  // Construct inner the operator and wrap it with an outer operator
  var innerOperator = new Operator(identityFn, [sequence]);
  var outerOperator = new Operator(identityFn, [innerOperator]);

  // Make sure rendering the outer operator leads to the expected result
  outerOperator.tick();
  var rendered = outerOperator.render();
  testPatternMatches(rendered, expected);
}

////////////////////////////////////////////////////////////////////////////////

// function testTimelineOperator()
// {
//   var sequence1 = evaluator.evaluatePattern(quote + "1 2 3" + quote);
//   var sequence2 = evaluator.evaluatePattern("slow 2 $ " + quote + "4 5 6 7" + quote);
//
//   var expected1 =   [
//       { "0/1" : ["1"] },
//       { "1/3" : ["2"] },
//       { "2/3" : ["3"] },
//     ];
//
//   var expected2 =   [
//       { "0/1" : ["4"] },
//       { "1/2" : ["5"] },
//       { "1/1" : ["6"] },
//       { "3/2" : ["7"] },
//     ];
//
//   var operator = makeTimelineOperator([sequence1, sequence2]);
//   var testAdvance = function(expected)
//   {
//     testPatternMatches(operator.render(), expected);
//     operator.tick();
//   }
//
//   testAdvance(expected1);
//   testAdvance(expected2);
//   testAdvance(expected1);
// }

////////////////////////////////////////////////////////////////////////////////

function testStrechOperator()
{
  var sequence = evaluator.evaluatePattern(quote + "1 2 3" + quote);

  var expected =   [
      { "0/1" : ["1"] },
      { "2/3" : ["2"] },
      { "4/3" : ["3"] },
    ];

  var operator = makeStrechOperator(sequence, 2);

  operator.tick();
  testPatternMatches(operator.render(), expected);
}

////////////////////////////////////////////////////////////////////////////////

function testShiftOperator()
{
  var sequence = evaluator.evaluatePattern(quote + "1 2 3 4" + quote);

  var expected =   [
      { "0/1" : ["3"] },
      { "1/4" : ["4"] },
      { "1/2" : ["1"] },
      { "3/4" : ["2"] },
    ];

  var operator = makeShiftOperator(sequence, 0.5);

  operator.tick();
  testPatternMatches(operator.render(), expected);
}

////////////////////////////////////////////////////////////////////////////////

function testAddOperator()
{
  var sequence = evaluator.evaluatePattern(quote + "1 2 3 4" + quote);
  var argument = evaluator.evaluatePattern(quote + "0 -2 -4 -8" + quote);

  var expected =   [
      { "0/1" : ["1"] },
      { "1/4" : ["0"] },
      { "1/2" : ["-1"] },
      { "3/4" : ["-4"] },
    ];

  var operator = makeAddOperator(sequence, argument);

  operator.tick();
  testPatternMatches(operator.render(), expected);
}

////////////////////////////////////////////////////////////////////////////////

function testBjorklundOperator()
{
  var sequence = evaluator.evaluatePattern(quote + "bd" + quote);

  var expected =   [
      { "0/1" : ["bd"] },
      { "3/8" : ["bd"] },
      { "3/4" : ["bd"] }
    ];

  var operator = makeBjorklundOperator(sequence, 3 ,8);
  operator.tick();
  testPatternMatches(operator.render(), expected);
}

////////////////////////////////////////////////////////////////////////////////

function testScaleOperator()
{
  var sequence = evaluator.evaluatePattern("'0 7 -2 6'");

  var expected = [
    { "0/1" : ["0"] },
    { "1/4" : ["12"] },
    { "1/2" : ["-3"] },
    { "3/4" : ["11"] }
  ]

  var operator = makeScaleOperator(sequence, "major");
  testPatternMatches(operator.render(), expected);
}

function test()
{
  var sequence = evaluator.evaluatePattern("'0 1 2 3'");

  var expected = [
    { "0/1" : ["0"] },
    { "1/4" : ["1"] },
    { "1/2" : ["2"] }
  ]

  var operator = makeTruncOperator(sequence, 0.75);
  testPatternMatches(operator.render(), expected);
}
////////////////////////////////////////////////////////////////////////////////

testSequenceImplementsOperatorInterface();
testOperatorsAreRecursive();
//testTimelineOperator();
testAddOperator();
testStrechOperator();
testShiftOperator();
testBjorklundOperator();
testScaleOperator();
test();
