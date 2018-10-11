# calc

The [SCXML definition](calc.scxml) originates from [this](https://www.w3.org/TR/scxml/#N11630).

<details>
<summary>calc statechart</summary>
<div><img src="calc.svg?sanitize=true"/></div>
</details>

## scenario: `calc.in` for computing 12 + 34 = 46

The following input events are included in [calc.in](calc.in).

```
{"event" : {"name" : "DIGIT.12"}}  
{"event" : {"name" : "OPER.PLUS"}}  
{"event" : {"name" : "DIGIT.34"}}  
{"event" : {"name" : "EQUALS"}}  
{"event" : {"name" : "terminate"}}
```

## running of the statechart

```
$ scxmlrun calc.scxml calc.in  
[log] "'result'"  :  "0"  
[log] "'result'"  :  "0"  
[log] "'result'"  :  "12"  
[log] ""  :  "OPER.PLUS"  
[log] "'result'"  :  "12"  
[log] "'result'"  :  "34"  
[log] "'result'"  :  "46"
```
