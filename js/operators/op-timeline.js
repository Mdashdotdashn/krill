
var TimelineOperator = function(elementArray)
{
  this.nodes_ = elementArray.map(e => { return e.content ? e.content : e}  );
  this.current_ = 0;
}

TimelineOperator.prototype.tick = function()
{
  this.current_ = (this.current_ + 1) % this.nodes_.length;
  this.nodes_[this.current_].tick();
}

TimelineOperator.prototype.render = function()
{
  return this.nodes_[this.current_].render();
}

makeTimelineOperator = function(operatorArray)
{
  return new TimelineOperator(operatorArray);
}
