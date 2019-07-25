const math = require("mathjs");
require("./op-pattern.js");
const Scale = require("tonal-scale");
const Interval = require("tonal-interval");


makeScaleOperator = function(source, scale)
{
  const scaledef = Scale.tokenize(scale);
  const intervals = Scale.props(scaledef[1]).intervals.map(w => Interval.semitones(w));
  const offset = scaledef[0] != '' ? Note.chroma(scaleddef[0]) : 0;

  var scaleFn = function(args)
  {
    const sourceSequence = args[0].render();
    const eventArray = sourceSequence.events_.map(e => {
      const values = e.values_.map(v => {
          const degree = parseInt(v);
          const modulo =  degree % intervals.length;
          const remainder = (degree - modulo) / intervals.length;
          const interval = v < 0 ? intervals.length + modulo : modulo;
          const octave = (v < 0 ? remainder -1 : remainder) * 12;
          return intervals[interval] + octave;
      });
      return new Event(e.time_, values);
    })
//    Dump(eventArray);
    return makePatternFromEventArray(eventArray);
  }

  return new Operator(scaleFn,[source]);
}
