assert = require("assert");
math = require("mathjs");

require("../js/type.js");
require("../js/input-evaluator.js");
require("../js/renderer/render-tree.js");

TestEvaluator = function()
{
  this.evaluator_ = new Evaluator();
}

TestEvaluator.prototype.evaluateRenderingTree = function(s)
{
  return this.evaluator_.evaluateRenderingTree(s);
}

evaluator = new TestEvaluator();
quote = "\"";
