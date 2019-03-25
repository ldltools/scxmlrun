// -*-javascript-*-

protocol
(pong; ping)*; pong?; quit ;;
  // regular pattern of the incoming events

rule

// pong ({count})
on pong
raise quit + ping
{
    var n = _event.data.count;
    if (n == 0)
	SCXML.raise ({name: "quit", data: {die_alone: 0}});
    else
	SCXML.raise ({name: "ping", data: {count: (n - 1)}});
        // note: this event is what the protocol means by "ping"

    console.log ("[pong]", _event.data.count);
}

// ping ({count})
on ping
do
{
    SCXML.send ({event: _event, topic: "ping"});
    // note: this outbound "ping" event is not included in the protocol
}

// quit ({die_alone})
on quit
do
{
    if (_event.data.die_alone == 0)
	SCXML.send ({event: {name: "quit", data: {die_alone: 1}}, topic: "ping"});
}
