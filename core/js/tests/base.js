assert = require("assert");
math = require("mathjs");

require("../type.js");
require("../input-evaluator.js");
require("../renderer/render-tree.js");

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
