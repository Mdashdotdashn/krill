const math = require("mathjs");
require("./op-pattern.js");

function bjorklund(steps, pulses) {

	steps = Math.round(steps);
	pulses = Math.round(pulses);

	if(pulses > steps || pulses == 0 || steps == 0) {
		return new Array();
	}

	pattern = [];
	   counts = [];
	   remainders = [];
	   divisor = steps - pulses;
	remainders.push(pulses);
	level = 0;

	while(true) {
		counts.push(Math.floor(divisor / remainders[level]));
		remainders.push(divisor % remainders[level]);
		divisor = remainders[level];
	       level += 1;
		if (remainders[level] <= 1) {
			break;
		}
	}

	counts.push(divisor);

	var r = 0;
	var build = function(level) {
		r++;
		if (level > -1) {
			for (var i=0; i < counts[level]; i++) {
				build(level-1);
			}
			if (remainders[level] != 0) {
	        	build(level-2);
			}
		} else if (level == -1) {
	           pattern.push(0);
		} else if (level == -2) {
           pattern.push(1);
		}
	};

	build(level);
	return pattern.reverse();
}

// return an array of { value:, weight: } representing the bjorklund steps
var buildWeightArray = function(step, pulse, source)
{
  // Build the bjorlund array made of [0, 1]
  var b = bjorklund(step, pulse);

  // Convert it to a weigthed array
  var result = new Array();
  result.push({ value: b[0], weight : 1 });

  b.slice(1).reduce((c,x) => {
    if (x == 0)
    {
      c[c.length-1].weight++;
    }
    else
    {
      c.push({value : 1, weight: 1});
    }
    return c;
  }, result);

  // For every step, insert either a rest sequence or the source one
  const restPattern = makePatternFromEventArray([new Event(math.fraction(0),["~"])]);
  const weightArray = result.map(x => {
     const v = x.value == 0 ? restPattern : source;
     return { sequence: v, weight: x.weight};
   });

   return weightArray;
}

makeBjorklundOperator = function(source, step, pulse)
{
  var bjorklundFn = function(args)
  {
    const source = args[0];
    const step = args[1];
    const pulse = args[2];
    const weightArray = buildWeightArray(step, pulse, source);
    return makePatternFromWeightArray(weightArray);
  }

  return new Operator(bjorklundFn,
     [source, makeValueWrapperOperator(step), makeValueWrapperOperator(pulse)]);
}
