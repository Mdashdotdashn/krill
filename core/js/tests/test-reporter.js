var assert = require("assert");
var serverApi = require("../main.js");

function createFakeApp(initialEvents)
{
  var queue = (initialEvents || []).slice();

  return {
    parse: function(command)
    {
      if (command === "throw")
      {
        throw new Error("boom");
      }
      if (command === "enqueue")
      {
        queue.push({ time: "1/2", values: ["bd"] });
      }
      return "ok";
    },
    drainReportedEvents: function()
    {
      var out = queue.slice();
      queue.length = 0;
      return out;
    }
  };
}

async function run()
{
  var app = createFakeApp([
    { time: "0", values: ["a"] },
    { time: "1/4", values: ["b"] }
  ]);

  var server = await serverApi.createServer(app, { port: 0 });
  try
  {
    var first = await server.inject({ method: "GET", url: "/reporter" });
    assert.strictEqual(first.statusCode, 200);
    assert.deepStrictEqual(JSON.parse(first.payload), {
      reply: [
        { time: "0", values: ["a"] },
        { time: "1/4", values: ["b"] }
      ]
    });

    var second = await server.inject({ method: "GET", url: "/reporter" });
    assert.strictEqual(second.statusCode, 200);
    assert.deepStrictEqual(JSON.parse(second.payload), { reply: [] });

    var enqueue = await server.inject({ method: "GET", url: "/command?command=enqueue" });
    assert.strictEqual(enqueue.statusCode, 200);
    assert.deepStrictEqual(JSON.parse(enqueue.payload), { reply: "ok" });

    var afterEnqueue = await server.inject({ method: "GET", url: "/reporter" });
    assert.strictEqual(afterEnqueue.statusCode, 200);
    assert.deepStrictEqual(JSON.parse(afterEnqueue.payload), {
      reply: [{ time: "1/2", values: ["bd"] }]
    });

    var originalConsoleLog = console.log;
    console.log = function() {};
    try
    {
      var commandError = await server.inject({ method: "GET", url: "/command?command=throw" });
      assert.strictEqual(commandError.statusCode, 200);
      assert.deepStrictEqual(JSON.parse(commandError.payload), { reply: "*error*: boom" });
    }
    finally
    {
      console.log = originalConsoleLog;
    }
  }
  finally
  {
    await server.stop();
  }
}

run().catch(function(err) {
  console.error(err);
  process.exit(1);
});
