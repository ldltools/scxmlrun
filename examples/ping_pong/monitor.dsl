// -*-javascript-*-

// NOTE
// - the behavior of this process is *stricter* than that of the process composed of ping and pong,
//   in the sense that a sequence like "ping; quit" or "ping; pong; ping; quit" is not permitted.
// - this monitor does not look into event parameter values, which are all dynamic entities.
//   

protocol
  (ping; pong)*; quit ;;
  // pairs of *ping* and *pong* events (repeated 0 or more times)

rule
// ping ({count})
on ping do { console.log ("[monitor] ping", _event.data.count); }
// pong ({count})
on pong do { console.log ("[monitor] pong", _event.data.count); }
// quit ({die_alone})
on quit do { console.log ("[monitor] quit"); }
