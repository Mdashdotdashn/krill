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
  testSequenceMatches(rendered, expected);
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
  testSequenceMatches(rendered, expected);
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
//     testSequenceMatches(operator.render(), expected);
//     operator.tick();
//   }
//
//   testAdvance(expected1);
//   testAdvance(expected2);
//   testAdvance(expected1);
// }

////////////////////////////////////////////////////////////////////////////////

function testValueWrapperOperator()
{
  var v = 1234;
  var operator = makeValueWrapperOperator(v);
  operator.tick();
  assert.equal(operator.render(), v);
}

////////////////////////////////////////////////////////////////////////////////

function testSlowOperator()
{
  var sequence = evaluator.evaluatePattern(quote + "1 2 3" + quote);

  var expected =   [
      { "0/1" : ["1"] },
      { "2/3" : ["2"] },
      { "4/3" : ["3"] },
    ];

  var operator = makeSlowOperator(sequence, 2);

  operator.tick();
  testSequenceMatches(operator.render(), expected);
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

  var operator = makeBjorklundOperator(sequence, 8 ,3);
  operator.tick();
  testSequenceMatches(operator.render(), expected);
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
  testSequenceMatches(operator.render(), expected);
}

////////////////////////////////////////////////////////////////////////////////
testSequenceImplementsOperatorInterface();
testOperatorsAreRecursive();
//testTimelineOperator();
testValueWrapperOperator();
testSlowOperator();
testBjorklundOperator();
testScaleOperator();
