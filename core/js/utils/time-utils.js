var math = require("mathjs");

// Centralized time/fraction utilities to reduce duplication across player and render nodes.

var TimeUtils = {
  // Convert any value to a mathjs Fraction.
  toFraction: function(value) {
    return math.fraction(value);
  },

  // Return the floor of the cycle (e.g., 1.75 → 1, -0.5 → -1).
  cycleStart: function(time) {
    return math.fraction(math.floor(TimeUtils.toFraction(time)));
  },

  // Return the start of the next cycle (e.g., 1.75 → 2, -0.5 → 0).
  nextCycleBoundary: function(time) {
    return math.add(TimeUtils.cycleStart(time), math.fraction(1));
  },

  // Epsilon used for point queries.
  epsilon: function() {
    return math.fraction(1, 1024);
  },

  // Check if two Fractions are equal.
  equal: function(a, b) {
    return math.equal(TimeUtils.toFraction(a), TimeUtils.toFraction(b));
  },

  // Check if a > b.
  larger: function(a, b) {
    return math.larger(TimeUtils.toFraction(a), TimeUtils.toFraction(b));
  },

  // Check if a >= b.
  largerEq: function(a, b) {
    return math.largerEq(TimeUtils.toFraction(a), TimeUtils.toFraction(b));
  },

  // Check if a < b.
  smaller: function(a, b) {
    return math.smaller(TimeUtils.toFraction(a), TimeUtils.toFraction(b));
  },

  // Check if a <= b.
  smallerEq: function(a, b) {
    return math.smallerEq(TimeUtils.toFraction(a), TimeUtils.toFraction(b));
  },

  // Add two time values.
  add: function(a, b) {
    return math.add(TimeUtils.toFraction(a), TimeUtils.toFraction(b));
  },

  // Subtract two time values.
  subtract: function(a, b) {
    return math.subtract(TimeUtils.toFraction(a), TimeUtils.toFraction(b));
  },

  // Multiply a time value by a scalar.
  multiply: function(a, scalar) {
    return math.multiply(TimeUtils.toFraction(a), scalar);
  },

  // Format a Fraction for display.
  format: function(value) {
    return math.format(TimeUtils.toFraction(value));
  },

  // Convert a Fraction to a number (useful for timing calculations).
  toNumber: function(value) {
    return math.number(TimeUtils.toFraction(value));
  }
};

if (typeof module !== 'undefined' && module.exports) {
  module.exports = TimeUtils;
}
