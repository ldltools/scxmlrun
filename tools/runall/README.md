## usage

```
scxmlrun-all <infile>
```

scxmlrun-all reads a definition of a composite process in JSON,
and generates a bash sript for its execution using `scxmlrun`.


## composite process definition

    <process>          ::= { "process" : <process_exp>, <attributes> }
    <process_exp>      ::= { "path": <path>, <attributes> }
                         | { "script": <script>, <attributes> }
                         | { "name": <name>, <attributes> }
                         | { "variable": {<name> : name, "process" : <process_exp>}, <process> }
                         | [ <process>, <process>, ..., <process> }

## process\_exp

### base process

- path

- script

- name

### parallel composition


## attributes

### common attributes

[Tags] type, protocol, mqtt_host

- type: string (optional)  
  "scxml" or "script"

- protocol: string (optional)  
  "mqtt" or "none"

- mqtt_host: string  
  MQTT broker


### attributes for specifiying input, output, error, log

[Tags] input, output, error, log

- input: alist
- output: alist
- error: alist
- log: alist

[Tags] path, script, mqtt_topic

These can appear as elements of _input_, _output_, _error_, or _log_-values.

- events: event array

- path: string

- script: string

- mqtt_topic: string (single-space delimited)
