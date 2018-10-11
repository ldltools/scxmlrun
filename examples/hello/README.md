# hello

## [hello.scxml](hello.scxml)

```
<scxml xmlns="http://www.w3.org/2005/07/scxml" version="1.0" datamodel="ecmascript">  
<final id="hello">  
    <onentry>  
      <script>console.log ("hello")</script>  
    </onentry>  
</final>  
</scxml>
```

## run [hello.scxml](hello.scxml) using `scxmlrun`

```
$ scxmlrun hello.scxml  
hello
```
